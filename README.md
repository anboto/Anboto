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

Anboto is proudly developed with the U++ framework using the C++ language. U++ makes C++ so simple and efficient that it appeals even to scripting language programmers. 

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
* Go to U++ folder (normally 'upp') and open uppsrc.var with a text editor like Notepad. You will get something like:
<pre>
UPP = "%U++ParentFolder$/upp/uppsrc";
OUTPUT = "%U++ParentFolder$/upp/out";

* Replace content of "UPP = ..." line with:
<pre>UPP = "%AnbotoParentFolder$/Anboto;%U++ParentFolder$/upp/uppsrc;%U++ParentFolder$/upp/bazaar";
* Save the file as 'Anboto.var'

From now on you will have 'Anboto' ready to be selected in the 'Select main package' dialog.
