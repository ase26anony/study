/* Test case to trigger early rematerialization pseudo-register replacement */
/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -c early_remat_test.c */
/* Then link with: gcc -O2 -fearly-remat early_remat_test.c early_remat_helpers.c -o test */

#include <stdint.h>
#include <stdio.h>

/* Force compiler to use many pseudo-registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Complex struct to force register pressure across calls */
struct MultiRegStruct {
    long a;
    double b;
    v4si c;
    float d;
    int e;
};

/* Forward declarations for helper functions in another file */
struct MultiRegStruct __attribute__((noinline)) helper_func1(int a, double b, long c, float d);
struct MultiRegStruct __attribute__((noinline)) helper_func2(v4si a, v4sf b, int c, double d);
double __attribute__((noinline)) helper_func3(struct MultiRegStruct s1, struct MultiRegStruct s2);

/* Main test function with high register pressure */
static double __attribute__((noinline, optimize("O3"))) 
test_function(int seed) {
    /* Declare many variables of different types to create register pressure */
    int t1 = seed + 1;
    double t2 = seed * 2.5;
    long t3 = seed * 3L;
    float t4 = seed * 4.0f;
    v4si t5 = {seed, seed + 1, seed + 2, seed + 3};
    v4sf t6 = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    
    /* Complex interdependent computations */
    /* Chain 1: Integer operations */
    int t7 = t1 * 2;
    int t8 = t7 + t1;
    int t9 = t8 - t7;
    int t10 = t9 * t8;
    int t11 = t10 / (t1 + 1);
    int t12 = t11 ^ t7;
    int t13 = t12 | t8;
    int t14 = t13 & t9;
    int t15 = t14 << 2;
    int t16 = t15 >> 1;
    
    /* Chain 2: Floating point operations */
    double t17 = t2 * 1.5;
    double t18 = t17 + t2;
    double t19 = t18 - t17;
    double t20 = t19 * t18;
    double t21 = t20 / (t2 + 1.0);
    float t22 = t4 * 2.0f;
    float t23 = t22 + t4;
    float t24 = t23 - t22;
    float t25 = t24 * t23;
    float t26 = t25 / (t4 + 1.0f);
    
    /* Chain 3: Vector operations */
    v4si t27 = t5 + (v4si){1, 2, 3, 4};
    v4si t28 = t27 * t5;
    v4si t29 = t28 - t27;
    v4si t30 = t29 & t28;
    v4sf t31 = t6 * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
    v4sf t32 = t31 + t6;
    v4sf t33 = t32 - t31;
    
    /* Chain 4: Mixed type operations with conversions */
    double t34 = t1 + t2;
    float t35 = t3 + t4;
    long t36 = t1 * t3;
    double t37 = t36 + t2;
    float t38 = t37 + t4;
    
    /* Create artificial register pressure with inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" (t1), "r" (t2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiRegStruct s1 = helper_func1(t1, t2, t3, t4);
    struct MultiRegStruct s2 = helper_func2(t27, t31, t16, t20);
    
    /* More computations using results from helpers */
    double t39 = s1.b + s2.b;
    float t40 = s1.d * s2.d;
    v4si t41 = s1.c + s2.c;
    
    /* Complex expression with many intermediate values */
    /* This should create many pseudo-registers */
    double result = 0.0;
    result += t1 * t2;
    result += t3 * t4;
    result += t7 * t8;
    result += t9 * t10;
    result += t11 * t12;
    result += t13 * t14;
    result += t15 * t16;
    result += t17 * t18;
    result += t19 * t20;
    result += t21 * t22;
    result += t23 * t24;
    result += t25 * t26;
    result += t34 * t35;
    result += t36 * t37;
    result += t38 * t39;
    result += t40 * helper_func3(s1, s2);
    
    /* Use all vector variables to keep them live */
    for (int i = 0; i < 4; i++) {
        result += t5[i] + t6[i];
        result += t27[i] + t28[i];
        result += t29[i] + t30[i];
        result += t31[i] + t32[i];
        result += t33[i] + t41[i];
    }
    
    return result;
}

int main(void) {
    double total = 0.0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger optimization */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to prevent dead code elimination */
        volatile int seed = i;
        total += test_function(seed);
        
        /* Prevent loop unrolling */
        if (i % 100 == 0) {
            asm volatile ("# Loop barrier" ::: "memory");
        }
    }
    
    printf("Result: %f\n", total);
    return 0;
}
