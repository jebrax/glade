ifndef VENDOR
VENDOR=..
endif

ifndef GLADE_BUILD_DIR
GLADE_BUILD_DIR=build/glade
endif

.PHONY: clean ass collision-test kinematic-mover-test

CXX_SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst src/%.cpp, $(GLADE_BUILD_DIR)/%.o, $(CXX_SRCS))
DEPS = $(patsubst src/%.cpp, $(GLADE_BUILD_DIR)/%.o.d, $(CXX_SRCS))

all: ass ${GLADE_BUILD_DIR}/libglade.a

# CPU-only detection test using the real engine sources, without GL or vendor libraries.
COLLISION_TEST_SRCS = tests/CollisionTest.cpp src/GladeObject.cpp src/debug/log.cpp src/io/Stream.cpp \
                     $(wildcard src/collisions/*.cpp)
COLLISION_TEST_HEADERS = $(wildcard include/glade/collisions/*.h src/collisions/*.h) \
                        include/glade/GladeObject.h $(wildcard include/glade/math/*.h)

collision-test: ${GLADE_BUILD_DIR}/collision-test
	${GLADE_BUILD_DIR}/collision-test

${GLADE_BUILD_DIR}/collision-test: ${COLLISION_TEST_SRCS} ${COLLISION_TEST_HEADERS}
	mkdir -p $(@D)
	${CXX} -std=c++20 -O0 -g -Iinclude ${COLLISION_TEST_SRCS} -o $@

KINEMATIC_TEST_SRCS = tests/KinematicMoverTest.cpp $(filter-out tests/CollisionTest.cpp,${COLLISION_TEST_SRCS})

kinematic-mover-test: ${GLADE_BUILD_DIR}/kinematic-mover-test
	${GLADE_BUILD_DIR}/kinematic-mover-test

${GLADE_BUILD_DIR}/kinematic-mover-test: ${KINEMATIC_TEST_SRCS} ${COLLISION_TEST_HEADERS}
	mkdir -p $(@D)
	${CXX} -std=c++20 -O0 -g -Iinclude ${KINEMATIC_TEST_SRCS} -o $@

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

${GLADE_BUILD_DIR}/util/GladeTexAtlasReader.o: src/util/GladeTexAtlasReader.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/rapidjson/include -c $< -o $@

${GLADE_BUILD_DIR}/generation/noise.o: src/generation/noise.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/PerlinNoise/include -c $< -o $@

${GLADE_BUILD_DIR}/net/NetworkManager.o: src/net/NetworkManager.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${VENDOR}/enet/include -c $< -o $@

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

.NOTPARALLEL:
