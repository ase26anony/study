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

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, float* restrict e, int n) {
    int i;
    /* Many independent floating point operations to create register pressure */
    for (i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i] + c[i];
        float t2 = b[i] * c[i] - d[i];
        float t3 = c[i] * d[i] + e[i];
        float t4 = d[i] * e[i] - a[i];
        float t5 = e[i] * a[i] + b[i];
        
        /* Group 2: More independent operations */
        float t6 = t1 * t2 + t3;
        float t7 = t2 * t3 - t4;
        float t8 = t3 * t4 + t5;
        float t9 = t4 * t5 - t1;
        float t10 = t5 * t1 + t2;
        
        /* Group 3: Mix with volatile to prevent reordering */
        t6 += vol_float1;
        t7 -= vol_float2;
        t8 *= vol_float1;
        
        /* Store results creating write pressure */
        a[i] = t6 + t7;
        b[i] = t7 - t8;
        c[i] = t8 * t9;
        d[i] = t9 / (t10 + 1.0f);  /* Division for latency */
        e[i] = t10 + t6;
        
        /* Artificial dependency chain with volatile */
        vol_float1 += 0.001f;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Long latency operations mixed with fast ones */
        float fdiv = farr1[i] / farr2[i];  /* FP division - high latency */
        int imul = arr1[i] * arr2[i];      /* Integer multiply */
        float fsqrt = sqrtf(farr1[i]);     /* FP sqrt - high latency */
        
        /* Volatile accesses create scheduling barriers */
        int v1 = vol_var1;
        int v2 = vol_var2;
        
        /* Independent integer operations that can be scheduled together */
        int t1 = arr1[i] + v1;
        int t2 = arr2[i] - v2;
        int t3 = t1 * t2;
        int t4 = t2 / (v1 + 1);  /* Integer division - variable latency */
        
        /* Mix operation types to compete for functional units */
        float ft1 = (float)t3 + fdiv;
        float ft2 = (float)t4 * fsqrt;
        
        /* Store results with different latencies */
        arr1[i] = t3 + t4;
        arr2[i] = t1 - t2;
        farr1[i] = ft1 * ft2;
        farr2[i] = ft1 / (ft2 + 1.0f);  /* Another division */
        
        /* Update volatile to create cross-iteration dependency */
        vol_var1 = (vol_var1 + 1) & 0xFF;
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(float* restrict a, float* restrict b, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Many independent instructions that can be reordered */
        float r1 = a[i] * 1.1f;
        float r2 = b[i] * 2.2f;
        float r3 = a[i] + 3.3f;
        float r4 = b[i] - 4.4f;
        float r5 = r1 * r2;
        float r6 = r3 + r4;
        float r7 = r5 / r6;
        float r8 = r6 * r7;
        float r9 = r7 + r8;
        float r10 = r8 - r9;
        
        /* Store results */
        a[i] = r9 + r10;
        b[i] = r10 * 2.0f;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "mov $0, %%esi\n"
            "mov $0, %%edi\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Main computational kernel */
__attribute__((noinline))
void compute_kernel(float* fa, float* fb, float* fc, float* fd, float* fe,
                    int* ia, int* ib, int size) {
    int iter;
    
    for (iter = 0; iter < ITERATIONS/100; iter++) {
        /* Mix different computation patterns */
        high_pressure_loop(fa, fb, fc, fd, fe, size);
        mixed_dependency(ia, ib, fa, fb, size);
        independent_instructions(fc, fd, size);
        
        /* Prevent loop invariant code motion */
        vol_var1 = (iter & 0xFF);
        vol_var2 = ((iter >> 8) & 0xFF);
    }
}

int main() {
    int i;
    float *fa, *fb, *fc, *fd, *fe;
    int *ia, *ib;
    float checksum = 0.0f;
    
    /* Allocate and initialize arrays */
    fa = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fb = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fc = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fd = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fe = (float*)malloc(ARRAY_SIZE * sizeof(float));
    ia = (int*)malloc(ARRAY_SIZE * sizeof(int));
    ib = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        fc[i] = (float)rand() / RAND_MAX * 100.0f;
        fd[i] = (float)rand() / RAND_MAX * 100.0f;
        fe[i] = (float)rand() / RAND_MAX * 100.0f;
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
    }
    
    /* Perform computation */
    compute_kernel(fa, fb, fc, fd, fe, ia, ib, ARRAY_SIZE);
    
    /* Calculate checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += fa[i] + fb[i] + fc[i] + fd[i] + fe[i] + ia[i] + ib[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Free memory */
    free(fa); free(fb); free(fc); free(fd); free(fe);
    free(ia); free(ib);
    
    return 0;
}
