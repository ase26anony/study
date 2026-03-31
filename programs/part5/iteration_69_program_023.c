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
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
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
        
        /* Group 3: Integer operations mixed in */
        i1 = (int)(t1 * 100);
        i2 = (int)(t2 * 100);
        i3 = (int)(t3 * 100);
        i4 = (int)(t4 * 100);
        
        /* Group 4: Cross dependencies to create priority differences */
        t9 = t1 + t5;
        t10 = t2 + t6;
        t11 = t3 + t7;
        t12 = t4 + t8;
        
        /* Group 5: More operations to increase pressure */
        t13 = t9 * t10;
        t14 = t11 * t12;
        t15 = t13 - t14;
        t16 = t13 + t14;
        
        /* Use volatile variables to force memory dependencies */
        t17 = t15 * vol_float1;
        t18 = t16 * vol_float2;
        
        /* Store results back */
        a[i] = t17 + t18;
        b[i] = t17 - t18;
        c[i] = t13;
        d[i] = t14;
        
        /* Integer operations with volatile */
        i5 = i1 + vol_var1;
        i6 = i2 + vol_var2;
        i7 = i3 * vol_var1;
        i8 = i4 / (vol_var2 + 1);
        
        /* More register pressure */
        t19 = (float)i5 + (float)i6;
        t20 = (float)i7 * (float)i8;
        
        /* Final store with dependency chain */
        a[i] += t19 + t20;
    }
}

/* Function with artificial resource conflicts and delays */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr1, float *restrict farr2, int n) {
    /* Create long dependency chains */
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Long latency floating point operations */
        float f1 = farr1[i] / (farr2[i] + 0.0001f);  /* Division has high latency */
        float f2 = sqrtf(fabsf(farr1[i]));           /* Function call latency */
        float f3 = f1 * f2;
        float f4 = f3 / (vol_float1 + 0.001f);
        
        /* Integer operations that can be scheduled independently */
        int x1 = arr1[i] * arr2[i];
        int x2 = arr1[i] + arr2[i];
        int x3 = arr1[i] - arr2[i];
        int x4 = arr1[i] & arr2[i];
        int x5 = arr1[i] | arr2[i];
        int x6 = arr1[i] ^ arr2[i];
        
        /* Mix float and int operations to compete for resources */
        acc1 += f1 + (float)x1;
        acc2 += f2 + (float)x2;
        acc3 += f3 + (float)x3;
        acc4 += f4 + (float)x4;
        
        sum1 += x1 + x5;
        sum2 += x2 + x6;
        sum3 += x3 * vol_var1;
        sum4 += x4 / (vol_var2 + 1);
        
        /* Memory operations with potential aliasing */
        arr1[i] = sum1 + (int)acc1;
        arr2[i] = sum2 + (int)acc2;
        farr1[i] = acc3;
        farr2[i] = acc4;
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx\n\t"
            "movl $0, %%esi\n\t"
            "movl $0, %%edi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
    
    /* Prevent dead code elimination */
    vol_var1 = sum1 + sum2;
    vol_var2 = sum3 + sum4;
    vol_float1 = acc1 + acc2;
    vol_float2 = acc3 + acc4;
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(int *restrict out, int *restrict in1, 
                              int *restrict in2, int n) {
    /* This function has many independent instructions that can be reordered */
    for (int i = 0; i < n; i += 4) {
        /* Block of independent operations - scheduler has many choices */
        int t1 = in1[i] * in2[i];
        int t2 = in1[i+1] + in2[i+1];
        int t3 = in1[i+2] - in2[i+2];
        int t4 = in1[i+3] & in2[i+3];
        
        int t5 = t1 * vol_var1;
        int t6 = t2 + vol_var2;
        int t7 = t3 / (vol_var1 + 1);
        int t8 = t4 | vol_var2;
        
        int t9 = t5 ^ t6;
        int t10 = t7 & t8;
        int t11 = t9 + t10;
        int t12 = t5 - t6;
        
        int t13 = t11 * t12;
        int t14 = t7 | t8;
        int t15 = t9 & t10;
        int t16 = t11 ^ t12;
        
        out[i] = t13 + t14;
        out[i+1] = t15 - t16;
        out[i+2] = t13 & t15;
        out[i+3] = t14 | t16;
    }
}

int main() {
    /* Allocate and initialize arrays */
    float *fa = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *fb = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *fc = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *fd = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    int *ia = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *ib = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *ic = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *id = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        fc[i] = (float)rand() / RAND_MAX * 100.0f;
        fd[i] = (float)rand() / RAND_MAX * 100.0f;
        
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
        ic[i] = rand() % 1000;
        id[i] = rand() % 1000;
    }
    
    /* Performance-critical loop - this is where scheduling matters */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different patterns to trigger various scheduling scenarios */
        high_pressure_loop(fa, fb, fc, fd, ARRAY_SIZE);
        mixed_dependency(ia, ib, fa, fb, ARRAY_SIZE);
        independent_instructions(ic, ia, ib, ARRAY_SIZE);
        
        /* Prevent loop invariant code motion */
        vol_var1 = (vol_var1 + 1) % 100;
        vol_var2 = (vol_var2 + 1) % 100;
        vol_float1 = vol_float1 * 0.99f;
        vol_float2 = vol_float2 * 0.99f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += ia[i] + ib[i] + ic[i] + id[i];
        float_sum += fa[i] + fb[i] + fc[i] + fd[i];
    }
    
    printf("Checksum: int=%d, float=%f\n", int_sum, float_sum);
    printf("Volatile values: %d, %d, %f, %f\n", 
           vol_var1, vol_var2, vol_float1, vol_float2);
    
    /* Cleanup */
    free(fa); free(fb); free(fc); free(fd);
    free(ia); free(ib); free(ic); free(id);
    
    return 0;
}
