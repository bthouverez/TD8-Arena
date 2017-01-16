#ifndef TOOLS_HEADER
#define TOOLS_HEADER


inline float getRandomFloat(float val_min, float val_max)
{
  // tO do : utiliser mt1997
  float r = (rand() % (1024*1024)) / (1024.0f*1024.0f - 1.0f);
  return val_min + r * (val_max - val_min);
}


#endif
