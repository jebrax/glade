#include <glade/generation/noise.h>
#include <glade/generation/Grid.h>
#include <glade/debug/log.h>

#include <PerlinNoise/PerlinNoise.hpp>

#include <cmath>
#include <ctime>

static const float ONE_METER_COORDS = 1.0;

static siv::PerlinNoise perlin { 13250756u };

void seed()
{
  ::srand(time(NULL));
  perlin.reseed(time(NULL));

}

float randRange(float from, float to)
{
  return from + (::rand() / (float) RAND_MAX) * (to - from);
}

float heightFunction1(float x, float z)
{
  return randRange(0.0f, 1.0f);
}

float heightFunction2(float x, float z)
{
  float noise = perlin.octave2D_01((x * 0.1), (z * 0.1), 6);
  return noise * 10.0;
}

float heightFunction3(float x, float z)
{
  float noise = perlin.octave2D_01((x * 0.1), (z * 0.1), 6);
  float diminisher = std::sin(x / 2.0);
  return std::pow(noise,diminisher) * 10.0;
}

float noise3D(float x, float y, float z)
{
  float freq = 0.6;
  return 1.0 * perlin.octave3D_01((x * freq), (y * freq), (z * freq), 8);
}

float isosurfaceHeightMap(float x, float y, float z, float maxHeightMeters)
{
  float wavelength = 20.0;
  int octaves = 6;
  float noise = perlin.octave2D_01(x / wavelength, z / wavelength, octaves);
  noise = std::pow(noise, 4.0);
  float maxY = maxHeightMeters * ONE_METER_COORDS;
  noise *= maxY;
  return (y - noise + maxY) / (2 * maxY);
}

float singleCube(float x, float y, float z, Grid &grid)
{
  Glade::Vector3i centerCellCoord(grid.chunkSizeCells / 2, 20, grid.chunkSizeCells / 2);

  if (grid.doesCubeVertBelongInTheCell(x, y, z, centerCellCoord)) {
    return 0.1;
  } else {
    return 0.8;
  }
}

void perlinTest()
{
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 5; y++) {
      float noise = perlin.octave2D_01(x, y, 6);
      log("(%d, %d): %f", y, x, noise);
    }
  }
}