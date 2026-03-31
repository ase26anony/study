/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation function with loop-carried dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* result) {
    int i, j;
    float sum = 0.0f;
    float tmp1, tmp2;
    
    /* Reduction with flow dependency on sum (register edge) */
    for (i = 0; i < n; ++i) {
        sum += a[i] * 1.5f;
    }
    
    /* Array transformation with one-element lag (memory flow edges) */
    #pragma GCC ivdep  /* Assert no loop-carried dependencies (may trigger DDG verification) */
    for (i = 1; i < n; ++i) {
        b[i] = a[i] + a[i-1];  /* Flow dependency on a[i-1] */
    }
    b[0] = a[0];
    
    /* Nested loop with varying bounds for scheduling complexity */
    for (i = 0; i < n/2; ++i) {
        float local_acc = 0.0f;
        /* Inner loop with dependency on outer index */
        for (j = i; j < n && j < i + 10; ++j) {
            /* Mixed memory accesses with non-affine index */
            int idx = j + (i % 3);
            if (idx < n) {
                local_acc += c[idx] * d[j];
            }
        }
        /* Conditional update creating condition edges */
        if (local_acc > 100.0f) {
            c[i] = conditional_update(c[i], 50.0f);
        }
    }
    
    /* Swap operations creating anti and output dependencies */
    for (i = 0; i < n - 1; i += 2) {
        /* Register anti-dependency: read then write */
        tmp1 = c[i];
        tmp2 = c[i + 1];
        
        /* Output dependency: write after write */
        c[i] = tmp2 * 0.8f;
        c[i + 1] = tmp1 * 1.2f;
        
        /* Memory anti-dependency with pointer aliasing prevention (restrict helps) */
        d[i] = c[i] + b[i];
    }
    
    /* Final reduction with multiple dependency types */
    float final_sum = 0.0f;
    for (i = 0; i < n; ++i) {
        /* Independent arithmetic chains for potential parallelism */
        float x = a[i] * 2.0f;
        float y = b[i] + 1.0f;
        float z = c[i] * 0.5f;
        
        /* Flow dependency on final_sum */
        final_sum += x + y + z;
        
        /* Condition edge */
        if (final_sum > 1000.0f) {
            final_sum *= 0.99f;  /* Slight reduction to prevent overflow */
        }
    }
    
    *result = sum + final_sum;
}

/* Secondary computation with different patterns */
void compute2(int n, float* restrict arr1, float* restrict arr2, float* res) {
    int i;
    float acc = 0.0f;
    
    /* Loop with pointer arithmetic for memory dependency edges */
    float* p1 = arr1;
    float* p2 = arr2;
    
    for (i = 0; i < n; ++i) {
        /* Memory flow dependency through pointers */
        float val = *p1++ + *p2++;
        
        /* Register output dependency */
        acc = acc + val;  /* Actually flow dependency, but scheduler sees write-after-write */
        
        /* Anti-dependency through array access */
        arr1[i] = acc * 0.1f;
        
        /* Complex addressing for memory edge creation */
        int idx = (i * 7) % n;
        if (idx > 0) {
            arr2[idx] = arr1[idx - 1] + 1.0f;
        }
    }
    
    *res = acc;
}

int main() {
    int i;
    float result1, result2;
    
    /* Allocate and initialize arrays with non-constant values */
    float* a = (float*)malloc(SIZE * sizeof(float));
    float* b = (float*)malloc(SIZE * sizeof(float));
    float* c = (float*)malloc(SIZE * sizeof(float));
    float* d = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize with semi-random values to prevent compile-time optimization */
    for (i = 0; i < SIZE; ++i) {
        a[i] = (float)(rand() % 100) / 10.0f;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call compute with different sizes to trigger various DDG constructions */
    compute(SIZE, a, b, c, d, &result1);
    compute(SIZE/2, a, b, c, d, &result2);
    compute2(SIZE, a, b, &result1);
    compute2(SIZE/4, c, d, &result2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %f\n", result1, result2);
    printf("Checksum: %f\n", result1 + result2);
    
    /* Additional calls with varying parameters */
    for (i = 100; i <= 500; i += 100) {
        float tmp;
        compute(i, a, b, c, d, &tmp);
        result1 += tmp;
    }
    
    printf("Final checksum: %f\n", result1);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
