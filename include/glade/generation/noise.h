#pragma once
#include <glade/generation/AdvancedMeshGenerator.h>

class Grid;

void seed();
float height_function1(float x, float z);
float height_function2(float x, float z);
float height_function3(float x, float z);
float noise3_d(float x, float y, float z);
float height_function4(float x, float z,
                       const AdvancedMeshGenerator::TerrainGeneratorSettings &settings);
float height_function5(
    float x, float z, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings);
float isosurface_from_height_map(float y, float height_value, float max_height, float isolevel);

void perlin_test();
