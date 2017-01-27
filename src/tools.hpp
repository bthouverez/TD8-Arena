#ifndef TOOLS_HEADER
#define TOOLS_HEADER

#include <vec.h>



inline float getRandomFloat(float val_min, float val_max)
{
  // tO do : utiliser mt1997
  float r = (rand() % (1024*1024)) / (1024.0f*1024.0f - 1.0f);
  return val_min + r * (val_max - val_min);
}

/**
 * @brief Bruit de Simplexe 3D (Ken Perlin)
 * @return float entre [-1.0, 1.0]
 */
float simplexNoise3D(const Vector &position);

/**
 * @brief A weighted sum of simplex noises at a given 3D position.
 * @param octaves number of noise layers to accumulate
 * @param persistence ratio of the amplitude of noise layer (n+1) to the amplitude of noise layer (n) - commonly a number near 0.5
 * @param lacunarity ratio of the frequency  of noise layer (n+1) to the frequency of noise layer (n) - commonly a number near 2.0
 * @param frequency base frequency (ie, the lowest one among the noise layers)
 * @param position the 3d point where the noise is evaluated.
 * @return a number in [0, 1]
 */
float fractalSimplex3D(int octaves, float persistence, float lacunarity, float frequency, const Vector &position);

#endif
