#include "renderable_asteroid.hpp"
#include "tools.hpp"


bool RenderableAsteroid::init()
{
  const int octaves = 6;
  const float persistence = 0.5f;
  const float lacunarity = 2.02f;
  const float frequency = 0.5f;
  const float size = (float)CUBOID_SUBDIV;

  m_fbmSeed = Vector(getRandomFloat(0.0f, 87.7f), getRandomFloat(0.0f, 83.5f), getRandomFloat(0.0f, 81.7f)); 

  // Face du cube en x positif :
  float * xpos = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];//positions
  float * xpos_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];//normales
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = 1.0f;
      p.y = -1.0f + 2.0f*i/size; 
      p.z = -1.0f + 2.0f*j/size; 
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 0] = p.x;
      xpos[index + 1] = p.y;
      xpos[index + 2] = p.z;

      p.x = 1.0f;
      p.y = -1.0f + 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*j/size; 
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 3] = p.x;
      xpos[index + 4] = p.y;
      xpos[index + 5] = p.z;       

      p.x = 1.0f;
      p.y = -1.0f + 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*(j+1)/size; 
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 6] = p.x;
      xpos[index + 7] = p.y;
      xpos[index + 8] = p.z;            

      // triangle 2:
      p.x = 1.0f;
      p.y = -1.0f + 2.0f*i/size;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 9] = p.x;
      xpos[index + 10] = p.y;
      xpos[index + 11] = p.z; 

      p.x = 1.0f;
      p.y = -1.0f + 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 12] = p.x;
      xpos[index + 13] = p.y;
      xpos[index + 14] = p.z; 

      p.x = 1.0f;
      p.y = -1.0f + 2.0f*i/size;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xpos[index + 15] = p.x;
      xpos[index + 16] = p.y;
      xpos[index + 17] = p.z; 

      // normales:
      Vector e1;
      e1.x = xpos[index + 3] - xpos[index + 0];
      e1.y = xpos[index + 4] - xpos[index + 1];
      e1.z = xpos[index + 5] - xpos[index + 2];
      Vector e2;
      e2.x = xpos[index + 6] - xpos[index + 0];
      e2.y = xpos[index + 7] - xpos[index + 1];
      e2.z = xpos[index + 8] - xpos[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      xpos_n[index + 0] = xpos_n[index + 3] = xpos_n[index + 6] = n.x;
      xpos_n[index + 1] = xpos_n[index + 4] = xpos_n[index + 7] = n.y;
      xpos_n[index + 2] = xpos_n[index + 5] = xpos_n[index + 8] = n.z;

      e1.x = xpos[index + 12] - xpos[index + 9];
      e1.y = xpos[index + 13] - xpos[index + 10];
      e1.z = xpos[index + 14] - xpos[index + 11];      

      e2.x = xpos[index + 15] - xpos[index + 9];
      e2.y = xpos[index + 16] - xpos[index + 10];
      e2.z = xpos[index + 17] - xpos[index + 11];

      n = normalize(cross(e1, e2));
      xpos_n[index + 9] = xpos_n[index + 12] = xpos_n[index + 15] = n.x;
      xpos_n[index + 10] = xpos_n[index + 13] = xpos_n[index + 16] = n.y;
      xpos_n[index + 11] = xpos_n[index + 14] = xpos_n[index + 17] = n.z;
    }

  // Face du cube en x négatif :    
  float * xneg = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  float * xneg_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = -1.0f;
      p.y = 1.0f - 2.0f*i/size;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 0] = p.x;
      xneg[index + 1] = p.y;
      xneg[index + 2] = p.z;

      p.x = -1.0f;
      p.y = 1.0f - 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 3] = p.x;
      xneg[index + 4] = p.y;
      xneg[index + 5] = p.z;

      p.x = -1.0f;
      p.y = 1.0f - 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 6] = p.x;
      xneg[index + 7] = p.y;
      xneg[index + 8] = p.z;

      // triangle 2:
      p.x = -1.0f;
      p.y = 1.0f - 2.0f*i/size; 
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 9] = p.x;
      xneg[index + 10] = p.y;
      xneg[index + 11] = p.z;

      p.x = -1.0f;
      p.y = 1.0f - 2.0f*(i+1)/size;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 12] = p.x;
      xneg[index + 13] = p.y;
      xneg[index + 14] = p.z;

      p.x = -1.0f;
      p.y = 1.0f - 2.0f*i/size;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      xneg[index + 15] = p.x;
      xneg[index + 16] = p.y;
      xneg[index + 17] = p.z;
            
      // normales:
      Vector e1;
      e1.x = xneg[index + 3] - xneg[index + 0];
      e1.y = xneg[index + 4] - xneg[index + 1];
      e1.z = xneg[index + 5] - xneg[index + 2];
      Vector e2;
      e2.x = xneg[index + 6] - xneg[index + 0];
      e2.y = xneg[index + 7] - xneg[index + 1];
      e2.z = xneg[index + 8] - xneg[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      xneg_n[index + 0] = xneg_n[index + 3] = xneg_n[index + 6] = n.x;
      xneg_n[index + 1] = xneg_n[index + 4] = xneg_n[index + 7] = n.y;
      xneg_n[index + 2] = xneg_n[index + 5] = xneg_n[index + 8] = n.z;

      e1.x = xneg[index + 12] - xneg[index + 9];
      e1.y = xneg[index + 13] - xneg[index + 10];
      e1.z = xneg[index + 14] - xneg[index + 11];      

      e2.x = xneg[index + 15] - xneg[index + 9];
      e2.y = xneg[index + 16] - xneg[index + 10];
      e2.z = xneg[index + 17] - xneg[index + 11];

      n = normalize(cross(e1, e2));
      xneg_n[index + 9] = xneg_n[index + 12] = xneg_n[index + 15] = n.x;
      xneg_n[index + 10] = xneg_n[index + 13] = xneg_n[index + 16] = n.y;
      xneg_n[index + 11] = xneg_n[index + 14] = xneg_n[index + 17] = n.z;
    }
  
  // Face du cube en y positif :
  float * ypos = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  float * ypos_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = 1.0f - 2.0f*i/size;
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 0] = p.x;
      ypos[index + 1] = p.y;
      ypos[index + 2] = p.z;

      p.x = 1.0f - 2.0f*(i+1)/size;
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 3] = p.x;
      ypos[index + 4] = p.y;
      ypos[index + 5] = p.z;
      
      p.x = 1.0f - 2.0f*(i+1)/size; 
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 6] = p.x;
      ypos[index + 7] = p.y;
      ypos[index + 8] = p.z;

      // triangle 2:
      p.x = 1.0f - 2.0f*i/size;
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 9] = p.x;
      ypos[index + 10] = p.y;
      ypos[index + 11] = p.z;

      p.x = 1.0f - 2.0f*(i+1)/size; 
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 12] = p.x;
      ypos[index + 13] = p.y;
      ypos[index + 14] = p.z;

      p.x = 1.0f - 2.0f*i/size; 
      p.y = 1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size; 
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      ypos[index + 15] = p.x;
      ypos[index + 16] = p.y;
      ypos[index + 17] = p.z;

      // normales:
      Vector e1;
      e1.x = ypos[index + 3] - ypos[index + 0];
      e1.y = ypos[index + 4] - ypos[index + 1];
      e1.z = ypos[index + 5] - ypos[index + 2];
      Vector e2;
      e2.x = ypos[index + 6] - ypos[index + 0];
      e2.y = ypos[index + 7] - ypos[index + 1];
      e2.z = ypos[index + 8] - ypos[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      ypos_n[index + 0] = ypos_n[index + 3] = ypos_n[index + 6] = n.x;
      ypos_n[index + 1] = ypos_n[index + 4] = ypos_n[index + 7] = n.y;
      ypos_n[index + 2] = ypos_n[index + 5] = ypos_n[index + 8] = n.z;

      e1.x = ypos[index + 12] - ypos[index + 9];
      e1.y = ypos[index + 13] - ypos[index + 10];
      e1.z = ypos[index + 14] - ypos[index + 11];      

      e2.x = ypos[index + 15] - ypos[index + 9];
      e2.y = ypos[index + 16] - ypos[index + 10];
      e2.z = ypos[index + 17] - ypos[index + 11];

      n = normalize(cross(e1, e2));
      ypos_n[index + 9] = ypos_n[index + 12] = ypos_n[index + 15] = n.x;
      ypos_n[index + 10] = ypos_n[index + 13] = ypos_n[index + 16] = n.y;
      ypos_n[index + 11] = ypos_n[index + 14] = ypos_n[index + 17] = n.z;           
    }

  // Face du cube en y négatif :    
  float * yneg = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  float * yneg_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 0] = p.x;
      yneg[index + 1] = p.y;
      yneg[index + 2] = p.z;

      p.x = -1.0f + 2.0f*(i+1)/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 3] = p.x;
      yneg[index + 4] = p.y;
      yneg[index + 5] = p.z;

      p.x = -1.0f + 2.0f*(i+1)/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 6] = p.x;
      yneg[index + 7] = p.y;
      yneg[index + 8] = p.z;

      // triangle 2:
      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*j/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 9] = p.x;
      yneg[index + 10] = p.y;
      yneg[index + 11] = p.z;

      p.x = -1.0f + 2.0f*(i+1)/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 12] = p.x;
      yneg[index + 13] = p.y;
      yneg[index + 14] = p.z;

      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f;
      p.z = -1.0f + 2.0f*(j+1)/size;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      yneg[index + 15] = p.x;
      yneg[index + 16] = p.y;
      yneg[index + 17] = p.z;

      // normales:
      Vector e1;
      e1.x = yneg[index + 3] - yneg[index + 0];
      e1.y = yneg[index + 4] - yneg[index + 1];
      e1.z = yneg[index + 5] - yneg[index + 2];
      Vector e2;
      e2.x = yneg[index + 6] - yneg[index + 0];
      e2.y = yneg[index + 7] - yneg[index + 1];
      e2.z = yneg[index + 8] - yneg[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      yneg_n[index + 0] = yneg_n[index + 3] = yneg_n[index + 6] = n.x;
      yneg_n[index + 1] = yneg_n[index + 4] = yneg_n[index + 7] = n.y;
      yneg_n[index + 2] = yneg_n[index + 5] = yneg_n[index + 8] = n.z;

      e1.x = yneg[index + 12] - yneg[index + 9];
      e1.y = yneg[index + 13] - yneg[index + 10];
      e1.z = yneg[index + 14] - yneg[index + 11];      

      e2.x = yneg[index + 15] - yneg[index + 9];
      e2.y = yneg[index + 16] - yneg[index + 10];
      e2.z = yneg[index + 17] - yneg[index + 11];

      n = normalize(cross(e1, e2));
      yneg_n[index + 9] = yneg_n[index + 12] = yneg_n[index + 15] = n.x;
      yneg_n[index + 10] = yneg_n[index + 13] = yneg_n[index + 16] = n.y;
      yneg_n[index + 11] = yneg_n[index + 14] = yneg_n[index + 17] = n.z;           
    }
  
  
  // Face du cube en z positif :
  float * zpos = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  float * zpos_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f + 2.0f*j/size; 
      p.z = 1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 0] = p.x;
      zpos[index + 1] = p.y;
      zpos[index + 2] = p.z;

      p.x = -1.0f + 2.0f*(i+1)/size; 
      p.y = -1.0f + 2.0f*j/size;
      p.z = 1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 3] = p.x;
      zpos[index + 4] = p.y;
      zpos[index + 5] = p.z;
      
      p.x = -1.0f + 2.0f*(i+1)/size;
      p.y = -1.0f + 2.0f*(j+1)/size; 
      p.z = 1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 6] = p.x;
      zpos[index + 7] = p.y;
      zpos[index + 8] = p.z;

      
      // triangle 2:
      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f + 2.0f*j/size;
      p.z = 1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 9] = p.x;
      zpos[index + 10] = p.y;
      zpos[index + 11] = p.z;

      p.x = -1.0f + 2.0f*(i+1)/size; 
      p.y = -1.0f + 2.0f*(j+1)/size; 
      p.z = 1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 12] = p.x;
      zpos[index + 13] = p.y;
      zpos[index + 14] = p.z;

      p.x = -1.0f + 2.0f*i/size; 
      p.y = -1.0f + 2.0f*(j+1)/size; 
      p.z = 1.0f; 
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zpos[index + 15] = p.x;
      zpos[index + 16] = p.y;
      zpos[index + 17] = p.z;

      // normales:
      Vector e1;
      e1.x = zpos[index + 3] - zpos[index + 0];
      e1.y = zpos[index + 4] - zpos[index + 1];
      e1.z = zpos[index + 5] - zpos[index + 2];
      Vector e2;
      e2.x = zpos[index + 6] - zpos[index + 0];
      e2.y = zpos[index + 7] - zpos[index + 1];
      e2.z = zpos[index + 8] - zpos[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      zpos_n[index + 0] = zpos_n[index + 3] = zpos_n[index + 6] = n.x;
      zpos_n[index + 1] = zpos_n[index + 4] = zpos_n[index + 7] = n.y;
      zpos_n[index + 2] = zpos_n[index + 5] = zpos_n[index + 8] = n.z;

      e1.x = zpos[index + 12] - zpos[index + 9];
      e1.y = zpos[index + 13] - zpos[index + 10];
      e1.z = zpos[index + 14] - zpos[index + 11];      

      e2.x = zpos[index + 15] - zpos[index + 9];
      e2.y = zpos[index + 16] - zpos[index + 10];
      e2.z = zpos[index + 17] - zpos[index + 11];

      n = normalize(cross(e1, e2));
      zpos_n[index + 9] = zpos_n[index + 12] = zpos_n[index + 15] = n.x;
      zpos_n[index + 10] = zpos_n[index + 13] = zpos_n[index + 16] = n.y;
      zpos_n[index + 11] = zpos_n[index + 14] = zpos_n[index + 17] = n.z;            
    }

  // Face du cube en z négatif :    
  float * zneg = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  float * zneg_n = new float[CUBOID_SUBDIV*CUBOID_SUBDIV*2*3*3];
  for (int j=0; j < CUBOID_SUBDIV; ++j)
    for (int i=0; i < CUBOID_SUBDIV; ++i)
    {
      int index = 18 * (j*CUBOID_SUBDIV + i);

      // triangle 1:
      Vector p;
      p.x = 1.0f - 2.0f*i/size; 
      p.y = -1.0f + 2.0f*j/size;
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 0] = p.x;
      zneg[index + 1] = p.y;
      zneg[index + 2] = p.z;

      p.x = 1.0f - 2.0f*(i+1)/size; 
      p.y = -1.0f + 2.0f*j/size; 
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 3] = p.x;
      zneg[index + 4] = p.y;
      zneg[index + 5] = p.z;

      p.x = 1.0f - 2.0f*(i+1)/size;
      p.y = -1.0f + 2.0f*(j+1)/size;
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 6] = p.x;
      zneg[index + 7] = p.y;
      zneg[index + 8] = p.z;

      // triangle 2:
      p.x = 1.0f - 2.0f*i/size; 
      p.y = -1.0f + 2.0f*j/size;
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 9] = p.x;
      zneg[index + 10] = p.y;
      zneg[index + 11] = p.z;

      p.x = 1.0f - 2.0f*(i+1)/size; 
      p.y = -1.0f + 2.0f*(j+1)/size;
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 12] = p.x;
      zneg[index + 13] = p.y;
      zneg[index + 14] = p.z;

      p.x = 1.0f - 2.0f*i/size;
      p.y = -1.0f + 2.0f*(j+1)/size;
      p.z = -1.0f;
      p = normalize(p);
      p = p * (0.25f + 0.75f*fractalSimplex3D(octaves, persistence, lacunarity, frequency, p + m_fbmSeed));
      zneg[index + 15] = p.x;
      zneg[index + 16] = p.y;
      zneg[index + 17] = p.z;

      // normales:
      Vector e1;
      e1.x = zneg[index + 3] - zneg[index + 0];
      e1.y = zneg[index + 4] - zneg[index + 1];
      e1.z = zneg[index + 5] - zneg[index + 2];
      Vector e2;
      e2.x = zneg[index + 6] - zneg[index + 0];
      e2.y = zneg[index + 7] - zneg[index + 1];
      e2.z = zneg[index + 8] - zneg[index + 2];
      
      Vector n = normalize(cross(e1, e2));
      zneg_n[index + 0] = zneg_n[index + 3] = zneg_n[index + 6] = n.x;
      zneg_n[index + 1] = zneg_n[index + 4] = zneg_n[index + 7] = n.y;
      zneg_n[index + 2] = zneg_n[index + 5] = zneg_n[index + 8] = n.z;

      e1.x = zneg[index + 12] - zneg[index + 9];
      e1.y = zneg[index + 13] - zneg[index + 10];
      e1.z = zneg[index + 14] - zneg[index + 11];      

      e2.x = zneg[index + 15] - zneg[index + 9];
      e2.y = zneg[index + 16] - zneg[index + 10];
      e2.z = zneg[index + 17] - zneg[index + 11];

      n = normalize(cross(e1, e2));
      zneg_n[index + 9] = zneg_n[index + 12] = zneg_n[index + 15] = n.x;
      zneg_n[index + 10] = zneg_n[index + 13] = zneg_n[index + 16] = n.y;
      zneg_n[index + 11] = zneg_n[index + 14] = zneg_n[index + 17] = n.z;        
    }

  
  // Wrap Up + Colors:
  m_num_vertices = 6*CUBOID_SUBDIV*CUBOID_SUBDIV*2*3;
  
  float * vertices = new float[m_num_vertices*3];
  float * normals = new float[m_num_vertices*3];

  float * faces[6] = { xpos, xneg, ypos, yneg, zpos, zneg };
  float * faces_n[6] = { xpos_n, xneg_n, ypos_n, yneg_n, zpos_n, zneg_n };
  for (int face = 0; face < 6; ++face)
    for (int i=0; i < CUBOID_SUBDIV*CUBOID_SUBDIV*18; ++i)
    {
      vertices[face * CUBOID_SUBDIV*CUBOID_SUBDIV*18 + i] = faces[face][i];
      normals[face * CUBOID_SUBDIV*CUBOID_SUBDIV*18 + i] = faces_n[face][i];
    }

  float * colors = new float[m_num_vertices * 3];
  const Vector perlin_offset(31.3f, 42.7f, 51.3f);
  for (int i=0; i < m_num_vertices; ++i)
  {
    Vector p(vertices[3*i+0], vertices[3*i+1], vertices[3*i+2]);
    float gray = fractalSimplex3D(5, 0.67f, 2.02f, 2.5f, p + perlin_offset);
    //gray *= 0.5f;
    gray = 0.5f + 0.5f * gray;
    //gray = 1.0f;
    colors[3*i + 0] = gray;
    colors[3*i + 1] = gray;
    colors[3*i + 2] = gray;
  }

  // GL :
  m_usetexture = false;
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, 3*m_num_vertices * 3 * sizeof(float), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, m_num_vertices * 3 * sizeof(float), vertices);//coords
  glBufferSubData(GL_ARRAY_BUFFER, m_num_vertices * 3 * sizeof(float), m_num_vertices * 3 * sizeof(float), colors);//couleurs
  glBufferSubData(GL_ARRAY_BUFFER, 2*m_num_vertices* 3 * sizeof(float), m_num_vertices*3 * sizeof(float), normals);//normales
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);//coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(m_num_vertices*3 * sizeof(float)));//couleurs
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(2*m_num_vertices*3 * sizeof(float)));//normales
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Cleanup:
  delete[] xpos;
  delete[] xpos_n;
  delete[] xneg;
  delete[] xneg_n;
  delete[] ypos;
  delete[] ypos_n;
  delete[] yneg;
  delete[] yneg_n;
  delete[] zpos;
  delete[] zpos_n;
  delete[] zneg;
  delete[] zneg_n;
  delete[] vertices;
  delete[] normals;
  delete[] colors;

  return true;
}
