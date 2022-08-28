#pragma once

void seed();

float heightFunction1(float x, float z);
float heightFunction2(float x, float z);
float heightFunction3(float x, float z);
float noise3D(float x, float y, float z);
float isosurfaceHeightMap(float x, float y, float z, float maxHeightMeters);
float singleCube(float x, float y, float z, float cellSize, int chunkSizeCells);

void perlinTest();
