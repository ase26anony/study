/* Main test driver with hot loop */
#include <stdint.h>
#include <stdio.h>

/* External helper functions */
extern struct DataPair helper1(int a, int b, int c, int d);
extern struct DataPair helper2(float a, float b, double c, long d);
extern struct DataPair helper3(v4si a, v4si b, v4si c);

/* Force compiler to keep computations */
volatile int loop_counter = 1000;

/* Vector type for register pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Struct for cross-function register pressure */
struct DataPair {
    long long a;
    double b;
    v4si c;
    int d;
};

/* Prevent inlining to increase register pressure across calls */
__attribute__((noinline, optimize("O0")))
struct DataPair test_function(int seed) {
    /* Many local variables of different types */
    volatile int v1 = seed;
    volatile float v2 = seed * 1.5f;
    volatile double v3 = seed * 2.5;
    volatile long v4 = seed * 3L;
    volatile v4si v5 = {seed, seed+1, seed+2, seed+3};
    volatile v4si v6 = {seed+4, seed+5, seed+6, seed+7};
    
    /* Chain of interdependent computations */
    int t1 = v1 + 1;
    float t2 = v2 * 2.0f + t1;
    double t3 = v3 / 1.5 + t2;
    long t4 = v4 << 2 | t1;
    v4si t5 = v5 + v6;
    v4si t6 = t5 * (v4si){2, 2, 2, 2};
    
    /* More temporaries with complex dependencies */
    int t7 = t1 * t1 - t4 % 256;
    float t8 = t2 * t2 / (t7 + 1);
    double t9 = t3 * t3 - t8;
    long t10 = t4 * t4 + t7;
    v4si t11 = t6 + t5;
    v4si t12 = t11 * (v4si){3, 3, 3, 3};
    
    /* Additional chain to increase register pressure */
    int t13 = t7 ^ t1;
    float t14 = t8 + t2 - t13;
    double t15 = t9 * t3 / (t14 + 1.0);
    long t16 = t10 | t4;
    v4si t17 = t12 - t11;
    v4si t18 = t17 >> 1;
    
    /* More operations creating pseudo-register pressure */
    int t19 = t13 & t7;
    float t20 = t14 * 0.5f;
    double t21 = t15 + 1.2345;
    long t22 = t16 ^ 0xAAAA;
    v4si t23 = t18 + t17;
    v4si t24 = t23 * (v4si){4, 4, 4, 4};
    
    /* Critical section: pseudo-register used as both operand and destination */
    int t25 = t19 + t13;  /* t19 used as operand */
    int t26 = t25 * t19;  /* t25 used, t19 used again */
    float t27 = t20 + t14;
    float t28 = t27 * t20;
    double t29 = t21 - t15;
    double t30 = t29 / t21;
    
    /* Artificial register pressure via inline asm */
    asm volatile(
        "/* Clobber physical registers to force pseudo-reg usage */\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after asm clobber */
    long t31 = t22 + t16;
    long t32 = t31 ^ t22;
    v4si t33 = t24 + t23;
    v4si t34 = t33 >> 2;
    
    /* Final computation using all temporaries */
    struct DataPair result;
    result.a = t25 + t26 + t31 + t32;
    result.b = t27 + t28 + t29 + t30;
    result.c = t33 + t34 + t5 + t6 + t11 + t12 + t17 + t18 + t23 + t24;
    result.d = t1 + t7 + t13 + t19;
    
    return result;
}

int main() {
    struct DataPair total = {0, 0.0, {0, 0, 0, 0}, 0};
    int iterations = loop_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        struct DataPair result = test_function(i);
        
        /* Use results to prevent elimination */
        total.a += result.a;
        total.b += result.b;
        total.c += result.c;
        total.d += result.d;
        
        /* Additional computation to increase pressure */
        if (i % 2 == 0) {
            struct DataPair h1 = helper1(i, i+1, i+2, i+3);
            total.a += h1.a;
            total.b += h1.b;
        }
    }
    
    /* Print to prevent dead code elimination */
    printf("Result: %lld %f %d %d %d %d %d\n", 
           total.a, total.b, 
           total.c[0], total.c[1], total.c[2], total.c[3],
           total.d);
    
    return 0;
}
