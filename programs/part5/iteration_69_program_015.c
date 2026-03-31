/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure and independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    int i;
    /* Many independent floating point operations to create register pressure */
    for (i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = c[i] / (d[i] + 0.001f);  /* Division has higher latency */
        float t3 = t1 + t2;
        float t4 = a[i] - b[i];
        float t5 = c[i] * d[i];
        float t6 = t4 * t5;
        
        /* Group 2: More independent operations */
        float t7 = sqrtf(fabsf(t3) + 1.0f);  /* High latency sqrt */
        float t8 = t6 * 0.5f;
        float t9 = t7 + t8;
        float t10 = sinf(t9 * 0.01f);       /* High latency sin */
        
        /* Group 3: Integer operations mixed with FP */
        int it1 = (int)t3;
        int it2 = (int)t6;
        int it3 = it1 * it2;
        int it4 = it3 + (int)t10;
        
        /* Store results creating dependencies */
        a[i] = t3 + t10;
        b[i] = t6 * t10;
        c[i] = (float)it4;
        
        /* Inline asm to clobber registers and force spills */
        asm volatile ("" : : : 
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr, int n) {
    int i;
    volatile int dep = vol_a;  /* Volatile dependency */
    
    for (i = 0; i < n; i++) {
        /* Long latency chain */
        int x = arr1[i];
        x = x * dep;           /* Depends on volatile */
        x = x + vol_b;         /* Another volatile dep */
        x = x / (vol_c + 1);   /* Integer division */
        
        /* Independent parallel chain */
        int y = arr2[i];
        y = y * y;
        y = y - arr1[i];
        
        /* FP operation competing for resources */
        float f = farr[i];
        f = f * 3.14159f;
        f = f / (vol_f1 + 0.1f);  /* FP division - high latency */
        
        /* Resource conflict: use same functional units */
        if (i % 2 == 0) {
            f = sqrtf(f);          /* Compete for FPU */
            x = x << 2;            /* Compete for ALU */
        } else {
            f = f * f;             /* Different FP op */
            y = y >> 1;            /* Different shift */
        }
        
        /* Memory operations with potential aliasing */
        arr1[i] = x + y;
        arr2[i] = y - (int)f;
        farr[i] = f * 0.5f;
        
        /* Artificial delay through inline asm */
        asm volatile ("mfence" ::: "memory");
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(float *restrict out, 
                              const float *restrict in1,
                              const float *restrict in2,
                              const float *restrict in3,
                              int n) {
    int i;
    
    /* Unrolled loop to create many independent instructions */
    for (i = 0; i < n; i += 4) {
        /* Block of independent FP operations - scheduler has many choices */
        float r0 = in1[i] * in2[i];
        float r1 = in1[i+1] + in2[i+1];
        float r2 = in1[i+2] - in2[i+2];
        float r3 = in1[i+3] / (in2[i+3] + 0.001f);
        
        float s0 = in3[i] * r0;
        float s1 = in3[i+1] * r1;
        float s2 = in3[i+2] * r2;
        float s3 = in3[i+3] * r3;
        
        float t0 = r0 + s0;
        float t1 = r1 + s1;
        float t2 = r2 + s2;
        float t3 = r3 + s3;
        
        /* More independent operations */
        float u0 = t0 * 0.1f;
        float u1 = t1 * 0.2f;
        float u2 = t2 * 0.3f;
        float u3 = t3 * 0.4f;
        
        float v0 = u0 + vol_f1;  /* Volatile dependency creates delay */
        float v1 = u1 + vol_f2;
        float v2 = u2 + vol_f3;
        float v3 = u3 + vol_f1;
        
        /* Store results */
        out[i] = v0;
        out[i+1] = v1;
        out[i+2] = v2;
        out[i+3] = v3;
    }
}

/* Main computational kernel that gets scheduled */
__attribute__((noinline))
float compute_kernel(int seed) {
    /* Local arrays to create register pressure */
    float array1[ARRAY_SIZE];
    float array2[ARRAY_SIZE];
    float array3[ARRAY_SIZE];
    float array4[ARRAY_SIZE];
    int iarray1[ARRAY_SIZE];
    int iarray2[ARRAY_SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float val = (float)((i + seed) % 100) * 0.01f;
        array1[i] = val;
        array2[i] = val * 2.0f;
        array3[i] = val * 3.0f;
        array4[i] = 0.0f;
        iarray1[i] = (i + seed) % 256;
        iarray2[i] = (i * seed) % 256;
    }
    
    /* Call functions that create different scheduling scenarios */
    high_pressure_loop(array1, array2, array3, array4, ARRAY_SIZE/2);
    mixed_dependency(iarray1, iarray2, array4, ARRAY_SIZE/4);
    independent_instructions(array1, array2, array3, array4, ARRAY_SIZE/2);
    
    /* Compute checksum to prevent dead code elimination */
    float sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += array1[i] + array2[i] + array3[i] + array4[i] 
               + (float)iarray1[i] + (float)iarray2[i];
    }
    
    return sum;
}

int main() {
    clock_t start = clock();
    float total = 0.0f;
    
    /* Perform many iterations to make this hot code */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary seed slightly each iteration to avoid constant propagation */
        int seed = (iter * 1103515245 + 12345) & 0x7fffffff;
        total += compute_kernel(seed);
        
        /* Progress indicator */
        if (iter % 10000 == 0) {
            printf("Iteration %d, running total: %f\n", iter, total);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final total: %f\n", total);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    printf("Performance: %.2f iterations/sec\n", ITERATIONS / elapsed);
    
    return 0;
}
