#include "glade/generation/AdvancedMeshGenerator.h"
#include <glade/generation/noise.h>
#include <glade/generation/Grid.h>
#include <glade/debug/log.h>

#include <PerlinNoise/PerlinNoise.hpp>

#include <cmath>
#include <ctime>
#include <random>

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

float heightFunction4(float x, float z, const AdvancedMeshGenerator::TerrainGeneratorSettings &settings)
{
  float noise = perlin.octave2D_01(x / settings.wavelength, z / settings.wavelength, settings.octaves);
  noise = std::pow(noise, settings.power);
  return noise * settings.maxHeight;
}

float heightFunction5(float x, float z, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &octaves)
{
  float noise = 0.0;

  for (auto &settings: octaves) {
    float octave = perlin.noise2D_01(x / settings.wavelength, z / settings.wavelength);
    octave = std::pow(octave, settings.power);
    octave *= settings.maxHeight;
    noise += octave;
  }

  return std::clamp(noise, 0.0f, octaves[0].maxHeight);
}

float isosurfaceFromHeightMap(float y, float heightValue, float maxHeight, float isolevel)
{
  y /= maxHeight;
  // clamp gives a uniform distrubution of iso values along Y only if isolevel == 0.5
  return std::clamp(isolevel + (y - heightValue) / 2, 0.0f, 1.0f);
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