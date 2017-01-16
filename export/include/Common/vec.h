#ifndef _VEC_H
#define _VEC_H

#include <math.h>
#include <stdio.h>

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif

typedef float VEC[4];
typedef float VEC2[2];
typedef float VEC3[3];

#define VEC4 VEC

typedef VEC AABOX[2];


/* VEC2 */
static inline void vec2_init(float *v, float x, float y) 
{ 
    v[0]= x; 
    v[1]= y; 
}

static inline void vec2_zero(float *v) 
{ 
    v[0]= 0.f; 
    v[1]= 0.f; 
}

static inline void vec2_add(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] + v2[0]; 
    dst[1]= v1[1] + v2[1]; 
}

static inline void vec2_sub(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] - v2[0]; 
    dst[1]= v1[1] - v2[1]; 
}

static inline void vec2_mul(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] * v2[0]; 
    dst[1]= v1[1] * v2[1]; 
}

static inline void vec2_div(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] / v2[0]; 
    dst[1]= v1[1] / v2[1]; 
}

static inline void vec2_copy(float *dst, const float *v) 
{ 
    dst[0]= v[0]; 
    dst[1]= v[1]; 
}

static inline float vec2_dot(const float *u, const float *v) 
{
    return u[0] * v[0] + u[1] * v[1];
}    

static inline float vec2_cross(const float *u, const float *v) 
{
    return u[0] * v[1] - u[1] * v[0];
}    

static inline void vec2_swap(float *u, float *v)
{
    float t[2];
    vec2_copy(t, u);
    vec2_copy(u, v);
    vec2_copy(v, t);
}

static inline float vec2_length2(const float *v) 
{
    return vec2_dot(v, v);
}

static inline float vec2_length(const float *v)
{
    return sqrtf(vec2_dot(v, v));
}

static inline void vec2_const_mul(float *dst, float k, const float *v) 
{ 
    dst[0]= k*v[0]; 
    dst[1]= k*v[1]; 
}

static inline void vec2_norm(float *dst, const float *v) 
{ 
    float d = 1.f / vec2_length(v); 
    vec2_const_mul(dst, d, v); 
}

static inline void vec2_opposite(float *dst, const float *v) 
{
    dst[0] = -v[0];
    dst[1] = -v[1];
}

static inline void vec2_mix(float *dst, const float *v0, const float *v1, const float k) 
{ 
    dst[0] = v0[0] + k*(v1[0] - v0[0]); 
    dst[1] = v0[1] + k*(v1[1] - v0[1]); 
}

/* VEC */
static inline void vec4_init(float *v, float x, float y, float z, float w) 
{ 
    v[0]= x; 
    v[1]= y; 
    v[2]= z; 
    v[3]= w; 
}

static inline void vec4_init_v3dir(float *v, const float *v3)
{
    vec4_init(v, v3[0], v3[1], v3[2], 0.f);
}

static inline void vec4_init_v3pos(float *v, const float *v3)
{
    vec4_init(v, v3[0], v3[1], v3[2], 1.f);
}

static inline void vec4_zero(float *v) 
{ 
    v[0]= 0.f; 
    v[1]= 0.f; 
    v[2]= 0.f; 
    v[3]= 0.f; 
}

static inline void vec4_copy(float *dst, const float *v) 
{ 
    dst[0]= v[0]; 
    dst[1]= v[1]; 
    dst[2]= v[2]; 
    dst[3]= v[3]; 
}

static inline void vec4_add(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] + v2[0]; 
    dst[1]= v1[1] + v2[1]; 
    dst[2]= v1[2] + v2[2]; 
    dst[3]= v1[3] + v2[3]; 
}

static inline void vec4_sub(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] - v2[0]; 
    dst[1]= v1[1] - v2[1]; 
    dst[2]= v1[2] - v2[2]; 
    dst[3]= v1[3] - v2[3]; 
}

static inline void vec4_mul(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] * v2[0]; 
    dst[1]= v1[1] * v2[1]; 
    dst[2]= v1[2] * v2[2]; 
    dst[3]= v1[3] * v2[3]; 
}

static inline void vec4_div(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] / v2[0]; 
    dst[1]= v1[1] / v2[1]; 
    dst[2]= v1[2] / v2[2]; 
    dst[3]= v1[3] / v2[3]; 
}

static inline void vec4_const_mul(float *dst, const float k, const float *v) 
{ 
    dst[0]= k*v[0]; 
    dst[1]= k*v[1]; 
    dst[2]= k*v[2]; 
    dst[3]= k*v[3]; 
}

static inline void vec4_add_const_mul(
    float *dst, const float *p, float k, const float *v) 
{ 
    dst[0]= p[0] + k*v[0]; 
    dst[1]= p[1] + k*v[1]; 
    dst[2]= p[2] + k*v[2]; 
    dst[3]= p[3] + k*v[3]; 
}

static inline float vec4_dot(const float *v1, const float *v2) 
{
    return v1[0]*v2[0] 
        + v1[1]*v2[1] 
        + v1[2]*v2[2]
        + v1[3]*v2[3];
}    

static inline float vec4_length2(const float *v0)
{
    return vec4_dot(v0, v0);
}

static inline float vec4_length(const float *v0)
{
    return sqrtf(vec4_length2(v0));
}

static inline void vec4_norm(float *dst, const float *v) 
{ 
    float d = 1.f / vec4_length(v); 
    vec4_const_mul(dst, d, v); 
}

static inline void vec4_mix(float *dst, const float *v0, const float *v1, const float k) 
{ 
    dst[0] = v0[0] + k*(v1[0] - v0[0]); 
    dst[1] = v0[1] + k*(v1[1] - v0[1]); 
    dst[2] = v0[2] + k*(v1[2] - v0[2]); 
    dst[3] = v0[3] + k*(v1[3] - v0[3]); 
}

static inline void vec4_print(const float *vec)
{
    printf("[%f %f %f %f]\n", vec[0], vec[1], vec[2], vec[3]);
}


/* VEC3 */
static inline void vec3_init(float *v, const float x, const float y, const float z) 
{ 
    v[0]= x; 
    v[1]= y; 
    v[2]= z; 
}

static inline void vec3_zero(float *v) 
{ 
    v[0]= 0.f; 
    v[1]= 0.f; 
    v[2]= 0.f; 
}

static inline void vec3_copy(float *dst, const float *v) 
{ 
    dst[0]= v[0]; 
    dst[1]= v[1]; 
    dst[2]= v[2]; 
}

static inline void vec3_neg(float *dst, const float *v) 
{ 
    dst[0]= -v[0]; 
    dst[1]= -v[1]; 
    dst[2]= -v[2]; 
}

static inline float vec3_dot(const float *v1, const float *v2) 
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}    

static inline float vec3_length2(const float *v) 
{
    return vec3_dot(v, v);
}

static inline float vec3_length(const float *v)
{
    return sqrtf(vec3_dot(v, v));
}

static inline void vec3_cross(float *dst, const float *v1, const float *v2) 
{ 
    //~ float tmp[3];
    dst[0]= v1[1]*v2[2] - v1[2]*v2[1]; 
    dst[1]= v1[2]*v2[0] - v1[0]*v2[2]; 
    dst[2]= v1[0]*v2[1] - v1[1]*v2[0]; 
    //~ vec3_copy(dst, tmp);
}

static inline void vec3_sub(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] - v2[0]; 
    dst[1]= v1[1] - v2[1]; 
    dst[2]= v1[2] - v2[2]; 
}

static inline void vec3_add(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] + v2[0]; 
    dst[1]= v1[1] + v2[1]; 
    dst[2]= v1[2] + v2[2]; 
}

static inline void vec3_mul(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] * v2[0]; 
    dst[1]= v1[1] * v2[1]; 
    dst[2]= v1[2] * v2[2]; 
}

static inline void vec3_div(float *dst, const float *v1, const float *v2) 
{ 
    dst[0]= v1[0] / v2[0]; 
    dst[1]= v1[1] / v2[1]; 
    dst[2]= v1[2] / v2[2]; 
}

static inline void vec3_add_const_mul(
    float *dst, const float *p, float k, const float *v) 
{ 
    dst[0]= p[0] + k*v[0]; 
    dst[1]= p[1] + k*v[1]; 
    dst[2]= p[2] + k*v[2]; 
}

static inline void vec3_const_mul(float *dst, float k, const float *v) 
{ 
    dst[0]= k*v[0]; 
    dst[1]= k*v[1]; 
    dst[2]= k*v[2]; 
}

static inline void vec3_norm(float *dst, const float *v) 
{ 
    float d = 1.f / vec3_length(v); 
    vec3_const_mul(dst, d, v); 
}

static inline void vec3_mirror(float *dst, float *n, const float *v) 
{ 
    float d= 2.f * vec3_dot(n, v); 
    dst[0]= d*n[0] - v[0]; 
    dst[1]= d*n[1] - v[1]; 
    dst[2]= d*n[2] - v[2]; 
    vec3_norm(dst, dst); 
}

static inline void vec3_inverse(float *dst, const float *v) 
{
    dst[0] = 1.f / v[0];
    dst[1] = 1.f / v[1];
    dst[2] = 1.f / v[2];
}

static inline void vec3_opposite(float *dst, const float *v) 
{
    dst[0] = -v[0];
    dst[1] = -v[1];
    dst[2] = -v[2];
}

static inline void vec3_lerp(float *dst, const float k, const float *v0, const float *v1) 
{ 
    dst[0] = v0[0] + k*(v1[0] - v0[0]); 
    dst[1] = v0[1] + k*(v1[1] - v0[1]); 
    dst[2] = v0[2] + k*(v1[2] - v0[2]); 
}

static inline void vec3_mix(float *dst, const float *v0, const float *v1, const float k) 
{ 
    dst[0] = v0[0] + k*(v1[0] - v0[0]); 
    dst[1] = v0[1] + k*(v1[1] - v0[1]); 
    dst[2] = v0[2] + k*(v1[2] - v0[2]); 
}

static inline int vec3_max(const float *vec)
{
    if(fabs(vec[0]) >= fabs(vec[1])
    && fabs(vec[0]) >= fabs(vec[2]))
        return 0;
    else if(fabs(vec[1]) >= fabs(vec[0])
         && fabs(vec[1]) >= fabs(vec[2]))
        return 1;
    else
        return 2;
}

static inline int vec3_min(const float *vec)
{
    if(fabs(vec[0]) <= fabs(vec[1])
    && fabs(vec[0]) <= fabs(vec[2]))
        return 0;
    else if(fabs(vec[1]) <= fabs(vec[0])
         && fabs(vec[1]) <= fabs(vec[2]))
        return 1;
    else
        return 2;
}

static inline void vec3_print(const float *vec)
{
    printf("[%f %f %f]\n", vec[0], vec[1], vec[2]);
}

#endif

