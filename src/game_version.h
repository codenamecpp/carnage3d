#pragma once

#define GAME_VERSION_MAJOR 0
#define GAME_VERSION_MINOR 0
#define GAME_VERSION_BUILD 4

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define GAME_TITLE "Carnage3D " STRINGIZE(GAME_VERSION_MAJOR) "." STRINGIZE(GAME_VERSION_MINOR) "." STRINGIZE(GAME_VERSION_BUILD)