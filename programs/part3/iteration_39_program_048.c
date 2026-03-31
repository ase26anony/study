/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>

/* External helper functions from second compilation unit */
struct LargeStruct {
    int a, b, c, d;
    float e, f;
    double g, h;
};

extern struct LargeStruct __attribute__((noinline)) 
helper1(int a, int b, float c, double d);

extern struct LargeStruct __attribute__((noinline))
helper2(struct LargeStruct s1, struct LargeStruct s2);

extern int __attribute__((noinline))
helper3(int a, int b, int c, int d, int e, int f, int g, int h);

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Test function with extreme register pressure */
static int __attribute__((noinline, optimize("O3")))
test_function(int input) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with complex interdependent computations */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 << 2;
    a5 = a4 | 0xFF;
    a6 = a5 & 0x0F;
    a7 = a6 ^ a5;
    a8 = a7 + a6;
    a9 = a8 * a7;
    a10 = a9 / (a8 ? a8 : 1);
    
    /* Float computations with dependencies */
    f1 = (float)a1 * g_volatile_float;
    f2 = f1 + 1.0f;
    f3 = f2 * f1;
    f4 = f3 / f2;
    f5 = f4 - f3;
    f6 = f5 + f4;
    f7 = f6 * 2.0f;
    f8 = f7 / 3.0f;
    
    /* Double computations with more dependencies */
    d1 = (double)a2 * g_volatile_double;
    d2 = d1 + 1.0;
    d3 = d2 * d1;
    d4 = d3 / d2;
    d5 = d4 - d3;
    d6 = d5 + d4;
    
    /* Long integer computations */
    l1 = (long)a3 * (long)a4;
    l2 = l1 + (long)a5;
    l3 = l2 * l1;
    l4 = l3 / (l2 ? l2 : 1);
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v3 * v1;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    
    /* Artificial inline assembly to clobber physical registers */
    asm volatile(
        "# Force register pressure\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        : 
        : "r" (a1), "r" (a2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex interdependent computations */
    int b1 = a1 + a2;
    int b2 = b1 * a3;
    int b3 = b2 - a4;
    int b4 = b3 + a5;
    int b5 = b4 * a6;
    int b6 = b5 / (a7 ? a7 : 1);
    int b7 = b6 ^ a8;
    int b8 = b7 | a9;
    int b9 = b8 & a10;
    int b10 = b9 << 2;
    
    /* Mix types for additional pressure */
    float f9 = (float)b1 + f1;
    float f10 = f9 * f2;
    float f11 = f10 / f3;
    float f12 = f11 - f4;
    
    double d7 = (double)b2 + d1;
    double d8 = d7 * d2;
    double d9 = d8 / d3;
    double d10 = d9 - d4;
    
    /* Create a pseudo-register with multiple uses in adjacent statements */
    int critical_var = b3 + b4;
    int use1 = critical_var * b5;      /* First use */
    int use2 = critical_var + b6;      /* Second use - same reg reused */
    int use3 = use1 * use2;            /* Third use - creates dependency chain */
    
    /* Force the pseudo-register to be live across multiple operations */
    critical_var = use3 / (critical_var ? critical_var : 1);
    use1 = critical_var << 3;
    use2 = use1 | 0xAA;
    critical_var = use2 ^ use1;
    
    /* Call helper functions for inter-procedural pressure */
    struct LargeStruct ls1 = helper1(a1, a2, f1, d1);
    struct LargeStruct ls2 = helper1(a3, a4, f2, d2);
    struct LargeStruct ls3 = helper2(ls1, ls2);
    
    int helper_result = helper3(
        b1, b2, b3, b4, b5, b6, b7, b8
    );
    
    /* Final computation using all variables to ensure they're live */
    int result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
        (int)f5 + (int)f6 + (int)f7 + (int)f8 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + 
        (int)d5 + (int)d6 +
        (int)l1 + (int)l2 + (int)l3 + (int)l4 +
        v3[0] + v3[1] + v3[2] + v3[3] +
        (int)vf3[0] + (int)vf3[1] + (int)vf3[2] + (int)vf3[3] +
        b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
        (int)f9 + (int)f10 + (int)f11 + (int)f12 +
        (int)d7 + (int)d8 + (int)d9 + (int)d10 +
        critical_var + use1 + use2 + use3 +
        ls1.a + ls1.b + ls1.c + ls1.d +
        ls2.a + ls2.b + ls2.c + ls2.d +
        ls3.a + ls3.b + ls3.c + ls3.d +
        helper_result;
    
    return result;
}

int main() {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
