#ifndef _MAT44_H
#define _MAT44_H

#include <stdio.h>
#include "vec.h"

#define M_PI       3.14159265358979323846

typedef float MAT44[16];
typedef float MAT33[9];		// 3x3 matrix

#define __M44(m, r, c) m[(c)*4 + (r)]
#define M33(m, r, c) m[(c)*3 + (r)]

/* les matrices sont stockées en colonne, ie au format OpenGL */ 

static float mat44_Id[16]= 
{
    1.f, 0.f, 0.f, 0.f,	// C0
    0.f, 1.f, 0.f, 0.f,	// C1
    0.f, 0.f, 1.f, 0.f,	// C2
    0.f, 0.f, 0.f, 1.f	// C3
};

/* a= b */
static inline void mat44_copy(float *a, const float *b)
{
    vec4_copy(&a[0], &b[0]);
    vec4_copy(&a[4], &b[4]);
    vec4_copy(&a[8], &b[8]);
    vec4_copy(&a[12], &b[12]);
}

/* a= 0 */
static inline void mat44_zero(float *a)
{
    vec4_zero(&a[0]);
    vec4_zero(&a[4]);
    vec4_zero(&a[8]);
    vec4_zero(&a[12]);
}

/* a= Id */
static inline void mat44_identity(float *a)
{
    mat44_copy(a, mat44_Id);
}

/* a= Id[3x3] */
static inline void mat44_identity33(float *a)
{
    vec4_copy(&a[0], &mat44_Id[0]);
    vec4_copy(&a[4], &mat44_Id[4]);
    vec4_copy(&a[8], &mat44_Id[8]);
    vec4_zero(&a[12]);
}

static inline void mat44_row4(float *a, const float *r0, const float *r1, const float *r2, const float *r3)
{
    __M44(a, 0, 0)= r0[0];
    __M44(a, 0, 1)= r0[1];
    __M44(a, 0, 2)= r0[2];
    __M44(a, 0, 3)= r0[3];
    
    __M44(a, 1, 0)= r1[0];
    __M44(a, 1, 1)= r1[1];
    __M44(a, 1, 2)= r1[2];
    __M44(a, 1, 3)= r1[3];
    
    __M44(a, 2, 0)= r2[0];
    __M44(a, 2, 1)= r2[1];
    __M44(a, 2, 2)= r2[2];
    __M44(a, 2, 3)= r2[3];
    
    __M44(a, 3, 0)= r3[0];
    __M44(a, 3, 1)= r3[1];
    __M44(a, 3, 2)= r3[2];
    __M44(a, 3, 3)= r3[3];
}

static inline void mat44_column4(float *a, const float *c0, const float *c1, const float *c2, const float *c3)
{
    __M44(a, 0, 0)= c0[0];
    __M44(a, 1, 0)= c0[1];
    __M44(a, 2, 0)= c0[2];
    __M44(a, 3, 0)= c0[3];
    
    __M44(a, 0, 1)= c1[0];
    __M44(a, 1, 1)= c1[1];
    __M44(a, 2, 1)= c1[2];
    __M44(a, 3, 1)= c1[3];
    
    __M44(a, 0, 2)= c2[0];
    __M44(a, 1, 2)= c2[1];
    __M44(a, 2, 2)= c2[2];
    __M44(a, 3, 2)= c2[3];
    
    __M44(a, 0, 3)= c3[0];
    __M44(a, 1, 3)= c3[1];
    __M44(a, 2, 3)= c3[2];
    __M44(a, 3, 3)= c3[3];
}

static inline void mat44_get_row4(const float *a, int r, float *v)
{
    vec4_init(v, 
        __M44(a, r, 0), 
        __M44(a, r, 1), 
        __M44(a, r, 2), 
        __M44(a, r, 3));
}

static inline void mat44_get_column4(const float *a, int c, float *v)
{
    vec4_init(v,
        __M44(a, 0, c),
        __M44(a, 1, c),
        __M44(a, 2, c),
        __M44(a, 3, c));
}


/* !!!  a = c * b  !!! */
/* (les matrices sont stoquées en colonnes) */
static inline void mat44_mul(float *a, const float *b, const float *c)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            a[i*4+j] = 
                b[i*4+0] * c[0*4+j] +
                b[i*4+1] * c[1*4+j] +
                b[i*4+2] * c[2*4+j] +
                b[i*4+3] * c[3*4+j];
        }
    }	
}

/* a = b * c */
static inline void mat44_mul2(float *a, const float *b, const float *c)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            a[i*4+j] = 
                c[i*4+0] * b[0*4+j] +
                c[i*4+1] * b[1*4+j] +
                c[i*4+2] * b[2*4+j] +
                c[i*4+3] * b[3*4+j];
        }
    }	
}

/* a= k * b */
static inline void mat44_const_mul(float *a, float k, const float *b)
{
    vec4_const_mul(&a[0], k, &b[0]);
    vec4_const_mul(&a[4], k, &b[4]);
    vec4_const_mul(&a[8], k, &b[8]);
    vec4_const_mul(&a[12], k, &b[12]);
}

/* !!! a = b * a !!! */
/* (les matrices sont stoquées en colonnes) */
static inline void mat44_compose(float *a, const float *b)
{
    float t[16];
    
    mat44_mul(t, a, b);
    mat44_copy(a, t);
}

/* a = a * b */
static inline void mat44_compose2(float *a, const float *b)
{
    float t[16];
    
    mat44_mul2(t, a, b);
    mat44_copy(a, t);
}


/* v= a * u */
static inline void mat44_mul_vec(float *v, const float *a, const float *u)
{
    int i;

    for (i=0; i < 4; i++) {
        v[i]= u[0]*a[0*4+i] 
            + u[1]*a[1*4+i]
            + u[2]*a[2*4+i]
            + u[3]*a[3*4+i];
    }
}

static inline void mat44_compose_vec(const float *a, float *u)
{
    float tmp[4];
    int i;

    for(i=0; i < 4; i++) {
        tmp[i]= u[0]*a[0*4+i] 
            + u[1]*a[1*4+i]
            + u[2]*a[2*4+i]
            + u[3]*a[3*4+i];
    }
    vec4_copy(u, tmp);
}

static inline void mat44_compose_dir(const float *a, float *u)
{
    float tmp[4];
    int i;

    for(i=0; i < 3; i++) 
        tmp[i]= u[0]*a[0*4+i] 
            + u[1]*a[1*4+i]
            + u[2]*a[2*4+i];
    vec4_copy(u, tmp);
}

/* a= u * u^T */
static inline void mat44_vecT(float *a, const float *v)
{
    int i;
    
    for(i= 0; i < 4; i++)
    {
        __M44(a, i, 0)= v[i] * v[0];
        __M44(a, i, 1)= v[i] * v[1];
        __M44(a, i, 2)= v[i] * v[2];
        __M44(a, i, 3)= v[i] * v[3];
    }
}

static inline void mat44_vec3T(float *a, const float *v)
{
    float t[4];
    
    vec4_init(t, v[0], v[1], v[2], 0.f);
    mat44_vecT(a, t);
}

/* a= b - c */
static inline void mat44_sub(float *a, const float *b, const float *c)
{
    vec4_sub(&a[0], &b[0], &c[0]);
    vec4_sub(&a[4], &b[4], &c[4]);
    vec4_sub(&a[8], &b[8], &c[8]);
    vec4_sub(&a[12], &b[12], &c[12]);
}

/* a= b + c */
static inline void mat44_add(float *a, const float *b, const float *c)
{
    vec4_add(&a[0], &b[0], &c[0]);
    vec4_add(&a[4], &b[4], &c[4]);
    vec4_add(&a[8], &b[8], &c[8]);
    vec4_add(&a[12], &b[12], &c[12]);
}

/* a= inverse(b) 
    adapted from mesa/GLU implementation
 */
static inline int mat44_inverse(float *inverse, const float *src)
{
    int i, j, k, swap;
    float t;
    float temp[4][4];

    for (i=0; i<4; i++) {
            for (j=0; j<4; j++) {
                temp[i][j] = src[i*4+j];
            }
    }

    mat44_identity(inverse);

    for (i = 0; i < 4; i++)
    {
        /* Look for largest element in column 
         */
        swap = i;
        for (j = i + 1; j < 4; j++)
        {
            if (fabs(temp[j][i]) > fabs(temp[i][i]))
                swap = j;
        }

        if (swap != i)
        {
            /* Swap rows.
             */
            for (k = 0; k < 4; k++)
            {
                t = temp[i][k];
                temp[i][k] = temp[swap][k];
                temp[swap][k] = t;

                t = inverse[i*4+k];
                inverse[i*4+k] = inverse[swap*4+k];
                inverse[swap*4+k] = t;
            }
        }

        if (fabs(temp[i][i]) < 1e-10)
        {
            /* No non-zero pivot.  The matrix is singular, which 
             shouldn't happen.  This means the user gave us a bad matrix.
             */
            return -1;
        }

        t = temp[i][i];
        for (k = 0; k < 4; k++)
        {
            temp[i][k] /= t;
            inverse[i*4+k] /= t;
        }
        
        for (j = 0; j < 4; j++)
        {
            if (j != i)
            {
                t = temp[j][i];
                for (k = 0; k < 4; k++)
                {
                    temp[j][k] -= temp[i][k]*t;
                    inverse[j*4+k] -= inverse[i*4+k]*t;
                }
            }
        }
    }
    
    return 0;
}

/*
	affiche en lignes (ordre naturel)
*/
static inline void mat44_print(const float *a)
{
    printf("row0  % -.8f  % -.8f  % -.8f  % -.8f\n", 
        __M44(a, 0, 0), __M44(a, 0, 1), __M44(a, 0, 2), __M44(a, 0, 3));
    printf("row1  % -.8f  % -.8f  % -.8f  % -.8f\n", 
        __M44(a, 1, 0), __M44(a, 1, 1), __M44(a, 1, 2), __M44(a, 1, 3));
    printf("row2  % -.8f  % -.8f  % -.8f  % -.8f\n", 
        __M44(a, 2, 0), __M44(a, 2, 1), __M44(a, 2, 2), __M44(a, 2, 3));
    printf("row3  % -.8f  % -.8f  % -.8f  % -.8f\n", 
        __M44(a, 3, 0), __M44(a, 3, 1), __M44(a, 3, 2), __M44(a, 3, 3));
    printf("\n");
}	

/* coordinate transformation matrix */

/* gl/rt scale */
static inline void mat44_scale(float *a, float x, float y, float z)
{
    mat44_identity(a);
    __M44(a, 0, 0)= x;
    __M44(a, 1, 1)= y;
    __M44(a, 2, 2)= z;
}

/* gl/rt translate */
static inline void mat44_translate(float *a, float x, float y, float z)
{
    mat44_identity(a);
    __M44(a, 0, 3)= x;
    __M44(a, 1, 3)= y;
    __M44(a, 2, 3)= z;
}

/* gl/rt rotate 
    cf. openGL 2 spec
 */
static inline void mat44_rotate(float *a, float angle, float x, float y, float z)
{
    float S[16];
    float C[16];
    float R[16];
    VEC3 u;
    float sinr, cosr;

    // R= u.u^T
    vec3_init(u, x, y, z);
    if(vec3_length2(u) < .00001f)
    {
        mat44_identity(a);
        return;
    }
    
    vec3_norm(u, u);
    mat44_vec3T(R, u);
    
    // S
    mat44_zero(S);
    sinr= sinf(angle / 180.0f * (float) M_PI);
    vec3_const_mul(u, sinr, u);
    __M44(S, 0, 1)= -u[2];
    __M44(S, 0, 2)=  u[1];
    __M44(S, 1, 0)=  u[2];
    __M44(S, 1, 2)= -u[0];
    __M44(S, 2, 0)= -u[1];
    __M44(S, 2, 1)=  u[0];
    
    // C
    mat44_identity33(C);
    mat44_sub(C, C, R);
    cosr= cosf(angle / 180.0f * (float) M_PI);
    mat44_const_mul(C, cosr, C);

    // a= R + C + S
    mat44_add(a, R, C);
    mat44_add(a, a, S);
    __M44(a, 3, 3)= 1.f;
}

/* adapted from mesa/GLU implementation
 */
static inline void mat44_lookat(float *a, 
    const float *eye, const float *center, const float *up)
{
    float forward[4];
    float side[4];
    float upn[4];
    
    vec3_sub(forward, center, eye);
    vec3_norm(forward, forward);
    
    vec3_cross(side, forward, up);
    vec3_norm(side, side);
    
    vec3_cross(upn, side, forward);
    
    mat44_identity(a);
    __M44(a, 0, 0)= side[0];
    __M44(a, 0, 1)= side[1];
    __M44(a, 0, 2)= side[2];
    
    __M44(a, 1, 0)= upn[0];
    __M44(a, 1, 1)= upn[1];
    __M44(a, 1, 2)= upn[2];
    
    __M44(a, 2, 0)= -forward[0];
    __M44(a, 2, 1)= -forward[1];
    __M44(a, 2, 2)= -forward[2];
    
    __M44(a, 0, 3)= -vec3_dot(eye, side);
    __M44(a, 1, 3)= -vec3_dot(eye, upn);
    __M44(a, 2, 3)= vec3_dot(eye, forward);
}

// static inline void mat44_frustum(float *a, 
//     float left, float right, 
//     float bottom, float top, 
//     float near, float far)
// {
//     mat44_zero(a);
//     __M44(a, 0, 0)= 2.f*near / (right - left);
//     __M44(a, 0, 2)= (right + left) / (right - left);
//     __M44(a, 1, 1)= 2.f*near / (top - bottom);
//     __M44(a, 1, 2)= (top + bottom) / (top - bottom);
//     __M44(a, 2, 2)= -(far + near) / (far - near);
//     __M44(a, 2, 3)= -2.f*far*near / (far - near);
//     __M44(a, 3, 2)= -1.f;
// }

/* adapted from 
 */
// static inline void mat44_perspective(float *a, 
//     float fovy, float aspect, 
//     float near, float far)
// {
//     float xmin, xmax, ymin, ymax;
// 
//     ymax = near * tanf(fovy / 2.f * (float) M_PI / 180.f);
//     ymin = -ymax;
//     xmin = ymin * aspect;
//     xmax = ymax * aspect;
// 
//     mat44_frustum(a, xmin, xmax, ymin, ymax, near, far);
// }

/* mxt44 = mx44^T */
static inline void mat44_transpose(float *mxt44, const float *mx44)
{
	int i, j;

	for( i=0; i<4; i++)
		for( j=0; j<4; j++)
			__M44( mxt44, i, j) = __M44( mx44, j, i);
}

/* mat3x3 -> mat4x4 */
static inline void mat33_to44(float *mx44, const float *mx33)
{
	mat44_identity(mx44);
	
	vec3_copy(&mx44[0], &mx33[0]);
	vec3_copy(&mx44[4], &mx33[3]);
	vec3_copy(&mx44[8], &mx33[6]);
}

/* mat4x4 -> mat3x3 */
static inline void mat44_to33(float *mx33, const float *mx44)
{
	vec3_copy(&mx33[0], &mx44[0]);
	vec3_copy(&mx33[3], &mx44[4]);
	vec3_copy(&mx33[6], &mx44[8]);
}

/* a = b */
static inline void mat33_copy(float *a, const float *b)
{
    vec3_copy(&a[0], &b[0]);
    vec3_copy(&a[3], &b[3]);
    vec3_copy(&a[6], &b[6]);
}

/* print mat3x3 */
static inline void mat33_print(const float *a)
{
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 0, 0), M33(a, 0, 1), M33(a, 0, 2));
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 1, 0), M33(a, 1, 1), M33(a, 1, 2));
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 2, 0), M33(a, 2, 1), M33(a, 2, 2));
    printf("\n");
}	

/* print transpose(mat3x3) */
static inline void mat33_t_print(const float *a)
{
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 0, 0), M33(a, 1, 0), M33(a, 2, 0));
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 0, 1), M33(a, 1, 1), M33(a, 2, 1));
    printf("% -.8f  % -.8f  % -.8f\n", 
        M33(a, 0, 2), M33(a, 1, 2), M33(a, 2, 2));
    printf("\n");
}	

#endif

