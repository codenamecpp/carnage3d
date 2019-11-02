# Carnage3D #

![alt text](https://github.com/codenamecpp/carnage3d/blob/master/carnage3d_logo.png)

This is open source Grand Theft Auto 1 remake project.

[![Build Status](https://travis-ci.org/codenamecpp/carnage3d.svg?branch=master)](https://travis-ci.org/codenamecpp/carnage3d)
[![Github Stars](https://img.shields.io/github/stars/codenamecpp/carnage3d?logo=github)](https://github.com/codenamecpp/carnage3d/stargazers)
![Platforms](https://img.shields.io/badge/platform-windows%20%7C%20linux-blue) ![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)

## Status ##
Currently it is in very early stage, a little progress so far: https://www.youtube.com/watch?v=91L_CJ0teEA

## Tools and technologies used ##
* C++11
* OpenGL 3.2
* Box2D
* cJSON
* Dear ImGui
* STBI
* GLM math library
* GLFW
* GLEW

## Compiling on Linux ##
At the moment, makefile configured so project will build with clang compiler.

Before build project make sure to install dependencies: 
* apt install libglew-dev libglm-dev libgl1-mesa-dev xorg-dev clang

Run these commands in terminal:
* git clone --recurse-submodules https://github.com/codenamecpp/carnage3d
* cd carnage3d
* make

## Compiling on Windows ##
You will need Visual Studio 2015 Community Edition to build project.
Before compile, add system environment variable __SDKDIR__ with path where all the dependencies lives. These dependencies are:
* GLEW
* GLM

## Configure ##
Game needs to be configured before launch, there is example config document in __gamedata/config/sys_config.json.default__ - it must be copy-pasted to same directory but withoud ".default" suffix: __gamedata/config/sys_config.json__ .

Most important param there is __gta_gamedata_location__ - it is location of GTA1 gamedata resources.

## How To Run ##

**Original GTA1 game resources required in order to play (Full or DEMO)**. Demo version still available for download https://www.rockstargames.com/gta/extras/demos.html .

It is possible to download and play demo version of the game easily just typing a few commands in console:
* make get_demoversion
* make run_demoversion

Note that game must be compiled and configured before this step as described above.

Tested on Ubuntu Linux and Windows 7 64bits.

## Command Line Params ##

To select specific level to play you can add command line argument **-mapname**, for example: **-mapname SANB.CMP**. By default **NYC** will loaded.

## Controls ##
It is similar to original:
* **Arrow** keys to walk/drive in directions
* **Space** to slide on car
* **Enter** to enter or exit car
* **Tab** to enable or disable car emergency lights (if it present)
* **Z**, **X** to change current weapon and **Ctrl** to shoot

When in "Free Look" camera mode, moving with **W**, **S**, **A**, **D** keys.

## Other GTA 1/2 re-implementations ##
* [WebGL-GTA](https://github.com/niklasvh/WebGL-GTA)
* [OpenGTA](http://ogta.fifengine.de/download.html)
* [GTA2NET](https://github.com/andrecarlucci/gta2net)
* [WebGL-GTA2](https://github.com/Kajakklubben/gta2-webgl)
* [GTA2 level viewer](https://github.com/aalin/gta2-level-viewer)
* [gta2.js](https://github.com/aalin/gta2.js)

## Screenshots ##
![alt text](https://github.com/codenamecpp/carnage3d/blob/master/screenshots/WorksOnLinux.png)
