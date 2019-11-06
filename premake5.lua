newoption {
    trigger = "sanitize",
    description = "enable sanitizers"
}

workspace "carnage3d"
   location '.build'
   configurations { "Debug", "Release" }
   cppdialect 'C++17'

configuration { "linux", "gmake" }
buildoptions { "-Wno-switch" }

if _OPTIONS["sanitize"] then
    buildoptions { "-fsanitize=address", "-fsanitize=undefined" }
    linkoptions { "-fsanitize=address", "-fsanitize=undefined" }
end

filter 'system:linux'
   platforms { 'x86_64' }

project "carnage3d"
	kind "WindowedApp"
   	language "C++"
   	files 
	{ 
		"src/*.h", 
		"src/*.cpp",
		"src/GLFW/internal.h",
		"src/GLFWglfw_config.h",
		"src/GLFW/glfw3.h",
		"src/GLFW/glfw3native.h",
		"src/GLFW/context.c",
		"src/GLFW/init.c",
		"src/GLFW/input.c",
		"src/GLFW/monitor.c",
		"src/GLFW/vulkan.c",
		"src/GLFW/window.c",
    	"src/GLFW/x11_platform.h",
		"src/GLFW/xkb_unicode.h",
		"src/GLFW/linux_joystick.h",
		"src/GLFW/posix_time.h",
		"src/GLFW/glx_context.h",
		"src/GLFW/egl_context.h",
		"src/GLFW/x11_init.c",
		"src/GLFW/x11_monitor.c",
		"src/GLFW/x11_window.c",
		"src/GLFW/glx_context.h",
		"src/GLFW/glx_context.c",
		"src/GLFW/glext.h",
		"src/GLFW/xkb_unicode.c",
		"src/GLFW/linux_joystick.c",
		"src/GLFW/posix_time.c",
		"src/GLFW/glx_context.c",
		"src/GLFW/egl_context.c",
		"src/GLFW/posix_thread.h",
		"src/GLFW/posix_thread.c",
		"src/GLFW/osmesa_context.c",
		"src/GLFW/osmesa_context.h"
	}
	includedirs { "third_party/Box2D" }
	includedirs { "GLFW" }
	links { "GL", "GLEW", "stdc++fs", "Box2D", "X11", "Xrandr", "Xinerama", "Xcursor", "pthread", "dl" }

	filter { "configurations:Debug" }
		defines { "DEBUG", "_DEBUG" }
		symbols "On"
		libdirs { "third_party/Box2D/Build/bin/x86_64/Debug" }

	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"
		libdirs { "third_party/Box2D/Build/bin/x86_64/Release" }
