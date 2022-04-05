// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

#include <Eigen/Eigen.h>
#include <Surface/Surface.h>
#include <Geom/Geom.h>
#include <Functions4U/Functions4U.h>

namespace Upp {
using namespace Eigen;

static void LoadStlTxt(String fileName, Surface &surf, bool &isText) {
	isText = false;
	
	FileInLine in(fileName);
	if (!in.IsOpen()) 
		throw Exc(Format(t_("Impossible to open file '%s'"), fileName));
	
	String line;
	FieldSplit f(in);	
	f.IsSeparator = [](int c)->int {return c == '\t' || c == ' ' || c == '!';};
	
	line = ToLower(TrimBoth(in.GetLine()));
	if (!line.StartsWith("solid"))
		throw Exc(in.Str() + ". "  + t_("'solid' field not found"));
	
	isText = true;
	
    while (!in.IsEof()) {	
		line = ToLower(TrimBoth(in.GetLine()));
		if (line.IsEmpty())
			continue;
		
		if (line.StartsWith("endsolid"))
			break;
		
		if (!line.StartsWith("facet normal"))
			throw Exc(in.Str() + ". "  + t_("'facet normal' field not found"));

		line = ToLower(TrimBoth(in.GetLine()));
		if (!line.StartsWith("outer loop"))
			throw Exc(in.Str() + ". "  + t_("'outer loop' field not found"));			

		int ids[5];
		for (int i = 0; i < 6; ++i) {
			f.Load(in.GetLine());
			String label = f.GetText(0);
			
			if (label == "vertex") {		
				if (i == 4)
					throw Exc(in.Str() + ". "  + t_("Too much vertex in facet"));			
				Point3D node(f.GetDouble(1), f.GetDouble(2), f.GetDouble(3));
				surf.nodes << node;
				ids[i] = surf.nodes.GetCount() - 1;
			} else if (label == "endloop") {
				if (i < 3)
					throw Exc(in.Str() + ". "  + t_("Too few vertex in facet"));
				Panel &panel = surf.panels.Add();
				panel.id[0] = ids[0];
				panel.id[1] = ids[1];
				panel.id[2] = ids[2];
				if (i == 3) 
					panel.id[3] = ids[0];
				else if (i == 4)
					panel.id[3] = ids[3];
			} else if (label == "endfacet") 
				break;
			else
				throw Exc(in.Str() + ". "  + Format(t_("Label '%s' not handled in facet"), label));	
		}
	}
}
			
static void STLFacetTxtOut(FileOut &out, const Point3D &p0, const Point3D &p1, const Point3D &p2, 
						const Point3D &p3, const Direction3D &normal, double factor) {
	out << "facet normal " << normal.x << " " << normal.y << " " << normal.z << "\n";
	out << "   outer loop" << "\n";
	out << "      vertex " << p0.x*factor << " " << p0.y*factor << " " << p0.z*factor << "\n";
	out << "      vertex " << p1.x*factor << " " << p1.y*factor << " " << p1.z*factor << "\n";
	out << "      vertex " << p2.x*factor << " " << p2.y*factor << " " << p2.z*factor << "\n";
	if (!IsNull(p3))
		out << "      vertex " << p3.x << " " << p3.y << " " << p3.z << "\n";	
	out << "   endloop" << "\n";
	out << "endfacet" << "\n";
}

void SaveStlTxt(String fileName, const Surface &surf, double factor) {
	FileOut out(fileName);
	if (!out.IsOpen())
		throw Exc(Format(t_("Impossible to open '%s'\n"), fileName));	
	
	const Vector<Panel> &panels = surf.panels;
	const Vector<Point3D> &nodes = surf.nodes;
	
	bool forceTriangles = true;
	
	out << "Anboto STL mesh export" << "\n";

	for (int i = 0; i < panels.GetCount(); ++i) {
		const Panel &panel = panels[i];
		const Point3D &p0 = nodes[panel.id[0]];
		const Point3D &p1 = nodes[panel.id[1]];
		const Point3D &p2 = nodes[panel.id[2]];
		if (forceTriangles) {
			STLFacetTxtOut(out, p0, p1, p2, Null, panel.normal0, factor);
			if (!panel.IsTriangle()) {
				const Point3D &p3 = nodes[panel.id[3]];
				STLFacetTxtOut(out, p2, p3, p0, Null, panel.normal1, factor);	
			}	
		} else {
			if (panel.IsTriangle()) 
				STLFacetTxtOut(out, p0, p1, p2, Null, panel.normalPaint, factor);
			else {
				const Point3D &p3 = nodes[panel.id[3]];
				STLFacetTxtOut(out, p0, p1, p2, p3, panel.normalPaint, factor);	
			}
		}
	}
	out << "endsolid U++ STL mesh export" << "\n";
}

static void LoadStlBin(String fileName, Surface &surf, String &header) {
	FileInBinary in(fileName);
	if (!in.IsOpen())
		throw Exc(Format(t_("Impossible to open file '%s'"), fileName));
	
	StringBuffer headerB(80);
	in.ReadB(headerB, 80);
	header = headerB;
	
	if (header.StartsWith("solid"))
		throw Exc(t_("Binary stl must not begin with 'solid' text"));
	
	/*int32 numFacets =*/ in.ReadB<int32>();
	
	while (!in.IsEof()) {
    	Direction3D normal;
    	normal.x = double(in.ReadB<float>());
    	normal.y = double(in.ReadB<float>());
    	normal.z = double(in.ReadB<float>());

		Panel &panel = surf.panels.Add();

		Point3D &node0 = surf.nodes.Add();
		node0.x = double(in.ReadB<float>());
		node0.y = double(in.ReadB<float>());
		node0.z = double(in.ReadB<float>());
		panel.id[0] = surf.nodes.GetCount()-1;
		
		Point3D &node1 = surf.nodes.Add();
		node1.x = double(in.ReadB<float>());
		node1.y = double(in.ReadB<float>());
		node1.z = double(in.ReadB<float>());
		panel.id[1] = surf.nodes.GetCount()-1;
		
		Point3D &node2 = surf.nodes.Add();			
		node2.x = double(in.ReadB<float>());
		node2.y = double(in.ReadB<float>());
		node2.z = double(in.ReadB<float>());
		panel.id[2] = surf.nodes.GetCount()-1;
		panel.id[3] = panel.id[0];		
				
		/*int16 attributeByteCount =*/ in.ReadB<int16>();
	}
}

static void STLFacetBinNodeOut(FileOutBinary &out, const Point3D &node) {
	out.Write(float(node.x));
	out.Write(float(node.y));
	out.Write(float(node.z));	
}

void SaveStlBin(String fileName, const Surface &surf, double factor) {
	FileOutBinary out(fileName);
	if (!out.IsOpen())
		throw Exc(Format(t_("Impossible to open '%s'\n"), fileName));	

	const Vector<Panel> &panels = surf.panels;
	const Vector<Point3D> &nodes = surf.nodes;
	
	String header = "U++ STL mesh export";
	header << String(' ', 80 - header.GetCount());
	out.Put64(header, 80);
	
	out.Write(int32(panels.GetCount()));
	
	for (int i = 0; i < panels.GetCount(); ++i) {
		const Panel &panel = panels[i];
		
		out.Write(float(panel.normal0.x*factor));
		out.Write(float(panel.normal0.y*factor));
		out.Write(float(panel.normal0.z*factor));
		STLFacetBinNodeOut(out, nodes[panel.id[0]]);
		STLFacetBinNodeOut(out, nodes[panel.id[1]]);
		STLFacetBinNodeOut(out, nodes[panel.id[2]]);
		out.Write(int16(0));
		
		if (!panel.IsTriangle()) {
			out.Write(float(panel.normal1.x*factor));
			out.Write(float(panel.normal1.y*factor));
			out.Write(float(panel.normal1.z*factor));
			STLFacetBinNodeOut(out, nodes[panel.id[2]]);
			STLFacetBinNodeOut(out, nodes[panel.id[3]]);
			STLFacetBinNodeOut(out, nodes[panel.id[0]]);	
			out.Write(int16(0));
		}
	}
}

void LoadStl(String file, Surface &surf, bool &isText, String &header) {
	try {
		LoadStlTxt(file, surf, isText);
	} catch(Exc e) {
		if (!isText) 
			LoadStlBin(file, surf, header);
		else
			throw std::move(e);
	}
	double mx = 0;
	for (int i = 0; i < surf.nodes.size(); ++i) {
		const auto &p = surf.nodes[i];
		if (abs(p.x) > mx)
			mx = abs(p.x);
		if (abs(p.y) > mx)
			mx = abs(p.y);
		if (abs(p.z) > mx)
			mx = abs(p.z);
	}
	if (mx > 500) {		// It's guessed to be in mm. Convert to m
		for (int i = 0; i < surf.nodes.size(); ++i) {
			auto &p = surf.nodes[i];
			p.x /= 1000;
			p.y /= 1000;
			p.z /= 1000;
		}
	}
}


}