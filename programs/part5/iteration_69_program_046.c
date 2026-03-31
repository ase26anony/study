/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_coverage_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create artificial dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, float* restrict result) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    float t21, t22, t23, t24, t25, t26, t27, t28, t29, t30;
    
    /* Unrolled loop with many independent operations */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i] + vol_float1;
        t2 = a[i+1] / b[i+1];  /* Division has longer latency */
        t3 = c[i] * d[i] - vol_float2;
        t4 = c[i+1] / d[i+1];  /* Another division */
        
        /* Group 2: More independent operations */
        t5 = t1 * t2;
        t6 = t3 * t4;
        t7 = t1 + t3;
        t8 = t2 + t4;
        
        /* Group 3: Cross dependencies to create priority differences */
        t9 = t5 * vol_var1;
        t10 = t6 * vol_var2;
        t11 = t7 / (vol_var1 + 1);
        t12 = t8 / (vol_var2 + 1);
        
        /* Group 4: Memory operations mixed with computation */
        t13 = a[i+2] * b[i+2];
        t14 = c[i+2] * d[i+2];
        t15 = t13 + t14;
        t16 = t13 - t14;
        
        /* Group 5: More operations to increase pressure */
        t17 = t9 * t10;
        t18 = t11 * t12;
        t19 = t15 * t16;
        t20 = t17 + t18;
        
        /* Final computations with artificial dependencies on volatiles */
        t21 = t19 * vol_float1;
        t22 = t20 * vol_float2;
        t23 = t21 / (vol_var1 ? 2.0f : 1.0f);
        t24 = t22 / (vol_var2 ? 3.0f : 1.0f);
        
        /* Store results with memory barrier effect */
        result[i] = t23;
        result[i+1] = t24;
        
        /* Repeat pattern for next set */
        t25 = a[i+3] * b[i+3];
        t26 = c[i+3] * d[i+3];
        t27 = t25 * t26;
        t28 = t25 / t26;  /* Another division */
        
        t29 = t27 * vol_float1;
        t30 = t28 * vol_float2;
        
        result[i+2] = t29;
        result[i+3] = t30;
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile("" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                     "xmm4", "xmm5", "xmm6", "xmm7");
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency_pattern(int* restrict arr1, int* restrict arr2, 
                              int* restrict arr3, int* restrict out) {
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Create chains of dependencies with different latencies */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Chain 1: Simple arithmetic */
        temp1 = arr1[i] + vol_var1;
        temp2 = temp1 * arr2[i];
        temp3 = temp2 - arr3[i];
        
        /* Chain 2: Independent chain */
        temp4 = arr2[i] * vol_var2;
        temp5 = temp4 / (vol_var1 + 1);  /* Integer division */
        temp6 = temp5 + arr1[i];
        
        /* Chain 3: Memory intensive with volatile */
        temp7 = arr3[i] + (int)vol_float1;
        temp8 = temp7 * (int)vol_float2;
        
        /* Resource conflict: use same functional units */
        out[i] = temp3 + temp6 + temp8;
        
        /* Conditional to create control flow and priority differences */
        if (out[i] > 1000) {
            out[i] = out[i] / 2;  /* Another division */
        } else {
            out[i] = out[i] * 3;
        }
        
        /* Artificial delay through volatile dependency */
        asm volatile("" : "+r"(out[i]) : "r"(vol_var1), "r"(vol_var2));
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instruction_blocks(float* data, int size) {
    /* Block of independent instructions - scheduler has many choices */
    float a = data[0], b = data[1], c = data[2], d = data[3];
    float e = data[4], f = data[5], g = data[6], h = data[7];
    
    /* These can be scheduled in any order */
    float r1 = a * b + c;
    float r2 = d * e - f;
    float r3 = g / h;           /* Division - different latency */
    float r4 = a + b + c + d;
    float r5 = e * f * g;
    float r6 = h / a;           /* Another division */
    float r7 = b * c * d * e;
    float r8 = f + g + h;
    float r9 = a / c;           /* More division */
    float r10 = b * d * f * h;
    
    /* Create some dependencies to mix priorities */
    float s1 = r1 + r2;
    float s2 = r3 * r4;
    float s3 = r5 / r6;         /* Division chain */
    float s4 = r7 - r8;
    float s5 = r9 * r10;
    
    /* Store results to prevent elimination */
    data[0] = s1; data[1] = s2; data[2] = s3;
    data[3] = s4; data[4] = s5;
    
    /* More independent operations */
    float t1 = s1 * s2;
    float t2 = s3 * s4;
    float t3 = s5 * vol_float1;
    float t4 = t1 / t2;         /* Division with dependency */
    float t5 = t3 * vol_float2;
    
    data[5] = t4; data[6] = t5;
}

int main() {
    /* Allocate and initialize arrays with random data */
    float* fa1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa4 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fresult = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    int* ia1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* ia2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* ia3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* iresult = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa1[i] = (float)rand() / RAND_MAX * 100.0f;
        fa2[i] = (float)rand() / RAND_MAX * 100.0f;
        fa3[i] = (float)rand() / RAND_MAX * 100.0f;
        fa4[i] = (float)rand() / RAND_MAX * 100.0f;
        ia1[i] = rand() % 1000;
        ia2[i] = rand() % 1000;
        ia3[i] = rand() % 1000;
    }
    
    /* Perform computation many times to ensure scheduler sees hot code */
    float checksum = 0.0f;
    int int_checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify volatile variables to create changing dependencies */
        vol_var1 = (iter % 100) + 1;
        vol_var2 = (iter % 50) + 1;
        vol_float1 = (float)(iter % 10) + 0.5f;
        vol_float2 = (float)(iter % 20) + 0.5f;
        
        /* Call functions that create different scheduling scenarios */
        high_pressure_loop(fa1, fa2, fa3, fa4, fresult);
        mixed_dependency_pattern(ia1, ia2, ia3, iresult);
        independent_instruction_blocks(fresult, 16);
        
        /* Accumulate checksums to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (i % 32 == 0) {
                checksum += fresult[i];
                int_checksum += iresult[i];
            }
        }
    }
    
    /* Print results to ensure computation isn't optimized away */
    printf("Checksums: float=%f, int=%d\n", checksum, int_checksum);
    
    /* Cleanup */
    free(fa1); free(fa2); free(fa3); free(fa4); free(fresult);
    free(ia1); free(ia2); free(ia3); free(iresult);
    
    return 0;
}
