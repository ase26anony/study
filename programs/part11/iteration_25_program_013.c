/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float* restrict tmp) {
    float local = a * 1.5f;
    *tmp = local + b;
    return local - b;
}

/* Main computation with mixed dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;  /* Register for anti/output dependencies */
    
    /* Reduction with flow dependency on sum (register edge) */
    for (i = 0; i < n; ++i) {
        sum += a[i] * 2.0f;
    }
    
    /* Nested loop with cross-iteration memory dependencies */
    for (i = 1; i < n - 1; ++i) {
        /* Flow dependency on a[i-1] (memory edge with distance 1) */
        b[i] = a[i] + a[i - 1];
        
        /* Anti-dependency: read a[i] before writing c[i] */
        float read_val = a[i];
        
        /* Output dependency on c[i] */
        c[i] = read_val * 3.0f;
        
        /* Complex addressing for non-affine memory access */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < n) {
            /* Memory flow edge with non-constant distance */
            d[idx] = c[i] * 0.5f;
        }
        
        /* Register anti/output dependencies via swap pattern */
        tmp_reg = b[i];
        b[i] = c[i];
        c[i] = tmp_reg;
        
        /* Conditional creating control/condition edges */
        if (sum > 100.0f) {
            a[i] *= 0.9f;  /* Output dependency on a[i] */
        }
        
        /* Inner loop with varying bounds for scheduling complexity */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Independent operations for potential parallelism */
            float x = a[j] * b[j];
            float y = c[j] + d[j];
            b[j] = x + y;  /* Memory output dependency */
        }
        
        /* Call to inline function for register/memory mix */
        tmp_reg = process_element(a[i], b[i], &c[i]);
        sum += tmp_reg;  /* Flow dependency on sum */
    }
    
    /* Final reduction with loop-carried dependency */
    float final_sum = 0.0f;
    for (i = 0; i < n; ++i) {
        final_sum += b[i] + c[i];
        /* Anti-dependency chain */
        float old = d[i];
        d[i] = final_sum * 0.1f;
        final_sum += old;
    }
    
    *result = sum + final_sum;
}

/* Secondary computation with different pattern */
void compute2(int m, float* restrict x, float* restrict y, float* res) {
    float acc = 0.0f;
    float tmp1, tmp2;
    
    for (int i = 0; i < m; ++i) {
        /* Multiple independent chains for modulo scheduling */
        tmp1 = x[i] * 1.1f;
        tmp2 = y[i] * 0.9f;
        
        /* Cross-iteration flow dependency */
        acc = acc + tmp1 - tmp2;
        
        /* Memory anti-dependency */
        float read_x = x[i];
        x[i] = tmp1 * tmp2;
        
        /* Condition edge */
        if (acc > tmp1) {
            y[i] = read_x * acc;
        } else {
            y[i] = tmp2 * acc;
        }
        
        /* Output dependency on array */
        x[i + (i % 2)] = y[i] * 0.5f;
    }
    
    *res = acc;
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    float *a, *b, *c, *d;
    float *x, *y;
    float result1, result2, final_result;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(size1 * sizeof(float));
    b = (float*)malloc(size1 * sizeof(float));
    c = (float*)malloc(size1 * sizeof(float));
    d = (float*)malloc(size1 * sizeof(float));
    
    x = (float*)malloc(size2 * sizeof(float));
    y = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with pattern (not all zeros) */
    for (int i = 0; i < size1; ++i) {
        a[i] = (i % 10) * 1.0f;
        b[i] = (i % 7) * 0.5f;
        c[i] = (i % 5) * 1.5f;
        d[i] = (i % 3) * 2.0f;
    }
    
    for (int i = 0; i < size2; ++i) {
        x[i] = (i % 8) * 0.8f;
        y[i] = (i % 6) * 1.2f;
    }
    
    /* Call compute functions multiple times with different sizes
     * to increase chance of DDG construction */
    compute(size1, a, b, c, d, &result1);
    compute(size1 - 100, a + 100, b + 100, c + 100, d + 100, &result2);
    compute2(size2, x, y, &final_result);
    
    /* Use results to prevent dead code elimination */
    final_result = result1 + result2 + final_result;
    printf("Result checksum: %f\n", final_result);
    
    /* Additional variant with different loop structure */
    {
        float small_a[50], small_b[50], small_c[50], small_res;
        for (int i = 0; i < 50; ++i) {
            small_a[i] = i * 0.1f;
            small_b[i] = i * 0.2f;
            small_c[i] = i * 0.3f;
        }
        compute(50, small_a, small_b, small_c, small_a, &small_res);
        printf("Small result: %f\n", small_res);
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
