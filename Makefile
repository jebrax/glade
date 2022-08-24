libglade.a: FreetypeFont.o Layout.o Widget.o log.o Matrix.o globals.o Path.o WavefrontObjReader.o CSVReader.o Simulator.o CollisionDetector.o Perception.o GladeRenderer.o GladeObject.o ResourceManager.o DesktopFileManager.o MarchingCubesTables.o MeshGenerator.o AdvancedMeshGenerator.o Grid.o noise.o lodepng.o assets builddir set-env
	libtool -static -o build/glade/libglade.a build/lodepng/lodepng.o build/glade/FreetypeFont.o build/glade/ResourceManager.o build/glade/Layout.o build/glade/Widget.o build/glade/log.o build/glade/Matrix.o build/glade/globals.o build/glade/Path.o build/glade/WavefrontObjReader.o build/glade/CSVReader.o build/glade/Simulator.o build/glade/CollisionDetector.o build/glade/Perception.o build/glade/MarchingCubesTables.o build/glade/MeshGenerator.o build/glade/AdvancedMeshGenerator.o build/glade/GladeRenderer.o build/glade/GladeObject.o build/glade/DesktopFileManager.o build/glade/Grid.o build/glade/noise.o

######################

lodepng.o: builddir-lodepng
	clang -O0 -g -c ${VENDOR}/lodepng/lodepng.cpp -o build/lodepng/lodepng.o

FreetypeFont.o: builddir
	clang -O0 -g -I ${VENDOR}/freetype-2.10.1/include -I include -DGLADE_MACOS -c src/ui/font/FreetypeFont.cpp -o build/glade/FreetypeFont.o

Layout.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/ui/layout/Layout.cpp -o build/glade/Layout.o

Widget.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/ui/Widget.cpp -o build/glade/Widget.o

log.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/debug/log.cpp -o build/glade/log.o

Matrix.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/math/Matrix.cpp -o build/glade/Matrix.o

globals.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/math/globals.cpp -o build/glade/globals.o

Path.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/util/Path.cpp -o build/glade/Path.o

WavefrontObjReader.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/util/WavefrontObjReader.cpp -o build/glade/WavefrontObjReader.o

CSVReader.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/util/CSVReader.cpp -o build/glade/CSVReader.o

ResourceManager.o: builddir
	clang -O0 -std=c++11 -g -I ${VENDOR}/assimp/include -I ${VENDOR}/freetype-2.10.1/include -I ${VENDOR}/lodepng -I include -DGLADE_MACOS -c src/util/ResourceManager.cpp -o build/glade/ResourceManager.o

DesktopFileManager.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/util/DesktopFileManager.cpp -o build/glade/DesktopFileManager.o

Simulator.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/physics/Simulator.cpp -o build/glade/Simulator.o

CollisionDetector.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/physics/CollisionDetector.cpp -o build/glade/CollisionDetector.o

Perception.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/render/Perception.cpp -o build/glade/Perception.o

MarchingCubesTables.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/generation/MarchingCubesTables.cpp -o build/glade/MarchingCubesTables.o

MeshGenerator.o: builddir
	clang -std=c++17 -O0 -g -I include -DGLADE_MACOS -c src/generation/MeshGenerator.cpp -o build/glade/MeshGenerator.o

AdvancedMeshGenerator.o: builddir
	clang -std=c++17 -O0 -g -I include -DGLADE_MACOS -c src/generation/AdvancedMeshGenerator.cpp -o build/glade/AdvancedMeshGenerator.o

Grid.o: builddir
	clang -std=c++17 -O0 -g -I include -DGLADE_MACOS -c src/generation/Grid.cpp -o build/glade/Grid.o

noise.o: builddir
	clang -std=c++17 -O0 -g -I include -I${VENDOR}/PerlinNoise/include -DGLADE_MACOS -c src/generation/noise.cpp -o build/glade/noise.o

GladeRenderer.o: builddir
	clang -O0 -g -I${VENDOR}/glew/include -I include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/render/GladeRenderer.cpp -o build/glade/GladeRenderer.o

functions.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/opengl/functions.cpp -o build/glade/functions.o

GladeObject.o: builddir
	clang -O0 -g -I include -DGLADE_MACOS -c src/GladeObject.cpp -o build/glade/GladeObject.o

######################

assets: builddir
	mkdir -p build/glade/assets
	cp -R res/* build/glade/assets

builddir: 
	mkdir -p build/glade

builddir-lodepng:
	mkdir -p build/lodepng

clean:
	rm -rf build

set-env:
ifndef VENDOR
VENDOR=deps
endif

