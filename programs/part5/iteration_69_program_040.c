/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Group 1: Independent floating-point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);  /* Division for latency */
        
        /* Group 2: More independent operations */
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 0.001f);
        
        /* Group 3: Cross dependencies to create priority differences */
        t9 = t1 + t5;   /* Depends on group 1 and 2 */
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / t8;  /* Division with dependency */
        
        /* Group 4: More operations to increase pressure */
        t13 = t9 * vol_f1;  /* Volatile dependency */
        t14 = t10 + vol_f2;
        t15 = t11 - vol_f3;
        t16 = t12 / 2.0f;
        
        /* Group 5: Final computations with mixed operations */
        t17 = t13 + t14;
        t18 = t15 * t16;
        t19 = t17 - t18;
        t20 = sqrtf(fabsf(t19));  /* High latency operation */
        
        /* Store results back, creating memory pressure */
        a[i] = t1 + t20;
        b[i] = t2 - t20;
        c[i] = t3 * t20;
        d[i] = t4 / (t20 + 0.001f);
        
        /* Artificial dependency on volatile to prevent reordering */
        if (vol_a > 0) {
            t1 += 0.1f;
        }
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      int *restrict arr3, int n) {
    int i;
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    for (i = 0; i < n; i++) {
        /* Independent integer operations - candidates for scheduling */
        r1 = arr1[i] * 3;
        r2 = arr1[i] + 5;
        r3 = arr1[i] - 7;
        r4 = arr1[i] / 2;
        
        r5 = arr2[i] * 11;
        r6 = arr2[i] + 13;
        r7 = arr2[i] - 17;
        r8 = arr2[i] / 19;
        
        /* Create dependencies that will cause delays */
        r9 = (r1 * r5) / (vol_b + 1);  /* Volatile dependency */
        r10 = (r2 + r6) * (vol_c - 1);
        
        /* Long latency operation with volatile */
        if (vol_d > 0) {
            /* Inline assembly to create specific register pressure */
            __asm__ volatile (
                "mov %0, %%eax\n\t"
                "imul %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "+r" (r9)
                : "r" (r3), "r" (r7)
                : "%eax", "cc"
            );
        }
        
        /* More operations with mixed types */
        float fr1 = (float)r9 * 1.5f;
        float fr2 = (float)r10 * 2.5f;
        
        /* Division - high latency */
        fr1 = fr1 / (fr2 + 0.0001f);
        
        /* Store with conditional to create control flow */
        arr3[i] = (int)fr1;
        if (i % 16 == 0) {
            arr3[i] += vol_a;  /* Volatile access creates memory barrier */
        }
    }
}

/* Function with many independent instructions in basic block */
__attribute__((noinline))
void independent_instructions(float *restrict out, float *restrict in, int n) {
    int i;
    
    for (i = 0; i < n; i += 8) {
        /* Eight independent groups of operations */
        float a0 = in[i] * 1.1f;
        float a1 = in[i+1] * 1.2f;
        float a2 = in[i+2] * 1.3f;
        float a3 = in[i+3] * 1.4f;
        float a4 = in[i+4] * 1.5f;
        float a5 = in[i+5] * 1.6f;
        float a6 = in[i+6] * 1.7f;
        float a7 = in[i+7] * 1.8f;
        
        /* More independent operations */
        float b0 = a0 + 10.0f;
        float b1 = a1 - 10.0f;
        float b2 = a2 * 2.0f;
        float b3 = a3 / 2.0f;  /* Division for latency */
        float b4 = sqrtf(fabsf(a4));
        float b5 = sinf(a5);
        float b6 = cosf(a6);
        float b7 = a7 * a7;
        
        /* Cross dependencies */
        float c0 = b0 + b4;
        float c1 = b1 * b5;
        float c2 = b2 - b6;
        float c3 = b3 / (b7 + 0.001f);
        
        /* Final results */
        out[i] = c0;
        out[i+1] = c1;
        out[i+2] = c2;
        out[i+3] = c3;
        out[i+4] = c0 * c1;
        out[i+5] = c1 - c2;
        out[i+6] = c2 / (c3 + 0.001f);
        out[i+7] = c3 + c0;
    }
}

int main() {
    float *array1, *array2, *array3, *array4;
    int *int_arr1, *int_arr2, *int_arr3;
    int i;
    float checksum = 0.0f;
    
    /* Allocate and initialize arrays */
    array1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array3 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array4 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    int_arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int_arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int_arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX * 100.0f;
        array2[i] = (float)rand() / RAND_MAX * 100.0f;
        array3[i] = (float)rand() / RAND_MAX * 100.0f;
        array4[i] = (float)rand() / RAND_MAX * 100.0f;
        
        int_arr1[i] = rand() % 1000;
        int_arr2[i] = rand() % 1000;
        int_arr3[i] = 0;
    }
    
    /* Perform computations to trigger scheduling analysis */
    for (i = 0; i < ITERATIONS; i++) {
        high_pressure_loop(array1, array2, array3, array4, ARRAY_SIZE/4);
        mixed_dependency(int_arr1, int_arr2, int_arr3, ARRAY_SIZE/4);
        independent_instructions(array1, array2, ARRAY_SIZE/8);
        
        /* Modify volatile variables to affect scheduling */
        if (i % 1000 == 0) {
            vol_a = (vol_a * 3) % 17;
            vol_b = (vol_b + 5) % 23;
            vol_c = (vol_c * 7) % 29;
            vol_d = (vol_d - 3) % 31;
            vol_f1 = (float)vol_a;
            vol_f2 = (float)vol_b;
            vol_f3 = (float)vol_c;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i] 
                  + (float)int_arr1[i] + (float)int_arr2[i] + (float)int_arr3[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Free memory */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    
    return 0;
}
