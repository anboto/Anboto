<!--
  Title: Anboto
  Description: A set of C++ libraries to streamline project development based on the U++ framework
  Authors: Iñaki Zabala.
  -->

# Anboto
**A set of C++ libraries to streamline project development based on the U++ framework.**

[![License: Apache v2](https://img.shields.io/badge/License-Apachev2-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
<img src="https://github.com/izabala123/BEMRosetta/blob/master/other/md%20resources/platforms-windows_linux-blue.svg" alt="Platforms">
<img src="https://img.shields.io/github/last-commit/anboto/Anboto.svg" alt="Last commit">

Anboto is proudly developed with [the U++ framework](https://www.ultimatepp.org/) using the C++ language. U++ makes C++ so simple and efficient that it appeals even to scripting language programmers. 

Anboto adds multiple additional features to U++ so that your projects can be developed faster.

## Libraries included
### General use
* Functions4U
* Controls4U
* ScatterDraw
* ScatterCtrl
* SysInfo
* OfficeAutomation
 
### Engineering and scientific
* Eigen
* STEM4U
* Surface
* GLCanvas
* Matio

### Libraries
These external libraries are included as they have been adapted for immediate use with Anboto. They are updated periodically and their licenses are compatible with Anboto's.
* plugin/Eigen
* plugin/assimp
* plugin/matio
* plugin/sundials

## Install
* Clone or copy Anboto to your computer. The best place to copy Anboto is the same folder as U++. A normal structure could be:
  * U++ParentFolder/upp
  * U++ParentFolder/upp/uppsrc
  * U++ParentFolder/upp/examples
  * U++ParentFolder/upp/bazaar
  * ...
  * U++ParentFolder/Anboto
* Go to U++ folder (normally 'upp') and open uppsrc.var with a text editor like Notepad.
  * In Windows, it will be in 'upp' folder
  * In Linux, it will be in 'upp/.config/u++/theide' folder 
* uppsrc.var will contain something like:
<pre>
UPP    = "U++ParentFolder/upp/uppsrc";
OUTPUT = "U++ParentFolder/upp/out";
</pre>
* Replace the content of "UPP = ..." line with:
<pre>
UPP    = "AnbotoParentFolder/Anboto;U++ParentFolder/upp/uppsrc;U++ParentFolder/upp/bazaar";
</pre>
* Save the file as 'Anboto.var'

From now on you will have 'Anboto' ready to be selected in the 'Select main package' dialog of TheIDE, and called from umk, the command line U++ make tool.

## Changes log

2021/03/10 STEM4U: New shortest path support: [Dijkstra](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm), [Bellman-Ford](https://en.wikipedia.org/wiki/Bellman%E2%80%93Ford_algorithm) and [Floyd-Warshall](https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm)
