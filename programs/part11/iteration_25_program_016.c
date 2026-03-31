/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg_coverage
 * Or with: gcc -O3 -funroll-loops -fno-peel-loops test_ddg_coverage.c -o test_ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024

/* Helper function with always_inline to force inlining */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float* restrict tmp_reg) {
    /* Creates register dependencies */
    float local = a * 1.5f;
    *tmp_reg = local + b;
    return *tmp_reg;
}

/* Main computation function with loop nest */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg = 0.0f;
    
    /* Reduction with flow dependency on sum (register flow edge) */
    for (i = 0; i < n; ++i) {
        sum += a[i] * 2.0f;  /* Flow/true dependency on sum */
    }
    
    /* Array transformation with one-element lag (memory flow edges) */
    #pragma GCC ivdep  /* Assert no loop-carried memory dependencies */
    for (i = 1; i < n; ++i) {
        b[i] = a[i] + a[i-1];  /* Flow dependency on a[i-1] */
    }
    
    /* Nested loop with dependent bounds */
    for (i = 0; i < n; ++i) {
        /* Inner loop bound depends on outer index */
        for (j = i; j < n && j < i + 10; ++j) {
            /* Mixed address calculation */
            int idx = j + (i % 2);
            if (idx < n) {
                c[idx] = b[j] * 3.0f;  /* Memory dependency */
            }
        }
    }
    
    /* Swap operations creating anti and output dependencies */
    for (i = 0; i < n - 1; i += 2) {
        float tmp = d[i];           /* Anti-dependency on d[i] */
        d[i] = d[i + 1];            /* Output dependency on d[i] */
        d[i + 1] = tmp;             /* Output dependency on d[i+1] */
        
        /* Conditional update creating condition edges */
        if (tmp > 0.5f) {           /* Condition depends on computed value */
            d[i] *= 0.8f;
        }
    }
    
    /* Complex loop with mixed operations */
    for (i = 0; i < n; ++i) {
        /* Independent arithmetic chains */
        float x = a[i] * 1.2f;
        float y = b[i] + 2.5f;
        float z = x + y;
        
        /* Process with helper (register edges) */
        float processed = process_element(z, c[i], &tmp_reg);
        
        /* Conditional store with memory dependency */
        if (processed > 0.0f) {     /* Condition edge */
            d[i] = processed;        /* Memory dependency */
        }
        
        /* Additional reduction with anti-dependency */
        sum = sum - tmp_reg + processed;  /* Multiple register dependencies */
    }
    
    *result = sum;
}

/* Variant with different access pattern */
void compute_variant(int n, float* restrict arr1, float* restrict arr2, 
                     float* restrict arr3, float* result) {
    float acc1 = 0.0f, acc2 = 0.0f;
    int i;
    
    /* Loop with pointer arithmetic */
    float* p1 = arr1;
    float* p2 = arr2;
    float* p3 = arr3;
    
    for (i = 0; i < n; ++i) {
        /* Chain of dependencies */
        float val1 = *p1++;
        float val2 = *p2++ + acc1;
        acc1 = val1 * val2;
        
        /* Independent chain */
        float val3 = *p3++ * 0.7f;
        acc2 = acc2 + val3;
        
        /* Cross-chain dependency */
        if (acc1 > acc2) {
            *(--p1) = acc2;  /* Anti-dependency through pointer */
        }
    }
    
    *result = acc1 + acc2;
}

int main() {
    /* Initialize with medium-sized arrays */
    float* a = (float*)malloc(SIZE * sizeof(float));
    float* b = (float*)malloc(SIZE * sizeof(float));
    float* c = (float*)malloc(SIZE * sizeof(float));
    float* d = (float*)malloc(SIZE * sizeof(float));
    float result1, result2, result3;
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
        c[i] = (float)rand() / RAND_MAX;
        d[i] = (float)rand() / RAND_MAX;
    }
    
    /* Call compute multiple times with different sizes
     * to increase chance of DDG construction */
    compute(SIZE, a, b, c, d, &result1);
    compute(SIZE / 2, a, b, c, d, &result2);
    compute_variant(SIZE, a, b, c, &result3);
    
    /* Additional calls with varying sizes */
    for (int iter = 0; iter < 3; ++iter) {
        int size = 100 + iter * 200;
        float temp_result;
        compute(size, a, b, c, d, &temp_result);
        result1 += temp_result;
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Results: %f, %f, %f\n", result1, result2, result3);
    printf("Checksum: %f\n", result1 + result2 + result3);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
