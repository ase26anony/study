/* Main test driver with hot loop */
#include <stdint.h>
#include <stdio.h>

/* External helper functions */
extern struct Vec4 helper1(int a, int b, int c, int d);
extern struct Vec4 helper2(struct Vec4 v1, struct Vec4 v2);
extern double helper3(struct Vec4 v, double factor);

/* Vector type for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for inter-procedural register pressure */
struct Vec4 {
    double x, y, z, w;
};

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Prevent inlining to maintain register pressure */
__attribute__((noinline, noipa))
double test_function(int seed) {
    /* Many local variables of different types */
    int a1 = seed * 2;
    int a2 = seed + 1;
    int a3 = seed - 5;
    int a4 = seed * 3;
    int a5 = seed / 2;
    int a6 = seed % 7;
    int a7 = seed ^ 0x55;
    int a8 = seed | 0xFF;
    int a9 = seed & 0xF0;
    int a10 = seed << 2;
    
    float f1 = seed * 1.5f;
    float f2 = seed * 2.5f;
    float f3 = seed * 0.5f;
    float f4 = seed * 3.14f;
    float f5 = seed * 2.71f;
    
    double d1 = seed * 1.234567;
    double d2 = seed * 2.345678;
    double d3 = seed * 3.456789;
    double d4 = seed * 4.567890;
    
    long l1 = seed * 1000L;
    long l2 = seed * 2000L;
    long l3 = seed * 3000L;
    long l4 = seed * 4000L;
    
    /* Vector types for wide register pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = {f2, f3, f4, f5};
    
    /* Complex interdependent computations */
    /* Chain 1: Integer operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 / (a5 + 1);
    int t5 = t4 | a6;
    int t6 = t5 & a7;
    int t7 = t6 ^ a8;
    int t8 = t7 << (a9 & 3);
    int t9 = t8 >> (a10 & 3);
    int t10 = t9 * t1;
    
    /* Chain 2: Floating point operations */
    float ft1 = f1 + f2;
    float ft2 = ft1 * f3;
    float ft3 = ft2 - f4;
    float ft4 = ft3 / f5;
    float ft5 = ft4 * ft1;
    float ft6 = ft5 + ft2;
    float ft7 = ft6 - ft3;
    float ft8 = ft7 / ft4;
    float ft9 = ft8 * ft5;
    float ft10 = ft9 + ft6;
    
    /* Chain 3: Double precision operations */
    double dt1 = d1 + d2;
    double dt2 = dt1 * d3;
    double dt3 = dt2 - d4;
    double dt4 = dt3 / d1;
    double dt5 = dt4 * dt1;
    double dt6 = dt5 + dt2;
    double dt7 = dt6 - dt3;
    double dt8 = dt7 / dt4;
    double dt9 = dt8 * dt5;
    double dt10 = dt9 + dt6;
    
    /* Chain 4: Long operations */
    long lt1 = l1 + l2;
    long lt2 = lt1 * l3;
    long lt3 = lt2 - l4;
    long lt4 = lt3 / (l1 + 1);
    long lt5 = lt4 * lt1;
    long lt6 = lt5 + lt2;
    long lt7 = lt6 - lt3;
    long lt8 = lt7 / lt4;
    long lt9 = lt8 * lt5;
    long lt10 = lt9 + lt6;
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = vt1 * v1;
    v4si vt3 = vt2 - v2;
    v4si vt4 = vt3 / (v1 + 1);
    
    v4sf vft1 = vf1 + vf2;
    v4sf vft2 = vft1 * vf1;
    v4sf vft3 = vft2 - vf2;
    v4sf vft4 = vft3 / (vf1 + 1.0f);
    
    /* Artificial register pressure with inline asm */
    /* Clobber many registers to force pseudo-register usage */
    asm volatile(
        "# Artificial clobber to increase register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r"(t10), "r"(ft10)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after asm to create live ranges */
    int final1 = t10 + (int)ft10;
    float final2 = ft10 + (float)t10;
    double final3 = dt10 + (double)final1;
    long final4 = lt10 + (long)final2;
    
    /* Use all variables in final computation */
    double result = (double)final1 + (double)final2 + final3 + (double)final4;
    
    /* Extract elements from vectors */
    int ve1 = vt1[0] + vt1[1] + vt1[2] + vt1[3];
    int ve2 = vt2[0] + vt2[1] + vt2[2] + vt2[3];
    int ve3 = vt3[0] + vt3[1] + vt3[2] + vt3[3];
    int ve4 = vt4[0] + vt4[1] + vt4[2] + vt4[3];
    
    float vfe1 = vft1[0] + vft1[1] + vft1[2] + vft1[3];
    float vfe2 = vft2[0] + vft2[1] + vft2[2] + vft2[3];
    float vfe3 = vft3[0] + vft3[1] + vft3[2] + vft3[3];
    float vfe4 = vft4[0] + vft4[1] + vft4[2] + vft4[3];
    
    /* Final complex result using all temporaries */
    result += (double)ve1 + (double)ve2 + (double)ve3 + (double)ve4;
    result += (double)vfe1 + (double)vfe2 + (double)vfe3 + (double)vfe4;
    result += (double)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    result += ft1 + ft2 + ft3 + ft4 + ft5 + ft6 + ft7 + ft8 + ft9;
    result += dt1 + dt2 + dt3 + dt4 + dt5 + dt6 + dt7 + dt8 + dt9;
    result += (double)lt1 + lt2 + lt3 + lt4 + lt5 + lt6 + lt7 + lt8 + lt9;
    
    return result;
}

int main() {
    double total = 0.0;
    int iterations = loop_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        /* Call helper functions to create inter-procedural pressure */
        struct Vec4 v1 = helper1(i, i+1, i+2, i+3);
        struct Vec4 v2 = helper1(i*2, i*3, i*4, i*5);
        struct Vec4 v3 = helper2(v1, v2);
        double partial = helper3(v3, 1.0 + i * 0.01);
        
        /* Main computation */
        total += test_function(i) + partial;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %f\n", total);
    return (int)total % 256;
}
