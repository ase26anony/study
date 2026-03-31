/* Main test driver with high register pressure loop */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External helper functions from second compilation unit */
extern struct Vec4 add_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 mul_vec4(struct Vec4 a, struct Vec4 b);
extern struct Vec4 cross_vec4(struct Vec4 a, struct Vec4 b);
extern double dot_product(struct Vec4 a, struct Vec4 b);
extern struct Complex complex_operation(struct Complex a, struct Complex b);

/* Vector types for register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Struct types to force register/memory pressure */
struct Vec4 {
    float x, y, z, w;
};

struct Complex {
    double real, imag;
};

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Force pseudo-register creation with complex expression chain */
__attribute__((noinline))
static double test_function(int seed) {
    /* Many local variables of different types */
    int a1 = seed + 1;
    int a2 = seed * 2;
    int a3 = seed / 3;
    int a4 = seed - 4;
    int a5 = seed % 5;
    
    float f1 = g_volatile_float * 1.1f;
    float f2 = g_volatile_float * 2.2f;
    float f3 = g_volatile_float * 3.3f;
    float f4 = g_volatile_float * 4.4f;
    float f5 = g_volatile_float * 5.5f;
    
    double d1 = g_volatile_double * 1.11;
    double d2 = g_volatile_double * 2.22;
    double d3 = g_volatile_double * 3.33;
    double d4 = g_volatile_double * 4.44;
    double d5 = g_volatile_double * 5.55;
    
    long l1 = (long)seed * 1000L;
    long l2 = (long)seed * 2000L;
    long l3 = (long)seed * 3000L;
    long l4 = (long)seed * 4000L;
    long l5 = (long)seed * 5000L;
    
    /* Vector operations for wide register pressure */
    v4sf v1 = {f1, f2, f3, f4};
    v4sf v2 = {f2, f3, f4, f5};
    v4sf v3 = {f3, f4, f5, f1};
    v4sf v4 = {f4, f5, f1, f2};
    
    v4si vi1 = {a1, a2, a3, a4};
    v4si vi2 = {a2, a3, a4, a5};
    v4si vi3 = {a3, a4, a5, a1};
    v4si vi4 = {a4, a5, a1, a2};
    
    /* Complex interdependent computations */
    /* Chain 1: Integer operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;        /* t1 used as operand and destination */
    int t3 = t2 - a4;        /* t2 used as operand and destination */
    int t4 = t3 / a5;        /* t3 used as operand and destination */
    int t5 = t4 ^ t1;        /* Multiple uses of previous temps */
    
    /* Chain 2: Float operations with dependencies */
    float ft1 = f1 * f2;
    float ft2 = ft1 + f3;    /* ft1 used as operand and destination */
    float ft3 = ft2 - f4;    /* ft2 used as operand and destination */
    float ft4 = ft3 * f5;    /* ft3 used as operand and destination */
    float ft5 = ft4 / ft1;   /* Multiple uses of previous temps */
    
    /* Chain 3: Double operations */
    double dt1 = d1 + d2;
    double dt2 = dt1 * d3;   /* dt1 used as operand and destination */
    double dt3 = dt2 - d4;   /* dt2 used as operand and destination */
    double dt4 = dt3 / d5;   /* dt3 used as operand and destination */
    double dt5 = dt4 + dt1;  /* Multiple uses of previous temps */
    
    /* Chain 4: Long operations */
    long lt1 = l1 + l2;
    long lt2 = lt1 * l3;     /* lt1 used as operand and destination */
    long lt3 = lt2 - l4;     /* lt2 used as operand and destination */
    long lt4 = lt3 / l5;     /* lt3 used as operand and destination */
    long lt5 = lt4 ^ lt1;    /* Multiple uses of previous temps */
    
    /* Vector operations creating more pressure */
    v4sf vt1 = v1 + v2;
    v4sf vt2 = vt1 * v3;     /* vt1 used as operand and destination */
    v4sf vt3 = vt2 - v4;     /* vt2 used as operand and destination */
    v4sf vt4 = vt3 / v1;     /* Multiple uses of previous temps */
    
    v4si vit1 = vi1 + vi2;
    v4si vit2 = vit1 * vi3;  /* vit1 used as operand and destination */
    v4si vit3 = vit2 - vi4;  /* vit2 used as operand and destination */
    v4si vit4 = vit3 & vi1;  /* Multiple uses of previous temps */
    
    /* Inline assembly to clobber physical registers */
    /* Clobber multiple registers to force pseudo-register usage */
    asm volatile (
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        : 
        : "r" (t1), "r" (t2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex chains mixing types */
    double mix1 = (double)t1 + ft1;
    double mix2 = mix1 * dt1;
    double mix3 = mix2 + (double)lt1;
    double mix4 = mix3 / dt2;
    double mix5 = mix4 * mix1;
    
    /* Struct operations */
    struct Vec4 vec1 = {ft1, ft2, ft3, ft4};
    struct Vec4 vec2 = {ft2, ft3, ft4, ft5};
    struct Vec4 vec3 = add_vec4(vec1, vec2);
    struct Vec4 vec4 = mul_vec4(vec3, vec1);
    
    struct Complex c1 = {dt1, dt2};
    struct Complex c2 = {dt3, dt4};
    struct Complex c3 = complex_operation(c1, c2);
    
    /* Final computation using all temporaries */
    double result = (double)t1 + (double)t2 + (double)t3 + (double)t4 + (double)t5 +
                   (double)ft1 + (double)ft2 + (double)ft3 + (double)ft4 + (double)ft5 +
                   dt1 + dt2 + dt3 + dt4 + dt5 +
                   (double)lt1 + (double)lt2 + (double)lt3 + (double)lt4 + (double)lt5 +
                   mix1 + mix2 + mix3 + mix4 + mix5 +
                   (double)vec3.x + (double)vec4.y +
                   c3.real + c3.imag;
    
    /* Extract elements from vectors */
    float vsum = vt1[0] + vt1[1] + vt1[2] + vt1[3] +
                 vt2[0] + vt2[1] + vt2[2] + vt2[3] +
                 vt3[0] + vt3[1] + vt3[2] + vt3[3] +
                 vt4[0] + vt4[1] + vt4[2] + vt4[3];
    
    int visum = vit1[0] + vit1[1] + vit1[2] + vit1[3] +
                vit2[0] + vit2[1] + vit2[2] + vit2[3] +
                vit3[0] + vit3[1] + vit3[2] + vit3[3] +
                vit4[0] + vit4[1] + vit4[2] + vit4[3];
    
    result += (double)vsum + (double)visum;
    
    return result;
}

int main() {
    double total = 0.0;
    int iterations = g_volatile_counter;
    
    printf("Starting early rematerialization test with %d iterations\n", iterations);
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent complete optimization */
        int seed = i + (int)g_volatile_float;
        double result = test_function(seed);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Occasionally call helper to create inter-procedural pressure */
        if (i % 100 == 0) {
            struct Vec4 v1 = {1.0f, 2.0f, 3.0f, 4.0f};
            struct Vec4 v2 = {5.0f, 6.0f, 7.0f, 8.0f};
            struct Vec4 v3 = cross_vec4(v1, v2);
            total += dot_product(v1, v3);
        }
    }
    
    printf("Test completed. Total: %f\n", total);
    return (total > 0.0) ? 0 : 1;
}
