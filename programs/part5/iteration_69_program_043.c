/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, float *restrict result) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Unrolled loop with many independent operations */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i] + c[i];
        t2 = a[i+1] * b[i+1] - c[i+1];
        t3 = a[i+2] / (b[i+2] + 1.0f);
        t4 = a[i+3] * b[i+3] * c[i+3];
        
        /* Group 2: More independent operations */
        t5 = d[i] * t1;
        t6 = d[i+1] * t2;
        t7 = d[i+2] * t3;
        t8 = d[i+3] * t4;
        
        /* Group 3: Integer operations mixed in */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = (int)t3;
        i4 = (int)t4;
        
        /* Group 4: More floating point with dependencies */
        t9 = t5 * vol_float1;
        t10 = t6 * vol_float2;
        t11 = t7 + vol_float1;
        t12 = t8 - vol_float2;
        
        /* Group 5: Integer operations creating delays */
        i5 = i1 * vol_var1;
        i6 = i2 * vol_var2;
        i7 = i3 + vol_var1;
        i8 = i4 - vol_var2;
        
        /* Group 6: Final computations with mixed types */
        t13 = t9 + (float)i5;
        t14 = t10 + (float)i6;
        t15 = t11 * (float)i7;
        t16 = t12 / (float)(i8 + 1);
        
        /* Store results - creates memory pressure too */
        result[i] = t13;
        result[i+1] = t14;
        result[i+2] = t15;
        result[i+3] = t16;
        
        /* Second half of unrolled iteration */
        t17 = a[i+4] * b[i+4] + c[i+4];
        t18 = a[i+5] * b[i+5] - c[i+5];
        t19 = a[i+6] / (b[i+6] + 1.0f);
        t20 = a[i+7] * b[i+7] * c[i+7];
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        result[i+4] = t17 * d[i+4];
        result[i+5] = t18 * d[i+5];
        result[i+6] = t19 * d[i+6];
        result[i+7] = t20 * d[i+7];
    }
}

/* Function with artificial dependencies and delays */
__attribute__((noinline))
void mixed_dependency_chain(int *restrict arr1, int *restrict arr2, 
                           float *restrict farr1, float *restrict farr2) {
    /* Create long dependency chains */
    int dep1 = vol_var1;
    int dep2 = vol_var2;
    float fdep1 = vol_float1;
    float fdep2 = vol_float2;
    
    /* Mixed operation types to compete for functional units */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Integer dependency chain */
        dep1 = arr1[i] * dep1 + arr2[i];
        dep2 = arr2[i] * dep2 - arr1[i];
        
        /* Floating point dependency chain - slow operations */
        fdep1 = farr1[i] / (fdep1 + 1.0f);  /* Division has high latency */
        fdep2 = sqrtf(fabsf(farr2[i] * fdep2)); /* sqrt has high latency */
        
        /* Memory operations with aliasing concerns */
        arr1[i] = dep1 + (int)fdep1;
        arr2[i] = dep2 + (int)fdep2;
        
        /* Conditional to create control flow and priority differences */
        if (i % 3 == 0) {
            farr1[i] = fdep1 * 2.0f;
        } else if (i % 3 == 1) {
            farr2[i] = fdep2 / 2.0f;
        } else {
            /* Complex expression with many operations */
            farr1[i] = (fdep1 + fdep2) * (fdep1 - fdep2) / 
                      (fdep1 * fdep2 + 1.0f);
        }
        
        /* Volatile access creates memory barrier and scheduling boundaries */
        int barrier = vol_var1;
        asm volatile("" : "+r" (barrier) : : "memory");
    }
}

/* Function with independent instruction groups for candidate selection */
__attribute__((noinline))
void independent_instruction_groups(float *restrict out, 
                                   const float *restrict in1,
                                   const float *restrict in2,
                                   const float *restrict in3) {
    /* Many independent instructions that can be reordered */
    float g1_a, g1_b, g1_c, g1_d;
    float g2_a, g2_b, g2_c, g2_d;
    float g3_a, g3_b, g3_c, g3_d;
    float g4_a, g4_b, g4_c, g4_d;
    
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        /* Group 1: Completely independent computations */
        g1_a = in1[i] * 1.1f;
        g1_b = in1[i+1] * 2.2f;
        g1_c = in1[i+2] * 3.3f;
        g1_d = in1[i+3] * 4.4f;
        
        /* Group 2: Another independent set */
        g2_a = in2[i] + 10.0f;
        g2_b = in2[i+1] + 20.0f;
        g2_c = in2[i+2] + 30.0f;
        g2_d = in2[i+3] + 40.0f;
        
        /* Group 3: More independent operations */
        g3_a = g1_a * g2_a;
        g3_b = g1_b * g2_b;
        g3_c = g1_c * g2_c;
        g3_d = g1_d * g2_d;
        
        /* Group 4: Final independent computations */
        g4_a = g3_a + in3[i];
        g4_b = g3_b + in3[i+1];
        g4_c = g3_c + in3[i+2];
        g4_d = g3_d + in3[i+3];
        
        /* Store results - all can be scheduled independently */
        out[i] = g4_a;
        out[i+1] = g4_b;
        out[i+2] = g4_c;
        out[i+3] = g4_d;
    }
}

int main() {
    /* Allocate and initialize data */
    float *a = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *result = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    int *arr1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *farr1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *farr2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f;
        c[i] = (float)rand() / RAND_MAX * 100.0f;
        d[i] = (float)rand() / RAND_MAX * 100.0f;
        
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 50.0f;
        farr2[i] = (float)rand() / RAND_MAX * 50.0f;
    }
    
    /* Perform many iterations to ensure scheduler sees hot code */
    float total = 0.0f;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(a, b, c, d, result);
        mixed_dependency_chain(arr1, arr2, farr1, farr2);
        independent_instruction_groups(result, a, b, c);
        
        /* Use results to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            total += result[i] + arr1[i] + farr1[i];
        }
        
        /* Modify volatile variables to change scheduling conditions */
        if (iter % 100 == 0) {
            vol_var1 = (vol_var1 * 3 + 1) % 100;
            vol_var2 = (vol_var2 * 5 + 1) % 100;
            vol_float1 = sinf((float)iter) * 10.0f;
            vol_float2 = cosf((float)iter) * 10.0f;
        }
    }
    
    /* Final output to prevent optimization */
    printf("Result checksum: %f\n", total);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(result);
    free(arr1); free(arr2); free(farr1); free(farr2);
    
    return 0;
}
