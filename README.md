# Carnage3D #

![Carnage3D Logo](https://raw.githubusercontent.com/codenamecpp/carnage3d/master/carnage3d_logo.png)

This is open source Grand Theft Auto 1 remake project.

[![Build Status](https://travis-ci.org/codenamecpp/carnage3d.svg?branch=master)](https://travis-ci.org/codenamecpp/carnage3d)
[![Github Stars](https://img.shields.io/github/stars/codenamecpp/carnage3d?logo=github)](https://github.com/codenamecpp/carnage3d/stargazers)
![Platforms](https://img.shields.io/badge/platform-windows%20%7C%20linux-blue) ![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)

## Status ##
Currently it is in very early stage, a progress so far: https://youtu.be/KnqaO0YZzZY

## Tools and technologies used ##
* [Box2D](https://github.com/erincatto/box2d)
* [cJSON](https://github.com/DaveGamble/cJSON)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [STB](https://github.com/nothings/stb)
* [GLM](https://github.com/g-truc/glm)
* [GLFW](https://github.com/glfw/glfw)
* [GLEW](https://github.com/nigels-com/glew)

## Compiling on Linux ##
At the moment, makefile configured so project will build with clang compiler.

Before build project make sure to install dependencies: 
* apt install libglew-dev libglm-dev libgl1-mesa-dev xorg-dev clang

Run these commands in terminal:
* git clone --recurse-submodules https://github.com/codenamecpp/carnage3d
* cd carnage3d
* make

## Compiling on Windows ##
You will need Visual Studio 2015 to build the project.
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

Tested on Ubuntu Linux, Windows 7, Windows 10.

## Command Line Params ##

* To select specific level to play you can add command line argument **-mapname**, for example: **-mapname SANB.CMP**
* To specify the game data location add argument **-gtadata** followed by path
* To enable split screen mode add **-numplayers**, for example **-numplayers 2**, max 4 players is supported

## Controls ##
It is similar to original:
* **Arrow** keys to walk/drive in directions
* **Space** to slide on car
* **Enter** to enter or exit car
* **Tab** to enable or disable car emergency lights (if it present)
* **Z**, **X** to change current weapon and **Ctrl** to shoot

When in "Free Look" camera mode, moving with **W**, **S**, **A**, **D** keys.

In addition there is support of xbox-compatible gamepads. To enable it or change default key bindings create config file gamedata/config/inputs.json (copy-paste template inputs.json.default) and modify it as you wish.

## Other GTA 1/2 re-implementations ##
* [WebGL-GTA](https://github.com/niklasvh/WebGL-GTA)
* [OpenGTA](http://ogta.fifengine.de/download.html)
* [GTA2NET](https://github.com/andrecarlucci/gta2net)
* [WebGL-GTA2](https://github.com/Kajakklubben/gta2-webgl)
* [GTA2 level viewer](https://github.com/aalin/gta2-level-viewer)
* [gta2.js](https://github.com/aalin/gta2.js)
* [GTAViewer](https://github.com/GitExl/GTAViewer)
* [rust-theft-auto](https://github.com/gaudecker/rust-theft-auto)

## Screenshots ##
First test launch on Linux
![ScreenShot](https://raw.githubusercontent.com/codenamecpp/carnage3d/master/screenshots/WorksOnLinux.png)

Split screen test on Windows
![ScreenShot](https://raw.githubusercontent.com/codenamecpp/carnage3d/master/screenshots/SplitScreenTest.png)
