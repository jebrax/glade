ifndef VENDOR
VENDOR=..
endif

ifndef GLADE_BUILD_DIR
GLADE_BUILD_DIR=build/glade
endif

.PHONY: clean ass

CXX_SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst src/%.cpp, $(GLADE_BUILD_DIR)/%.o, $(CXX_SRCS))
DEPS = $(patsubst src/%.cpp, $(GLADE_BUILD_DIR)/%.o.d, $(CXX_SRCS))

all: ass ${GLADE_BUILD_DIR}/libglade.a

###################### Linkage
${GLADE_BUILD_DIR}/libglade.a: build/lodepng/lodepng.o
${GLADE_BUILD_DIR}/libglade.a: ${OBJS}
	libtool -static build/lodepng/lodepng.o ${OBJS} -o $@

###################### Compilation

COMMON_INCLUDES = -Iinclude
CXX = clang++
CXXFLAGS = -std=c++20 -O0 -g -DGLADE_MACOS ${COMMON_INCLUDES} -MMD -MF $@.d

-include $(DEPS)

${GLADE_BUILD_DIR}/%.o: src/%.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -c $< -o $@

build/lodepng/lodepng.o:
	mkdir -p build/lodepng
	${CXX} ${CXXFLAGS} -c ${VENDOR}/lodepng/lodepng.cpp -o $@

${GLADE_BUILD_DIR}/physics/CollisionDetector.o: src/physics/CollisionDetector.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/libccd/src -c $< -o $@

${GLADE_BUILD_DIR}/physics/SpherePhysicalObject.o: src/physics/SpherePhysicalObject.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/libccd/src -c $< -o $@

${GLADE_BUILD_DIR}/physics/IsosurfaceCellPhysicalObject.o: src/physics/IsosurfaceCellPhysicalObject.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/libccd/src -c $< -o $@

${GLADE_BUILD_DIR}/ui/font/FreetypeFont.o: src/ui/font/FreetypeFont.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/freetype/include -c $< -o $@

${GLADE_BUILD_DIR}/util/ResourceManager.o: src/util/ResourceManager.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/assimp/include -I${VENDOR}/freetype/include -I${VENDOR}/lodepng -c $< -o $@

${GLADE_BUILD_DIR}/generation/noise.o: src/generation/noise.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/PerlinNoise/include -c $< -o $@

${GLADE_BUILD_DIR}/render/GladeRenderer.o: src/render/GladeRenderer.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/glew/include -DGL_SILENCE_DEPRECATION -c $< -o $@

${GLADE_BUILD_DIR}/opengl/functions.o: src/opengl/functions.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -DGL_SILENCE_DEPRECATION -c $< -o $@

ass:
	rm -rf ${GLADE_BUILD_DIR}/assets
	mkdir -p ${GLADE_BUILD_DIR}/assets
	cp -R res/* ${GLADE_BUILD_DIR}/assets

clean:
	rm -rf build

