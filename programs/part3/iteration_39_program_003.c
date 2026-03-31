/* Main test driver with hot loop */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External helper functions from second compilation unit */
extern struct Vec4 add_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 mul_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 complex_operation(struct Vec4 a, struct Vec4 b, struct Vec4 c);
extern double compute_pressure(int a, int b, int c, int d, int e, int f);

/* Vector type for register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Struct for cross-function register pressure */
struct Vec4 {
    float x, y, z, w;
};

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* Noinline to prevent inlining and increase register pressure across calls */
__attribute__((noinline, noipa))
float test_function(int seed) {
    /* Many local variables of different types to create register pressure */
    int a1 = seed + 1;
    int a2 = seed * 2;
    int a3 = seed / 3;
    int a4 = seed - 4;
    int a5 = seed % 5;
    int a6 = seed | 0xFF;
    int a7 = seed & 0x0F;
    int a8 = seed ^ 0x55;
    int a9 = seed << 2;
    int a10 = seed >> 1;
    
    float f1 = a1 * 0.1f;
    float f2 = a2 * 0.2f;
    float f3 = a3 * 0.3f;
    float f4 = a4 * 0.4f;
    float f5 = a5 * 0.5f;
    
    double d1 = f1 * 1.1;
    double d2 = f2 * 1.2;
    double d3 = f3 * 1.3;
    double d4 = f4 * 1.4;
    double d5 = f5 * 1.5;
    
    long l1 = a1 * 1000L;
    long l2 = a2 * 2000L;
    long l3 = a3 * 3000L;
    long l4 = a4 * 4000L;
    
    /* Vector operations for wide register pressure */
    v4sf v1 = {f1, f2, f3, f4};
    v4sf v2 = {f2, f3, f4, f5};
    v4sf v3 = {f3, f4, f5, f1};
    
    v4si vi1 = {a1, a2, a3, a4};
    v4si vi2 = {a5, a6, a7, a8};
    v4si vi3 = {a9, a10, a1, a2};
    
    /* Complex interdependent computations */
    /* Chain 1: Creates many pseudo-registers with multiple uses */
    int t1 = a1 + a2;
    int t2 = t1 * a3;          /* t1 used here */
    int t3 = t2 - a4;          /* t2 used here */
    int t4 = t3 / a5;          /* t3 used here */
    int t5 = t4 | a6;          /* t4 used here */
    int t6 = t5 & a7;          /* t5 used here */
    int t7 = t6 ^ a8;          /* t6 used here */
    int t8 = t7 << 2;          /* t7 used here */
    int t9 = t8 >> 1;          /* t8 used here */
    int t10 = t9 % 10;         /* t9 used here */
    
    /* Chain 2: Floating point dependencies */
    float ft1 = f1 + f2;
    float ft2 = ft1 * f3;      /* ft1 used here */
    float ft3 = ft2 - f4;      /* ft2 used here */
    float ft4 = ft3 / f5;      /* ft3 used here */
    float ft5 = ft4 * 2.0f;    /* ft4 used here */
    
    /* Chain 3: Mixed type computations */
    double dt1 = d1 + ft1;
    double dt2 = dt1 * d2;     /* dt1 used here */
    double dt3 = dt2 - ft2;    /* dt2 used here */
    double dt4 = dt3 / d3;     /* dt3 used here */
    double dt5 = dt4 * 1.5;    /* dt4 used here */
    
    /* Vector operations creating more pressure */
    v4sf vt1 = v1 + v2;
    v4sf vt2 = vt1 * v3;       /* vt1 used here */
    v4sf vt3 = vt2 - v1;       /* vt2 used here */
    
    v4si vit1 = vi1 + vi2;
    v4si vit2 = vit1 * vi3;    /* vit1 used here */
    v4si vit3 = vit2 & vi1;    /* vit2 used here */
    
    /* Inline assembly to clobber physical registers and increase pressure */
    /* Clobber multiple registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r0, r1, r0\n"
        : 
        : "r" (t1), "r" (t2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory", "cc"
    );
    
    /* More computations after assembly to force reloads */
    int t11 = t10 + a9;
    int t12 = t11 * a10;
    int t13 = t12 - t1;
    int t14 = t13 / t2;
    int t15 = t14 | t3;
    
    float ft6 = ft5 + ft1;
    float ft7 = ft6 * ft2;
    float ft8 = ft7 - ft3;
    float ft9 = ft8 / ft4;
    float ft10 = ft9 * ft5;
    
    /* Struct operations for cross-function pressure */
    struct Vec4 vec1 = {ft1, ft2, ft3, ft4};
    struct Vec4 vec2 = {ft5, ft6, ft7, ft8};
    struct Vec4 vec3 = {ft9, ft10, f1, f2};
    
    /* Call helper functions to increase inter-procedural pressure */
    struct Vec4 vec_result = complex_operation(vec1, vec2, vec3);
    double extra_pressure = compute_pressure(t1, t2, t3, t4, t5, t6);
    
    /* Final computation using all temporaries to keep them live */
    float result = 
        (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + 
         t11 + t12 + t13 + t14 + t15) * 0.01f +
        (ft1 + ft2 + ft3 + ft4 + ft5 + ft6 + ft7 + ft8 + ft9 + ft10) +
        (float)(d1 + d2 + d3 + d4 + d5 + dt1 + dt2 + dt3 + dt4 + dt5) +
        vec_result.x + vec_result.y + vec_result.z + vec_result.w +
        (float)extra_pressure +
        vt1[0] + vt2[1] + vt3[2] +
        (float)(vit1[0] + vit2[1] + vit3[2]) +
        (float)(l1 + l2 + l3 + l4) * 0.0001f;
    
    return result;
}

int main() {
    float total = 0.0f;
    int iterations = g_iterations;
    
    printf("Starting early rematerialization test...\n");
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent loop unrolling */
        volatile int seed = i * 1234567;
        float result = test_function(seed);
        total += result;
        
        /* Prevent dead code elimination */
        asm volatile ("" : "+g" (total));
    }
    
    printf("Test completed. Result: %f\n", total);
    return (int)total % 256;
}
