CPUS := $(shell nproc)
PREMAKE_BIN := $(shell which premake5)
ifeq (, $(PREMAKE_BIN))
PREMAKE_BIN = .build/premake5
endif

all: build_release
	true

build_sanitize: box2d premake
	$(PREMAKE_BIN) gmake --cc=clang --sanitize
	make -C .build config=debug_x86_64 -j$(CPUS)
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Debug/carnage3d bin/carnage3d-debug

build_debug: box2d premake
	$(PREMAKE_BIN) gmake --cc=clang
	make -C .build config=debug_x86_64 -j$(CPUS)
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Debug/carnage3d bin/carnage3d-debug

build_release: box2d premake
	$(PREMAKE_BIN) gmake --cc=clang
	make -C .build config=release_x86_64 -j$(CPUS)
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Release/carnage3d bin/carnage3d-release

get_demoversion:
	mkdir -p gamedata/demoversions
	cd gamedata/demoversions 
	wget "https://web.archive.org/web/20200930091556/https://www.rockstargames.com/gta/demos/gtaects.zip" -O gamedata/demoversions/gtaects.zip
	unzip gamedata/demoversions/gtaects.zip -d gamedata/demoversions

clean:
	$(PREMAKE_BIN) gmake --cc=clang
	rm -rf third_party/Box2D/build
	make -C .build clean

run:
	./bin/carnage3d-release
	
run_demoversion:
	./bin/carnage3d-release -mapname SANB.CMP -gtadata "gamedata/demoversions/GTAECTS/GTADATA"

builddir: 
	test -d .build || mkdir .build

premake: builddir
	test -e $(PREMAKE_BIN) || (cd .build && \
	wget https://github.com/premake/premake-core/releases/download/v5.0.0-beta1/premake-5.0.0-beta1-linux.tar.gz && \
	tar xzf premake-5.0.0-beta1-linux.tar.gz && \
	rm premake-5.0.0-beta1-linux.tar.gz)

box2d:
	cd third_party/Box2D && \
	mkdir -p build && cd build && \
	cmake -DCMAKE_BUILD_TYPE=Release -DBOX2D_BUILD_DOCS=Off -DBOX2D_BUILD_UNIT_TESTS=Off -DBOX2D_BUILD_TESTBED=Off .. && \
	cmake --build .
