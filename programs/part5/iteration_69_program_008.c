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

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, 
                       float *restrict c, float *restrict d, int n) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
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
        t9 = t1 + t5;
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / (t8 + 0.001f);
        
        /* Integer operations mixed in */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = (int)t3;
        i4 = (int)t4;
        
        /* More operations to increase pressure */
        t13 = t9 * t10;
        t14 = t11 + t12;
        t15 = t13 - t14;
        t16 = t13 / (t14 + 0.001f);
        
        /* Use volatile to force memory dependencies */
        t17 = t15 * vol_f1;
        t18 = t16 + vol_f2;
        
        /* Store results to prevent elimination */
        a[i] = t17 + t18;
        b[i] = t13 - t14;
        c[i] = t9 * vol_f3;
        d[i] = t10 / vol_f1;
        
        /* Integer operations with different priorities */
        i5 = i1 + i2;
        i6 = i3 * i4;
        i7 = i5 - i6;
        i8 = i5 / (abs(i6) + 1);
        
        /* Use all variables to keep them live */
        t19 = (float)(i7 + i8);
        t20 = t19 * 0.5f;
        
        /* Final store using all computed values */
        a[i] += t20;
    }
}

/* Function with artificial delays and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                     float *restrict farr1, float *restrict farr2, int n) {
    /* Create long latency operations and dependencies */
    for (int i = 0; i < n; i++) {
        /* Volatile reads create memory dependencies and delays */
        int v1 = vol_a;
        int v2 = vol_b;
        int v3 = vol_c;
        int v4 = vol_d;
        
        /* Independent chains with different lengths */
        int chain1 = arr1[i] + v1;
        int chain2 = arr2[i] * v2;
        int chain3 = chain1 - v3;
        int chain4 = chain2 / (abs(v4) + 1);
        
        /* Floating point ops compete for different functional units */
        float fchain1 = farr1[i] * 1.234567f;
        float fchain2 = farr2[i] / 3.141592f;  /* Division for latency */
        float fchain3 = fchain1 + fchain2;
        float fchain4 = fchain1 - fchain2;
        
        /* Mix integer and float operations */
        chain1 = (int)(fchain3 * 100.0f);
        chain2 = (int)(fchain4 * 100.0f);
        
        /* Create conditional to affect priority */
        if (chain1 > chain2) {
            fchain3 = fchain3 * 2.0f;
            chain3 = chain1 - chain2;
        } else {
            fchain3 = fchain3 / 2.0f;
            chain3 = chain2 - chain1;
        }
        
        /* More operations to create scheduling candidates */
        float fchain5 = fchain3 * fchain4;
        int chain5 = chain3 * chain4;
        
        /* Use inline assembly to clobber registers and create pressure */
        asm volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (chain5)
            : "r" (v1)
            : "%eax", "%ebx"
        );
        
        /* Store results */
        arr1[i] = chain3 + chain5;
        arr2[i] = chain4;
        farr1[i] = fchain3 + fchain5;
        farr2[i] = fchain4;
        
        /* Update volatiles to create memory dependencies */
        vol_a = chain1;
        vol_b = chain2;
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(float *restrict a, float *restrict b, int n) {
    /* Many independent statements that can be reordered */
    for (int i = 0; i < n; i += 8) {
        /* Group 1: Completely independent operations */
        float t1 = a[i] * 1.1f;
        float t2 = b[i] * 2.2f;
        float t3 = a[i+1] * 3.3f;
        float t4 = b[i+1] * 4.4f;
        
        /* Group 2: More independent operations */
        float t5 = a[i+2] + 5.5f;
        float t6 = b[i+2] + 6.6f;
        float t7 = a[i+3] - 7.7f;
        float t8 = b[i+3] - 8.8f;
        
        /* Group 3: Yet more independent operations */
        float t9 = t1 + t2;
        float t10 = t3 * t4;
        float t11 = t5 / (t6 + 0.001f);
        float t12 = t7 - t8;
        
        /* Group 4: Final independent computations */
        float t13 = t9 * t10;
        float t14 = t11 + t12;
        float t15 = t13 / (t14 + 0.001f);
        float t16 = t13 - t14;
        
        /* Store results in different patterns */
        a[i] = t15;
        b[i] = t16;
        a[i+1] = t9;
        b[i+1] = t10;
        a[i+2] = t11;
        b[i+2] = t12;
        a[i+3] = t13;
        b[i+3] = t14;
        
        /* More independent groups for larger candidate sets */
        float u1 = a[i+4] * 9.9f;
        float u2 = b[i+4] * 10.1f;
        float u3 = a[i+5] / 11.1f;
        float u4 = b[i+5] / 12.1f;
        
        float u5 = u1 + u2;
        float u6 = u3 * u4;
        float u7 = u5 - u6;
        float u8 = u5 / (u6 + 0.001f);
        
        a[i+4] = u7;
        b[i+4] = u8;
        a[i+5] = u5;
        b[i+5] = u6;
    }
}

int main() {
    /* Allocate and initialize arrays */
    float *a = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *d = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
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
        farr1[i] = (float)rand() / RAND_MAX * 100.0f;
        farr2[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    /* Perform computation many times to ensure scheduling analysis */
    float total = 0.0f;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(a, b, c, d, ARRAY_SIZE);
        mixed_dependency(arr1, arr2, farr1, farr2, ARRAY_SIZE);
        independent_instructions(a, b, ARRAY_SIZE);
        
        /* Update volatiles to create varying conditions */
        vol_a = (vol_a * 1103515245 + 12345) & 0x7fffffff;
        vol_b = (vol_b * 1103515245 + 12345) & 0x7fffffff;
        vol_f1 = sinf((float)iter * 0.01f);
        vol_f2 = cosf((float)iter * 0.01f);
        
        /* Accumulate checksum to prevent dead code elimination */
        total += a[iter % ARRAY_SIZE] + b[iter % ARRAY_SIZE] +
                c[iter % ARRAY_SIZE] + d[iter % ARRAY_SIZE];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2); free(farr1); free(farr2);
    
    return 0;
}
