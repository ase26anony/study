/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *a, float *b, float *c, float *d, int n) {
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = c[i] + d[i];
        float t3 = a[i] / (b[i] + 0.001f);  /* Division for latency */
        float t4 = t1 * t2;
        
        /* Group 2: More independent operations */
        float t5 = sqrtf(fabsf(t3));
        float t6 = t4 * vol_float1;  /* Volatile dependency */
        float t7 = t5 + vol_float2;
        
        /* Group 3: Integer operations mixed in */
        int it1 = (int)(t1 * 1000);
        int it2 = (int)(t2 * 1000);
        int it3 = it1 + it2 + vol_var1;
        int it4 = it1 * it2 - vol_var2;
        
        /* Store results creating register pressure */
        a[i] = t4 + t6;
        b[i] = t5 * t7;
        c[i] = (float)it3 * 0.001f;
        d[i] = (float)it4 * 0.001f;
        
        /* Inline asm to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "mov $0, %%esi\n"
            "mov $0, %%edi\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with artificial dependencies and delays */
__attribute__((noinline))
void mixed_dependency(int *arr1, int *arr2, float *farr, int n) {
    /* Create long dependency chains with volatile accesses */
    int acc1 = vol_var1;
    int acc2 = vol_var2;
    float facc1 = vol_float1;
    float facc2 = vol_float2;
    
    for (int i = 0; i < n; i++) {
        /* Long latency operation (division) creates delay */
        facc1 = facc1 / (farr[i] + 0.001f);
        
        /* Volatile access creates scheduling barrier */
        acc1 = acc1 * vol_var1;
        
        /* Independent parallel chains */
        int t1 = arr1[i] * 3;
        int t2 = arr2[i] * 7;
        float t3 = sinf(farr[i]);
        float t4 = cosf(farr[i]);
        
        /* More volatile dependencies */
        acc2 = acc2 + vol_var2;
        facc2 = facc2 * vol_float2;
        
        /* Resource conflicts: mix operations */
        arr1[i] = t1 + acc1;
        arr2[i] = t2 - acc2;
        farr[i] = t3 * facc1 + t4 * facc2;
        
        /* Memory barrier to force ordering */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(float *a, float *b, float *c, int n) {
    /* Large basic block with independent groups */
    for (int i = 0; i < n; i += 4) {
        /* Group A: Completely independent */
        float a0 = a[i] * 1.1f;
        float a1 = a[i+1] * 2.2f;
        float a2 = a[i+2] * 3.3f;
        float a3 = a[i+3] * 4.4f;
        
        /* Group B: Independent from A */
        float b0 = b[i] + 10.0f;
        float b1 = b[i+1] + 20.0f;
        float b2 = b[i+2] + 30.0f;
        float b3 = b[i+3] + 40.0f;
        
        /* Group C: Mix operations */
        float c0 = sqrtf(a0 + b0);
        float c1 = sqrtf(a1 + b1);
        float c2 = sqrtf(a2 + b2);
        float c3 = sqrtf(a3 + b3);
        
        /* Group D: More mixing */
        float d0 = c0 * c[i];
        float d1 = c1 * c[i+1];
        float d2 = c2 * c[i+2];
        float d3 = c3 * c[i+3];
        
        /* Stores - creates register pressure */
        a[i] = d0;
        a[i+1] = d1;
        a[i+2] = d2;
        a[i+3] = d3;
        b[i] = c0;
        b[i+1] = c1;
        b[i+2] = c2;
        b[i+3] = c3;
    }
}

/* Main computation that will be scheduled */
__attribute__((noinline))
float compute_heavy(float *data, int size, int iterations) {
    float *buf1 = (float*)malloc(size * sizeof(float));
    float *buf2 = (float*)malloc(size * sizeof(float));
    float *buf3 = (float*)malloc(size * sizeof(float));
    float *buf4 = (float*)malloc(size * sizeof(float));
    int *ibuf1 = (int*)malloc(size * sizeof(int));
    int *ibuf2 = (int*)malloc(size * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        buf1[i] = (float)i * 0.1f;
        buf2[i] = (float)(size - i) * 0.2f;
        buf3[i] = sinf((float)i * 0.01f);
        buf4[i] = cosf((float)i * 0.01f);
        ibuf1[i] = i * 3;
        ibuf2[i] = i * 7;
    }
    
    float result = 0.0f;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix different scheduling patterns */
        high_pressure_loop(buf1, buf2, buf3, buf4, size/2);
        mixed_dependency(ibuf1, ibuf2, buf1, size/4);
        independent_instructions(buf2, buf3, buf4, size/2);
        
        /* Accumulate result to prevent dead code elimination */
        for (int i = 0; i < 16; i++) {
            result += buf1[i] + buf2[i] + buf3[i] + buf4[i] 
                    + ibuf1[i] + ibuf2[i];
        }
        
        /* Modify volatile to change dependencies */
        if (iter % 100 == 0) {
            vol_var1 = (vol_var1 * 3) % 17;
            vol_var2 = (vol_var2 * 5) % 23;
            vol_float1 = sinf(vol_float1 * 1.1f);
            vol_float2 = cosf(vol_float2 * 0.9f);
        }
    }
    
    free(buf1);
    free(buf2);
    free(buf3);
    free(buf4);
    free(ibuf1);
    free(ibuf2);
    
    return result;
}

int main() {
    clock_t start = clock();
    
    /* Allocate working data */
    float *data = (float*)malloc(SIZE * sizeof(float));
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i / SIZE;
    }
    
    /* Perform computation - this is where scheduling happens during compilation */
    float result = compute_heavy(data, SIZE, ITERATIONS);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %f\n", result);
    printf("Time: %f seconds\n", elapsed);
    printf("Performance: %f iterations/sec\n", ITERATIONS / elapsed);
    
    free(data);
    
    /* Use result to prevent optimization */
    if (result > 1000000.0f) {
        printf("Unexpectedly large result\n");
    }
    
    return 0;
}
