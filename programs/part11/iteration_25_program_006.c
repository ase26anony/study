/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation function with complex loop-carried dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* scalar) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;
    
    /* Reduction with flow dependency on sum (register edge) */
    for (i = 0; i < n; ++i) {
        sum += a[i] * (*scalar);
    }
    
    /* Array transformation with one-element lag (memory flow edges) */
    /* Use pragma to influence dependency analysis */
    #pragma GCC ivdep
    for (i = 1; i < n; ++i) {
        b[i] = a[i] + a[i-1];  /* Flow dependency on a[i-1] */
    }
    
    /* Complex nested loop with mixed dependencies */
    for (i = 0; i < n; ++i) {
        /* Anti-dependency: read before write */
        tmp_reg = c[i];
        
        /* Output dependency: write after write */
        c[i] = d[i];
        
        /* Flow dependency through register */
        d[i] = tmp_reg + sum * 0.1f;
        
        /* Conditional update creating condition edges */
        if (d[i] > 10.0f) {
            d[i] = conditional_update(d[i], 10.0f);
        }
        
        /* Nested loop with varying bounds */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Memory dependency with non-affine index */
            int idx = i + (j % 3);
            if (idx < n) {
                a[idx] = b[j] * c[i] + d[idx];
            }
        }
    }
    
    /* Additional loop with pointer arithmetic */
    float* ptr_a = a;
    float* ptr_b = b;
    for (i = 0; i < n - 1; ++i) {
        /* Multiple independent operations */
        float x = *ptr_a * *scalar;
        float y = *ptr_b + sum;
        *ptr_a = x + y;
        
        /* Complex addressing */
        ptr_a++;
        ptr_b++;
        
        /* Another conditional creating control dependency */
        if (i % 2 == 0) {
            *ptr_a = *ptr_a * 0.9f;
        } else {
            *ptr_a = *ptr_a * 1.1f;
        }
    }
}

/* Second computation with different pattern */
void compute2(int m, int n, float* restrict arr1, float* restrict arr2) {
    int i, j;
    float acc1 = 0.0f, acc2 = 0.0f;
    
    for (i = 0; i < m; ++i) {
        /* Two independent reduction chains */
        acc1 += arr1[i] * arr2[i];
        acc2 += arr1[i] - arr2[i];
        
        /* Cross-iteration dependency */
        if (i > 0) {
            arr1[i] = arr1[i] + arr1[i-1] * 0.5f;
        }
        
        /* Inner loop with carried dependency */
        for (j = 0; j < n && j < 10; ++j) {
            float temp = arr2[j];
            arr2[j] = arr1[i];
            arr1[i] = temp + acc1;
        }
    }
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    float scalar = 2.5f;
    float checksum = 0.0f;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size1 * sizeof(float));
    float* b = (float*)malloc(size1 * sizeof(float));
    float* c = (float*)malloc(size1 * sizeof(float));
    float* d = (float*)malloc(size1 * sizeof(float));
    float* arr1 = (float*)malloc(size2 * sizeof(float));
    float* arr2 = (float*)malloc(size2 * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize with random values */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f;
        c[i] = (float)rand() / RAND_MAX * 100.0f;
        d[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    for (int i = 0; i < size2; ++i) {
        arr1[i] = (float)rand() / RAND_MAX * 50.0f;
        arr2[i] = (float)rand() / RAND_MAX * 50.0f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d, &scalar);
    compute(size1 / 2, a, b, c, d, &scalar);
    compute(size1 / 4, a, b, c, d, &scalar);
    
    compute2(size2, 8, arr1, arr2);
    compute2(size2 / 2, 4, arr1, arr2);
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr1);
    free(arr2);
    
    return 0;
}
