all: build_debug
	true

build_debug: box2d premake
	.build/premake5 gmake --cc=clang
	make -C .build config=debug_x86_64 -j $(($(grep -c ^processor /proc/cpuinfo)/2))
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Debug/carnage3d bin/carnage3d-debug

build_release: box2d premake
	.build/premake5 gmake --cc=clang
	make -C .build config=release_x86_64 -j $(($(grep -c ^processor /proc/cpuinfo)/2))
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Release/carnage3d bin/carnage3d-release

clean:
	.build/premake5 gmake --cc=clang
	make -C .build clean

run:
	bin/carnage3d-release

builddir: 
	test -d .build || mkdir .build

premake: builddir
	test -e .build/premake5 || (cd .build && \
	wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-linux.tar.gz && \
	tar xzf premake-5.0.0-alpha14-linux.tar.gz && \
	rm premake-5.0.0-alpha14-linux.tar.gz)

box2d: premake
	.build/premake5 --file=third_party/Box2D/premake5.lua gmake
	make -C third_party/Box2D/Build config=debug_x86_64
	make -C third_party/Box2D/Build config=release_x86_64


