/* Main test driver for early rematerialization */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
volatile int g_volatile_counter = 100;

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiRegStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long extra[2];
};

/* Forward declarations for helper functions in separate compilation unit */
struct MultiRegStruct __attribute__((noinline)) helper_func1(struct MultiRegStruct a, struct MultiRegStruct b);
struct MultiRegStruct __attribute__((noinline)) helper_func2(struct MultiRegStruct a, int b, float c, double d);
v4si __attribute__((noinline)) vector_helper(v4si a, v4si b, v4si c);

/* Main test function with high register pressure */
static long long __attribute__((noinline)) 
test_function(int seed) 
{
    /* Declare many variables of different types to force pseudo-registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with seed to prevent constant propagation */
    a1 = seed;
    f1 = seed * 1.5f;
    d1 = seed * 2.5;
    l1 = seed * 3LL;
    
    /* Create long chain of interdependent computations */
    /* This forces serial evaluation and many temporary pseudo-registers */
    
    /* Integer computation chain */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 * a2 + a1;
    a5 = (a4 << 3) | (a3 & 0xFF);
    a6 = a5 ^ a4 ^ a3 ^ a2;
    a7 = a6 * 0x1234567 + a5;
    a8 = a7 - a6 + a5 - a4;
    a9 = a8 * 3 / 2 + a7;
    a10 = a9 % 1000 + a8;
    
    /* Float computation chain with type mixing */
    f2 = f1 * 3.14f + a1;
    f3 = f2 / 2.0f - f1;
    f4 = f3 * a2 + f2 * a3;
    f5 = f4 - f3 + f2 - f1;
    f6 = f5 * 1.414f + a4;
    f7 = f6 / 3.14159f * f5;
    f8 = f7 + f6 * 0.5f + a5;
    f9 = f8 - f7 * 2.0f + f6;
    f10 = f9 * f8 / f7 + a6;
    
    /* Double computation chain */
    d2 = d1 * 3.1415926535 + f1;
    d3 = d2 / 2.7182818284 - d1;
    d4 = d3 * a7 + d2 * f2;
    d5 = d4 - d3 + d2 - d1;
    d6 = d5 * 1.7320508075 + a8;
    d7 = d6 / 2.2360679774 * d5;
    d8 = d7 + d6 * 0.3333333333 + a9;
    d9 = d8 - d7 * 1.5 + d6;
    d10 = d9 * d8 / d7 + a10;
    
    /* Long long computation chain */
    l2 = l1 * 123456789LL + a1;
    l3 = l2 / 1000LL - l1;
    l4 = l3 * a2 + l2 * a3;
    l5 = l4 ^ l3 ^ l2 ^ l1;
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v1 * v2 - v3;
    v5 = v4 << 2 | v3 >> 1;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 * vf2 - vf1 / vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 * vd2 - vd1 / vd2;
    
    /* Critical section: Create pseudo-register with multiple uses */
    /* This is designed to trigger the replacement logic */
    int critical_var = a10 * 2 + 3;
    
    /* Use critical_var in multiple dependent operations */
    /* This creates a DF_REF with multiple uses */
    int use1 = critical_var * 5;
    int use2 = critical_var + use1;
    int use3 = critical_var - use2;
    int use4 = critical_var * use3;
    int use5 = critical_var / (use4 + 1);
    
    /* Force critical_var to be live across these operations */
    critical_var = use1 + use2 + use3 + use4 + use5;
    
    /* More operations on critical_var to increase reference count */
    float f_crit = critical_var * 1.5f;
    double d_crit = critical_var * 2.5;
    critical_var = (int)(f_crit + d_crit) ^ critical_var;
    
    /* Artificial register pressure via inline assembly */
    /* Clobber many physical registers to force pseudo-register usage */
    asm volatile (
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" (critical_var), "r" (use5)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Inter-procedural calls to increase cross-function pressure */
    struct MultiRegStruct s1 = {
        .vec_int = v5,
        .vec_float = vf3,
        .vec_double = vd1,
        .extra = {l5, critical_var}
    };
    
    struct MultiRegStruct s2 = {
        .vec_int = v4,
        .vec_float = vf2,
        .vec_double = vd2,
        .extra = {l4, use4}
    };
    
    /* These calls create additional register pressure across boundaries */
    struct MultiRegStruct s3 = helper_func1(s1, s2);
    struct MultiRegStruct s4 = helper_func2(s3, critical_var, f_crit, d_crit);
    
    /* Vector helper call */
    v4si v6 = vector_helper(v5, v4, v3);
    
    /* Final computation using all variables to ensure they're live */
    long long result = 
        (long long)a10 + (long long)f10 + (long long)d10 +
        l5 + 
        v6[0] + v6[1] + v6[2] + v6[3] +
        (long long)s4.extra[0] + (long long)s4.extra[1] +
        (long long)critical_var;
    
    /* One more use of critical_var in a way that might trigger remat */
    result ^= (critical_var * 2) | (critical_var >> 3);
    
    return result;
}

int main(void) 
{
    long long total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Mix different seeds to prevent pattern recognition */
        int seed = (i * 1234567) ^ 0xABCDEF;
        total += test_function(seed);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    return (int)(total % 1000);
}
