# Carnage3D #
This is grand theft auto 1 remake project, just for fun.
Tools and technologies used:
* C++11
* OpenGL 3.2
* Box2D
* cJSON
* Dear ImGui
* STBI
* GLM math library
* GLFW
* GLEW

### Compiling on Linux ###
At the moment, makefile configured so project will build with clang compiler.

Before build project make sure to install dependencies: apt install libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev xorg-dev clang

Run these commands in terminal:
* git clone --recurse-submodules https://github.com/codenamecpp/carnage3d
* cd carnage3d
* make

Done! Now proceed to __How To Run__ section below.

### Compiling on Windows ###
You will need Visual Studio 2015 Community Edition to build project.
Before compile, add system environment variable __SDKDIR__ with path where all the dependencies lives. These dependencies are:
* GLEW
* GLM
* GLFW

### How To Run ###
Game needs to be configured before launch, there is example config document in __gamedata/config/sys_config.json.default__ - it must be copy-pasted to same directory but withoud ".default" suffix: __gamedata/config/sys_config.json__ . 

Set screen resolution (optionally) but most importand you must specify path to original GTA1 game data - __gta_gamedata_location__

**Original GTA1 game resources required in order to play (Full or DEMO)**. Demo version still available for download https://www.rockstargames.com/gta/extras/demos.html .

To select specific level to play you can add command line argument **-mapname**, for example: **-mapname SANB.CMP**. By default **NYC** will loaded.

Currently it is in very early stage, a little progress so far: https://www.youtube.com/watch?v=91L_CJ0teEA

Tested on Ubuntu Linux:
![alt text](https://github.com/codenamecpp/carnage3d/blob/master/screenshots/WorksOnLinux.png)
