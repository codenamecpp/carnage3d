workspace "carnage3d"
   location '.build'
   configurations { "Debug", "Release" }
   cppdialect 'C++17'

configuration { "linux", "gmake" }
   buildoptions { "-Wno-switch", "-fsanitize=address", "-fsanitize=undefined" }
   linkoptions { "-fsanitize=address", "-fsanitize=undefined" }

filter 'system:linux'
   platforms { 'x86_64' }

project "carnage3d"
   kind "WindowedApp"
   language "C++"
   files { "src/*.h", "src/*.cpp" }

   includedirs { "third_party/Box2D" }
   links { "glfw", "GL", "GLEW", "stdc++fs", "Box2D" }


   filter { "configurations:Debug" }
      defines { "DEBUG", "_DEBUG" }
      symbols "On"
      libdirs { "third_party/Box2D/Build/bin/x86_64/Debug" }

   filter { "configurations:Release" }
      defines { "NDEBUG" }
      optimize "On"
      libdirs { "third_party/Box2D/Build/bin/x86_64/Release" }
