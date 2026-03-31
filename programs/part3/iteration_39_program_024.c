/* Test case for early-remat.cc uncovered lines 930-937 */
/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -c main.c helper.c -flto */

#include <stdint.h>
#include <stdio.h>

/* Forward declarations for helper functions */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

struct MultiReg __attribute__((noinline)) helper1(int a, int b, float c, double d);
struct MultiReg __attribute__((noinline)) helper2(long a, double b, int c, float d);
struct MultiReg __attribute__((noinline)) helper3(struct MultiReg a, struct MultiReg b);
float __attribute__((noinline)) helper4(double a, double b, double c, double d);
double __attribute__((noinline)) helper5(float a, float b, float c, float d, float e);

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_function(int seed) {
    /* Declare many local variables of different types */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v1 + v2;
    int v4 = v2 - v3;
    int v5 = v3 * v4;
    int v6 = v4 / (v1 ? v1 : 1);
    int v7 = v5 ^ v6;
    int v8 = v6 | v7;
    int v9 = v7 & v8;
    int v10 = v8 << 2;
    
    float f1 = (float)v1 * g_volatile_float;
    float f2 = f1 * 2.0f;
    float f3 = f1 + f2;
    float f4 = f2 - f3;
    float f5 = f3 * f4;
    float f6 = f4 / (f1 ? f1 : 1.0f);
    
    double d1 = (double)v2 * g_volatile_double;
    double d2 = d1 * 2.0;
    double d3 = d1 + d2;
    double d4 = d2 - d3;
    double d5 = d3 * d4;
    double d6 = d4 / (d1 ? d1 : 1.0);
    
    long l1 = (long)v3 * 1000L;
    long l2 = l1 * 3L;
    long l3 = l1 + l2;
    long l4 = l2 - l3;
    long l5 = l3 * l4;
    
    /* Vector operations for wide register pressure */
    v4si vec1 = {v1, v2, v3, v4};
    v4si vec2 = {v5, v6, v7, v8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec3 - vec4;
    
    v4sf vecf1 = {f1, f2, f3, f4};
    v4sf vecf2 = {f5, f6, f1, f2};
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf1 * vecf2;
    
    v2df vecd1 = {d1, d2};
    v2df vecd2 = {d3, d4};
    v2df vecd3 = vecd1 + vecd2;
    v2df vecd4 = vecd1 * vecd2;
    
    /* Complex interdependent computations */
    int t1 = v9 + v10;
    int t2 = t1 * v1;
    int t3 = t2 - v2;
    int t4 = t3 / (v3 ? v3 : 1);
    int t5 = t4 ^ v4;
    int t6 = t5 | v5;
    int t7 = t6 & v6;
    int t8 = t7 << 3;
    int t9 = t8 >> 1;
    int t10 = t9 + v7;
    
    float ft1 = f5 + f6;
    float ft2 = ft1 * f1;
    float ft3 = ft2 - f2;
    float ft4 = ft3 / (f3 ? f3 : 1.0f);
    float ft5 = ft4 * f4;
    float ft6 = ft5 + f5;
    
    double dt1 = d5 + d6;
    double dt2 = dt1 * d1;
    double dt3 = dt2 - d2;
    double dt4 = dt3 / (d3 ? d3 : 1.0);
    double dt5 = dt4 * d4;
    double dt6 = dt5 + d5;
    
    /* Inline assembly to clobber physical registers and increase pressure */
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
    
    /* More computations after assembly to force register reloading */
    int u1 = t10 + v8;
    int u2 = u1 * v9;
    int u3 = u2 - v10;
    int u4 = u3 / (t1 ? t1 : 1);
    int u5 = u4 ^ t2;
    int u6 = u5 | t3;
    int u7 = u6 & t4;
    int u8 = u7 << 2;
    int u9 = u8 >> 1;
    int u10 = u9 + t5;
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiReg mr1 = helper1(u1, u2, ft1, dt1);
    struct MultiReg mr2 = helper2(l1, dt2, u3, ft2);
    struct MultiReg mr3 = helper3(mr1, mr2);
    
    float fresult = helper4(dt3, dt4, dt5, dt6);
    double dresult = helper5(ft3, ft4, ft5, ft6, fresult);
    
    /* Final complex computation using all temporaries */
    int final1 = u10 + mr3.a + mr3.b;
    int final2 = final1 * (int)(fresult * 100.0f);
    int final3 = final2 + (int)(dresult * 1000.0);
    int final4 = final3 ^ vec5[0];
    int final5 = final4 | vec5[1];
    int final6 = final5 & vec5[2];
    int final7 = final6 + vec5[3];
    
    /* Use vector results */
    v4si final_vec = vec3 + vec4 + vec5;
    final7 += final_vec[0] + final_vec[1] + final_vec[2] + final_vec[3];
    
    v4sf final_vecf = vecf3 + vecf4;
    final7 += (int)(final_vecf[0] + final_vecf[1] + final_vecf[2] + final_vecf[3]);
    
    v2df final_vecd = vecd3 + vecd4;
    final7 += (int)(final_vecd[0] + final_vecd[1]);
    
    return final7;
}

int main(void) {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger optimization */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        total ^= test_function(i + 1);
        total -= test_function(i + 2);
        total |= test_function(i + 3);
    }
    
    printf("Result: %d\n", total);
    return total != 0;
}
