/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External helper functions from second compilation unit */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

extern struct MultiReg __attribute__((noinline)) helper1(int a, int b, float c, double d);
extern struct MultiReg __attribute__((noinline)) helper2(long a, long b, float c, double d);
extern struct MultiReg __attribute__((noinline)) helper3(int a, float b, double c, long d);
extern int __attribute__((noinline)) helper4(struct MultiReg m1, struct MultiReg m2);

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static int test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with seed */
    a1 = seed;
    f1 = seed * 1.5f;
    d1 = seed * 2.5;
    l1 = seed * 3L;
    
    /* Complex interdependent computations to create many pseudo-registers */
    /* First chain: integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 << 3;
    a5 = a4 ^ a2;
    a6 = a5 | a3;
    a7 = a6 & a4;
    a8 = a7 + a5 - a3;
    a9 = a8 * 2 + a6;
    a10 = a9 % 17 + a7;
    
    /* Float chain with dependencies */
    f2 = f1 * 2.0f + 1.0f;
    f3 = f2 / 1.5f - f1;
    f4 = f3 * f2 + f1;
    f5 = f4 - f3 * 0.5f;
    f6 = f5 + f2 / 3.0f;
    f7 = f6 * f4 - f3;
    f8 = f7 / 2.0f + f5;
    
    /* Double chain */
    d2 = d1 * 3.14159;
    d3 = d2 / 2.71828 + d1;
    d4 = d3 * d2 - d1;
    d5 = d4 + d3 / 1.41421;
    d6 = d5 * 2.0 - d4;
    
    /* Long chain */
    l2 = l1 * 5L + 11L;
    l3 = l2 / 3L - l1;
    l4 = l3 << 2;
    l5 = l4 ^ l2 | l3;
    
    /* Vector operations - use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v1 * v2 - v3;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 * vf2 + vf1;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 * vd2 - (v2df){d5, d6};
    
    /* Critical section: operations where pseudo-registers are used
       both as source and destination in adjacent statements */
    int temp1 = a10 + a9;
    int temp2 = temp1 * a8;  /* temp1 used as source, creates pseudo-register pressure */
    int temp3 = temp2 - temp1;  /* temp1 used again */
    int temp4 = temp3 / temp2;  /* temp2 used again */
    
    float ftemp1 = f8 + f7;
    float ftemp2 = ftemp1 * f6;  /* ftemp1 as source */
    float ftemp3 = ftemp2 - ftemp1;  /* ftemp1 used again */
    float ftemp4 = ftemp3 / ftemp2;  /* ftemp2 used again */
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile (
        "/* Clobber many registers to force pseudo-register usage */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        : 
        : "r" (temp1), "r" (temp2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after assembly to force register reloading */
    temp1 = temp4 * 3 + temp3;
    temp2 = temp1 / 2 - temp4;
    ftemp1 = ftemp4 * 1.5f + ftemp3;
    ftemp2 = ftemp1 / 2.0f - ftemp4;
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiReg m1 = helper1(a10, temp1, ftemp1, d6);
    struct MultiReg m2 = helper2(l5, l4, ftemp2, d5);
    struct MultiReg m3 = helper3(temp2, ftemp2, d4, l3);
    
    /* Use all computed values to ensure they're live */
    int result = a10 + temp1 + temp2 + temp3 + temp4;
    result += (int)f8 + (int)ftemp1 + (int)ftemp2 + (int)ftemp3 + (int)ftemp4;
    result += (int)d6 + (int)d5 + (int)d4;
    result += (int)l5 + (int)l4 + (int)l3;
    
    /* Use vector results */
    result += v3[0] + v3[1] + v3[2] + v3[3];
    result += (int)vf3[0] + (int)vf3[1] + (int)vf3[2] + (int)vf3[3];
    
    /* Cross-function result */
    result += helper4(m1, m2);
    result += m3.a + m3.b + m3.c + m3.d;
    
    return result;
}

int main() {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger optimization */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
