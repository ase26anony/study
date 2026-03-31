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

/* Volatile variables to prevent optimization and create artificial dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, float *restrict e, int n) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float u1, u2, u3, u4, u5, u6, u7, u8, u9, u10;
    float v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Unrolled loop with many independent operations */
    for (i = 0; i < n - 3; i += 4) {
        /* Group 1: Independent floating-point operations */
        t1 = a[i] * b[i] + c[i];
        t2 = a[i+1] * b[i+1] + c[i+1];
        t3 = a[i+2] * b[i+2] + c[i+2];
        t4 = a[i+3] * b[i+3] + c[i+3];
        
        /* Group 2: More independent operations */
        u1 = d[i] / (e[i] + 1.0f);  /* Division creates longer latency */
        u2 = d[i+1] / (e[i+1] + 1.0f);
        u3 = d[i+2] / (e[i+2] + 1.0f);
        u4 = d[i+3] / (e[i+3] + 1.0f);
        
        /* Group 3: Cross-dependent operations */
        v1 = t1 + u1 * vol_f1;  /* volatile dependency creates scheduling barrier */
        v2 = t2 + u2 * vol_f2;
        v3 = t3 + u3 * vol_f3;
        v4 = t4 + u4 * vol_f1;
        
        /* Group 4: Memory operations with address calculations */
        a[i] = v1 * t2 - u3;
        a[i+1] = v2 * t3 - u4;
        a[i+2] = v3 * t4 - u1;
        a[i+3] = v4 * t1 - u2;
        
        /* More operations to increase pressure */
        t5 = b[i] * c[i] - d[i];
        t6 = b[i+1] * c[i+1] - d[i+1];
        t7 = b[i+2] * c[i+2] - d[i+2];
        t8 = b[i+3] * c[i+3] - d[i+3];
        
        u5 = e[i] * vol_f1 + t5;  /* volatile read */
        u6 = e[i+1] * vol_f2 + t6;
        u7 = e[i+2] * vol_f3 + t7;
        u8 = e[i+3] * vol_f1 + t8;
        
        /* Store results */
        b[i] = u5; b[i+1] = u6; b[i+2] = u7; b[i+3] = u8;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      int *restrict arr3, int n) {
    int i;
    /* Create artificial resource conflicts with inline asm */
    for (i = 0; i < n; i++) {
        int temp1, temp2, temp3, temp4;
        
        /* Independent integer operations */
        temp1 = arr1[i] * vol_a;  /* volatile read creates delay */
        temp2 = arr2[i] + vol_b;
        temp3 = arr3[i] - vol_c;
        temp4 = temp1 ^ temp2;
        
        /* Inline asm to clobber registers and create pressure */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (temp4)
            : "r" (temp3)
            : "%eax", "%ebx", "cc"
        );
        
        /* More operations with different latencies */
        arr1[i] = temp4 * 7;
        arr2[i] = (temp1 + temp2) >> 2;
        
        /* Conditional to create control flow and priority differences */
        if (arr3[i] > 1000) {
            arr3[i] = arr3[i] / 3;  /* Division has longer latency */
        } else {
            arr3[i] = arr3[i] * 3;
        }
        
        /* Memory barrier-like operation */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(float *restrict a, float *restrict b, int n) {
    int i;
    /* Many independent operations that can be reordered */
    for (i = 0; i < n; i++) {
        /* Group of independent instructions */
        float r1 = a[i] * 1.1f;
        float r2 = b[i] * 2.2f;
        float r3 = a[i] + 3.3f;
        float r4 = b[i] - 4.4f;
        float r5 = r1 * r2;
        float r6 = r3 / r4;
        float r7 = r5 + r6;
        float r8 = r1 - r2;
        float r9 = r3 * r4;
        float r10 = r7 * r8;
        
        /* Store results preventing dead code elimination */
        a[i] = r9 + vol_f1;  /* volatile dependency */
        b[i] = r10 * vol_f2;
        
        /* More independent groups */
        float s1 = r1 * r3;
        float s2 = r2 * r4;
        float s3 = s1 + s2;
        float s4 = s1 - s2;
        
        /* Use results */
        a[i] += s3;
        b[i] += s4;
    }
}

int main() {
    int i;
    float *array1, *array2, *array3, *array4, *array5;
    int *int_arr1, *int_arr2, *int_arr3;
    
    /* Allocate and initialize arrays */
    array1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array3 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array4 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array5 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    int_arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int_arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int_arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX;
        array2[i] = (float)rand() / RAND_MAX;
        array3[i] = (float)rand() / RAND_MAX;
        array4[i] = (float)rand() / RAND_MAX;
        array5[i] = (float)rand() / RAND_MAX;
        
        int_arr1[i] = rand() % 2000;
        int_arr2[i] = rand() % 2000;
        int_arr3[i] = rand() % 2000;
    }
    
    /* Perform computation many times to ensure scheduler sees hot code */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call functions with different characteristics */
        high_pressure_loop(array1, array2, array3, array4, array5, ARRAY_SIZE);
        mixed_dependency(int_arr1, int_arr2, int_arr3, ARRAY_SIZE);
        independent_instructions(array1, array2, ARRAY_SIZE);
        
        /* Update volatile variables to create changing dependencies */
        if (i % 100 == 0) {
            vol_a = (vol_a * 3) % 17;
            vol_b = (vol_b + 5) % 23;
            vol_c = (vol_c * 2) % 29;
            vol_f1 = sinf(vol_f1 + 0.1f);
            vol_f2 = cosf(vol_f2 + 0.2f);
            vol_f3 = tanf(vol_f3 + 0.3f);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float sum = 0.0f;
    int int_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array1[i] + array2[i] + array3[i] + array4[i] + array5[i];
        int_sum += int_arr1[i] + int_arr2[i] + int_arr3[i];
    }
    
    printf("Checksum: float=%f, int=%d\n", sum, int_sum);
    
    /* Cleanup */
    free(array1); free(array2); free(array3); free(array4); free(array5);
    free(int_arr1); free(int_arr2); free(int_arr3);
    
    return 0;
}
