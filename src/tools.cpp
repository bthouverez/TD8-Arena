#include "tools.hpp"

#include <cmath>


inline const Vector & gradient3D(int index)
{
  static const float zero = 0.0f;
  static const float one = 1.0f;
  // The 3d gradients are the midpoints of the vertices of a cube.
  static const Vector gradient[16] = {
      {one,one,zero}, {-one,one,zero}, {one,-one,zero}, {-one,-one,zero},
      {one,zero,one}, {-one,zero,one}, {one,zero,-one}, {-one,zero,-one},
      {zero,one,one}, {zero,-one,one}, {zero,one,-one}, {zero,-one,-one},
      {one,one,zero}, {-one,one,zero}, {zero,one,-one}, {zero,-one,-one}
  }; 
  return gradient[index];
} 

/* (tool) floating point modulo 289 */
static inline float mod289f(float value){  
  return (value - 289.0f*std::floor(value/289.0f));
}

/* (tool) floating point hash permute */
static inline float permute(float value){
  return mod289f(value * (1.0f + (34.0f*value)));
}

/**
 * @brief Bruit de Simplexe 3D (Ken Perlin)
 * @return float entre [-1.0, 1.0]
 */
float simplexNoise3D(const Vector &position)
{
  constexpr float zero = 0;
  constexpr float one = 1.0;
  constexpr float cst_skew   = 1.0/3.0;// skew factor in 3D,
  constexpr float cst_unskew = 1.0/6.0;// unskew factor
  constexpr float cst_contrib = 0.6;
  const Vector CORNER_OFFSETS[6] = { {one, zero, zero}, {zero, one, zero}, {zero, zero, one}
                                             ,{one, one, zero }, {one, zero, one }, {zero, one, one } };    
  
  // Skew/Unskew the input space to determine which simplex corner1 we're in :    
  Vector corner1 = position;
  float a = (position.x+position.y+position.z)*cst_skew; 
  corner1.x += a;
  corner1.y += a;
  corner1.z += a;
  corner1.x = std::floor(corner1.x);
  corner1.y = std::floor(corner1.y);
  corner1.z = std::floor(corner1.z);// in discrete (i,j,k) coords    
  Vector cornerOffset1 = position;
  cornerOffset1 = cornerOffset1 - corner1;// The x,y,z offsets from the base corner
  a = (corner1.x+corner1.y+corner1.z)*cst_unskew;
  cornerOffset1.x += a;
  cornerOffset1.y += a;
  cornerOffset1.z += a;
  corner1.x = mod289f(corner1.x);
  corner1.y = mod289f(corner1.y);
  corner1.z = mod289f(corner1.z);    

  // For the 3d case, the simplex shape is a slightly irregular tetrahedron.      
  unsigned char i2, i3;
  if(cornerOffset1.x >= cornerOffset1.y) {
    if(cornerOffset1.y >= cornerOffset1.z) {// X Y Z order
      i2 = 0; // 1 0 0
      i3 = 3; // 1 1 0
    } else if(cornerOffset1.x >= cornerOffset1.z) {// X Z Y order
      i2 = 0; // 1 0 0
      i3 = 4; // 1 0 1
    } else {// Z X Y order
      i2 = 2; // 0 0 1
      i3 = 4; // 1 0 1
    } 
  } else { 
    if(cornerOffset1.y < cornerOffset1.z) {// Z Y X order
      i2 = 2; // 0 0 1
      i3 = 5; // 0 1 1
    } else if(cornerOffset1.x < cornerOffset1.z) {// Y Z X order
      i2 = 1; // 0 1 0
      i3 = 5; // 0 1 1
    } else {// Y X Z order
      i2 = 1; // 0 1 0
      i3 = 3; // 1 1 0
    } 
  }
  Vector corner2 = CORNER_OFFSETS[i2];// Offsets of the second corner of the simplex in discrete (i,j,k) coords 
  Vector corner3 = CORNER_OFFSETS[i3];// offsets of the third corner.
  
  // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
  // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
  // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where c = cst_unskew = 1/6.  
  Vector cornerOffset4 = cornerOffset1;
  cornerOffset4.x += cst_unskew;// Offsets to the fourth corner in (x,y,z) coords           
  cornerOffset4.y += cst_unskew;
  cornerOffset4.z += cst_unskew;
  Vector cornerOffset2 = cornerOffset4;
  cornerOffset2 = cornerOffset2 - corner2;    // offsets to the second corner in (x,y,z) coords  
  cornerOffset4.x += cst_unskew; 
  cornerOffset4.y += cst_unskew; 
  cornerOffset4.z += cst_unskew; 
  Vector cornerOffset3 = cornerOffset4;
  cornerOffset3 = cornerOffset3 - corner3;    // offsets to the third corner in (x,y,z) coords  
  cornerOffset4.x += cst_unskew - one;   
  cornerOffset4.y += cst_unskew - one;   
  cornerOffset4.z += cst_unskew - one;   

  // Hash 4 gradients and calculate the 4 corners contributions :    
  int gi1 = static_cast<int>(permute(corner1.x + permute(corner1.y + permute(corner1.z)))) % 16;
  int gi4 = static_cast<int>(permute(corner1.x + one + permute(corner1.y + one + permute(corner1.z + one)))) % 16;
  corner2 = corner2 + corner1;  
  corner3 = corner3 + corner1;
  int gi2 = static_cast<int>(permute(corner2.x + permute(corner2.y + permute(corner2.z)))) % 16;  
  int gi3 = static_cast<int>(permute(corner3.x + permute(corner3.y + permute(corner3.z)))) % 16;  
  float n1, n2, n3, n4;//noise contributions
  n1 = cst_contrib - dot(cornerOffset1, cornerOffset1);
  n2 = cst_contrib - dot(cornerOffset2, cornerOffset2);
  n3 = cst_contrib - dot(cornerOffset3, cornerOffset3);
  n4 = cst_contrib - dot(cornerOffset4, cornerOffset4);
  if(n1 < zero) n1 = zero;
  else {
    n1 *= n1;
    n1 *= n1 * dot(gradient3D(gi1), cornerOffset1);
  }
  if(n4 < zero) n4 = zero;
  else {
    n4 *= n4;
    n4 *= n4 * dot(gradient3D(gi4), cornerOffset4);
  }  
  if(n2 < zero) n2 = zero;
  else {
    n2 *= n2;
    n2 *= n2 * dot(gradient3D(gi2), cornerOffset2);
  }
  if(n3 < zero) n3 = zero;
  else {
    n3 *= n3;
    n3 *= n3 * dot(gradient3D(gi3), cornerOffset3);
  }
  
  //accumulate and scale to fit [-1,1]
  return 32.0f*(n1 + n2 + n3 + n4);
}


float fractalSimplex3D(int octaves, float persistence, float lacunarity, float frequency, const Vector &position)
{
  constexpr float zero = 0.0f;
  constexpr float one = 1.0f;
  float noise = zero;  
  float amplitude = one;  
  float maxAmp = zero;
  Vector pos = position;
  for(int i=0; i < octaves; ++i)
  {
    pos = frequency * position;
    noise += amplitude * simplexNoise3D(pos);
    maxAmp += amplitude;    
    amplitude *= persistence;
    frequency *= lacunarity;    
  }
  return ((one + noise/maxAmp)*0.5f);
}
