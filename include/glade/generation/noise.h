#pragma once

class Grid;

void seed();
float heightFunction1(float x, float z);
float heightFunction2(float x, float z);
float heightFunction3(float x, float z);
float noise3D(float x, float y, float z);
float heightFunction4(float x, float z);
float isosurfaceFromHeightMap(float y, float heightValue, float maxHeight, float isolevel);

void perlinTest();
