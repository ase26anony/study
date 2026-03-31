/* Main test file to force early rematerialization of pseudo-registers */
#include <stdint.h>
#include <stdio.h>

/* External helper functions to increase register pressure */
extern struct DataPair helper1(int a, int b, int c, int d);
extern struct DataPair helper2(float a, float b, double c, double d);
extern struct DataPair helper3(long a, long b, int64_t c, int64_t d);

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static long test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    int64_t i64_1, i64_2, i64_3, i64_4;
    
    /* Vector variables for wide register pressure */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with volatile to prevent constant propagation */
    a1 = seed + g_volatile_counter;
    f1 = g_volatile_float + seed;
    d1 = g_volatile_double * seed;
    l1 = (long)seed * g_volatile_counter;
    
    /* Complex chain of interdependent computations */
    /* First chain: integer operations */
    a2 = a1 * 3 + 7;
    a3 = a2 / 2 - a1;
    a4 = a3 << 3;
    a5 = a4 ^ a2;
    a6 = a5 | a3;
    a7 = a6 & 0xFFFF;
    a8 = a7 + a4 - a2;
    a9 = a8 * 11 % 97;
    a10 = a9 ^ a7 | a5;
    
    /* Second chain: floating point operations */
    f2 = f1 * 2.0f;
    f3 = f2 + f1 / 3.0f;
    f4 = f3 - f2 * 0.5f;
    f5 = f4 * f3;
    f6 = f5 / (f2 + 1.0f);
    f7 = f6 - f4 + f3;
    f8 = f7 * 2.0f - f5;
    
    /* Third chain: double precision */
    d2 = d1 * 1.5;
    d3 = d2 + d1 / 2.0;
    d4 = d3 - d2 * 0.25;
    d5 = d4 * d3;
    d6 = d5 / (d2 + 1.0);
    
    /* Fourth chain: long integers */
    l2 = l1 * 3L + 7L;
    l3 = l2 / 2L - l1;
    l4 = l3 << 2;
    l5 = l4 ^ l2;
    l6 = l5 | l3;
    l7 = l6 & 0xFFFFFFFFL;
    l8 = l7 + l4 - l2;
    
    /* Fifth chain: 64-bit integers */
    i64_1 = (int64_t)l1 * a1;
    i64_2 = i64_1 + (int64_t)a2 * 3;
    i64_3 = i64_2 ^ i64_1;
    i64_4 = i64_3 | (int64_t)a3;
    
    /* Vector operations - these use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 ^ v4;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 * vf2 + vf1;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many physical registers to force pseudo-register usage */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        : 
        : "r" (a1), "r" (a2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More interdependent operations to create dataflow references */
    /* This pattern may trigger the specific replacement logic */
    int temp1 = a10 + f8;
    int temp2 = temp1 * d6;
    long temp3 = temp2 + l8;
    int64_t temp4 = temp3 * i64_4;
    
    /* Use the result in multiple ways to create register references */
    int result1 = temp1 + temp2;
    float result2 = temp1 * f8;
    double result3 = temp2 * d6;
    long result4 = temp3 + l8;
    
    /* Force register pressure with another inline assembly */
    asm volatile(
        "# More pressure\n"
        : 
        : 
        : "r14", "r15", "cc", "memory"
    );
    
    /* Final computation using all temporaries */
    long final_result = 
        (long)result1 + 
        (long)result2 + 
        (long)result3 + 
        result4 + 
        (long)temp4 +
        v5[0] + v5[1] + v5[2] + v5[3] +
        (long)(vf3[0] + vf3[1] + vf3[2] + vf3[3]) +
        (long)(vd1[0] + vd1[1] + vd2[0] + vd2[1]);
    
    return final_result;
}

/* Hot loop to increase compilation significance */
int main() {
    long total = 0;
    int iterations = g_volatile_counter;
    
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %ld\n", total);
    return (int)(total % 1000);
}
