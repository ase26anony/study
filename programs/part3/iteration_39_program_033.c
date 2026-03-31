/* Test for early-remat.cc uncovered lines 930-937 */
/* Compile with: gcc -O2 -fearly-remat -fipa-ra -flto main.c helper.c -o test */

#include <stdio.h>
#include <stdint.h>

/* Forward declarations from helper.c */
extern int helper1(int a, int b, int c, int d, int e);
extern float helper2(float a, float b, float c);
extern double helper3(double a, double b, double c, double d);
extern long helper4(long a, long b, long c, long d, long e, long f);

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile int g_volatile_seed = 42;

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static int test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5, l6;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    
    /* Initialize with seed to create dependencies */
    a1 = seed;
    a2 = a1 * 3 + 1;
    a3 = a2 / 2 - a1;
    a4 = a3 << 3 | a2;
    a5 = a4 ^ a3;
    a6 = a5 + a4 - a3;
    a7 = a6 * 7 % 31;
    a8 = a7 & a6 | a5;
    a9 = a8 - a7 + a6;
    a10 = a9 * 11 / 5;
    
    /* Float operations creating many pseudo-registers */
    f1 = (float)a1 / 3.14f;
    f2 = f1 * 2.718f + (float)a2;
    f3 = f2 - f1 * 0.5f;
    f4 = f3 * f2 / f1;
    f5 = f4 + f3 - f2;
    f6 = f5 * 1.414f;
    
    /* Double operations */
    d1 = (double)a3 * 3.1415926535;
    d2 = d1 / 2.7182818284 + (double)f1;
    d3 = d2 * d1 - 1.6180339887;
    d4 = d3 + d2 / d1;
    d5 = d4 * 0.5772156649;
    
    /* Long operations */
    l1 = (long)a4 * 123456789L;
    l2 = l1 + (long)a5 * 987654321L;
    l3 = l2 - l1 >> 3;
    l4 = l3 * 7 + l2;
    l5 = l4 ^ l3 & l2;
    l6 = l5 - l4 + l3;
    
    /* Vector operations - use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v3 * v1 - v2;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, 1.0f, 2.0f};
    vf3 = vf1 * vf2 + (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many registers to force pseudo-register usage */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" (a1), "r" (a2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Complex interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    int t1 = a1 + a2;
    int t2 = t1 * a3 - a4;
    int t3 = t2 + a5 / t1;
    int t4 = t3 ^ t2 & t1;
    int t5 = t4 << 2 | t3;
    int t6 = t5 - t4 + t3;
    int t7 = t6 * 3 % 17;
    int t8 = t7 & t6 | t5;
    int t9 = t8 - t7 + t6;
    int t10 = t9 * 13 / 7;
    
    /* More operations creating register pressure */
    float ft1 = f1 + f2;
    float ft2 = ft1 * f3 - f4;
    float ft3 = ft2 + f5 / ft1;
    float ft4 = ft3 * 1.5f - ft2;
    
    double dt1 = d1 + d2;
    double dt2 = dt1 * d3 - d4;
    double dt3 = dt2 + d5 / dt1;
    
    /* Call helper functions to create inter-procedural pressure */
    int h1 = helper1(t1, t2, t3, t4, t5);
    float h2 = helper2(ft1, ft2, ft3);
    double h3 = helper3(dt1, dt2, dt3, d5);
    long h4 = helper4(l1, l2, l3, l4, l5, l6);
    
    /* Use all temporaries in final computation */
    int result = (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10) +
                 (int)(ft1 + ft2 + ft3 + ft4) +
                 (int)(dt1 + dt2 + dt3) +
                 (int)(h1 + h2 + h3 + h4) +
                 v3[0] + v3[1] + v3[2] + v3[3] +
                 (int)vf3[0];
    
    /* Another inline assembly to prevent optimization */
    asm volatile("# Result computed: %0" : : "r" (result));
    
    return result;
}

/* Main function with hot loop */
int main() {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        int seed = g_volatile_seed + i;
        total += test_function(seed);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("nop");
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
