/* Main test file to induce register pressure and trigger early rematerialization */
#include <stdint.h>
#include <stdlib.h>

/* Force compiler to use many pseudo-registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 100;
volatile int g_volatile_seed = 12345;

/* Complex struct to force register pressure across calls */
struct LargeStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long_data[4];
    double double_data[4];
};

/* Forward declarations for helper functions in separate file */
struct LargeStruct __attribute__((noinline)) helper_func1(int a, int b, int c, int d, 
                                                         float e, float f, double g, double h);
struct LargeStruct __attribute__((noinline)) helper_func2(struct LargeStruct s1, 
                                                         struct LargeStruct s2);
int __attribute__((noinline)) helper_func3(struct LargeStruct s);

/* Inline assembly to clobber physical registers and increase pressure */
#define CLOBBER_REGS() \
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                 "r8", "r9", "r10", "r11", "r12", "memory")

/* Main test function with dense computation */
__attribute__((noinline, optimize("O3")))
long test_function(int iterations) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with volatile to prevent constant propagation */
    a1 = g_volatile_seed;
    f1 = (float)g_volatile_seed * 0.5f;
    d1 = (double)g_volatile_seed * 0.25;
    l1 = (long)g_volatile_seed * 2L;
    
    /* Create complex dependency chain forcing pseudo-register usage */
    /* First chain: integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = (a3 << 3) | (a2 & 0xFF);
    a5 = a4 ^ a3 ^ a2;
    a6 = a5 * 11 % 31;
    a7 = a6 + a5 - a4 + a3;
    a8 = a7 * a6 / (a5 + 1);
    a9 = a8 | a7 | a6;
    a10 = a9 & a8 & a7;
    
    /* Float operations with dependencies */
    f2 = f1 * 1.5f + 2.3f;
    f3 = f2 / 1.7f - f1;
    f4 = f3 * f2 + f1;
    f5 = f4 - f3 * 0.5f;
    f6 = f5 * 2.0f / f4;
    f7 = f6 + f5 - f4 + f3;
    f8 = f7 * 1.1f;
    
    /* Double operations */
    d2 = d1 * 1.25 + 3.14;
    d3 = d2 / 2.0 - d1;
    d4 = d3 * d2 + d1;
    d5 = d4 - d3 * 0.75;
    d6 = d5 * 1.5 / d4;
    
    /* Long operations */
    l2 = l1 * 3L + 1000L;
    l3 = l2 / 2L - l1;
    l4 = l3 * 5L + l2;
    l5 = l4 ^ l3 ^ l2;
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v3 * v1 - v2;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 * vf2 + vf1;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 * vd2 - (v2df){d5, d6};
    
    /* Critical section: operations where variables are used as both
       source and destination in adjacent statements - this creates
       the pattern that might trigger the replacement logic */
    int temp1 = a10;
    a10 = temp1 * a9 + a8;  /* a10 used as source and destination */
    temp1 = a10;            /* Force another use */
    
    float temp2 = f8;
    f8 = temp2 * f7 - f6;   /* f8 used as source and destination */
    temp2 = f8;             /* Force another use */
    
    double temp3 = d6;
    d6 = temp3 / d5 * d4;   /* d6 used as source and destination */
    temp3 = d6;             /* Force another use */
    
    /* Clobber registers to increase pressure */
    CLOBBER_REGS();
    
    /* More operations to ensure all variables are live */
    l1 = l5 + (long)a10 + (long)temp1;
    l2 = l4 + (long)(f8 * 100.0f) + (long)(temp2 * 50.0f);
    l3 = l3 + (long)(d6 * 1000.0) + (long)(temp3 * 500.0);
    
    /* Mix scalar and vector operations in same basic block */
    v4 = v4 + (v4si){a10, temp1, a9, a8};
    vf3 = vf3 * (v4sf){temp2, f8, f7, f6};
    
    /* Call helper functions to create inter-procedural pressure */
    struct LargeStruct s1 = helper_func1(a1, a2, a3, a4, f1, f2, d1, d2);
    struct LargeStruct s2 = helper_func1(a5, a6, a7, a8, f3, f4, d3, d4);
    
    /* Another clobber to force spill/reload */
    CLOBBER_REGS();
    
    struct LargeStruct s3 = helper_func2(s1, s2);
    
    /* Final computation using all temporaries */
    long result = l1 + l2 + l3 + l4 + l5;
    result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    result += (long)(d1 + d2 + d3 + d4 + d5 + d6);
    result += v3[0] + v3[1] + v3[2] + v3[3];
    result += (long)vf3[0] + (long)vf3[1] + (long)vf3[2] + (long)vf3[3];
    result += (long)vd1[0] + (long)vd1[1];
    
    result += helper_func3(s3);
    
    return result;
}

/* Main function with loop to ensure code is executed */
int main() {
    long total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to ensure the function is compiled and executed multiple times */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Modify volatile to prevent loop unrolling */
        g_volatile_seed = (g_volatile_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        return 1;
    }
    
    return 0;
}
