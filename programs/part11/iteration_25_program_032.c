/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg_coverage
 * Or with: gcc -O3 -funroll-loops -fno-peel-loops test_ddg_coverage.c -o test_ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float compute_element(float a, float b, float c, int cond) {
    /* Multiple arithmetic operations creating register dependencies */
    float t1 = a * 1.5f;
    float t2 = b * 2.0f;
    float t3 = t1 + t2;
    float t4 = c * 0.5f;
    
    /* Conditional operation creating condition dependencies */
    return (cond > 0) ? t3 + t4 : t3 - t4;
}

/* Main computation function with loop carrying various dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* restrict result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg = 0.0f;  /* Register for anti/output dependencies */
    
    /* Outer loop with parameterized bound */
    for (i = 1; i < n; ++i) {
        /* 1. FLOW DEPENDENCY (Register): Reduction pattern */
        sum += a[i] * 0.7f;
        
        /* 2. FLOW DEPENDENCY (Memory): Array copy with one-element shift */
        b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        
        /* 3. ANTI and OUTPUT DEPENDENCIES: Swap-like operation */
        tmp_reg = c[i];        /* ANTI: read before write in next stmt */
        c[i] = d[i];           /* OUTPUT: write to c[i] */
        d[i] = tmp_reg;        /* OUTPUT: write to d[i] */
        
        /* 4. COMPLEX MEMORY ACCESS: Non-affine index for memory edges */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < n) {
            a[idx] = a[idx] * 1.1f + b[i];
        }
        
        /* 5. CONDITION DEPENDENCY: If statement based on computed value */
        float cond_val = sum * 0.01f;
        if (cond_val > 1.0f) {
            /* Creates condition edge between cond_val computation and this block */
            b[i] = b[i] * 2.0f;
        }
        
        /* 6. NESTED LOOP: Inner loop with dependent bound */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (for testing) */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Independent chain of operations */
            float x = a[j] * 3.0f;
            float y = b[j] + 1.0f;
            d[j] = compute_element(x, y, c[j], j % 2);
        }
        
        /* 7. MIXED DATA TYPES: Integer and float operations */
        int int_val = (int)sum;
        if (int_val % 7 == 0) {
            a[i] = a[i] + int_val * 0.5f;
        }
    }
    
    /* Store final result to prevent dead code elimination */
    *result = sum + b[n-1] + c[n-1] + d[n-1];
}

/* Secondary computation with different pattern */
void compute2(int n, float* restrict arr1, float* restrict arr2, float* res) {
    float acc1 = 0.0f, acc2 = 0.0f;
    float tmp1, tmp2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple accumulators with cross-iteration dependencies */
        tmp1 = arr1[i];
        tmp2 = arr2[i];
        
        acc1 = acc1 * 0.9f + tmp1;
        acc2 = acc2 * 0.8f + tmp2;
        
        /* Write-after-read (anti-dependency) */
        arr1[i] = acc1;
        arr2[i] = acc2;
        
        /* Write-after-write (output dependency) on shared temporary */
        float shared_tmp = acc1 + acc2;
        if (i % 2 == 0) {
            shared_tmp = shared_tmp * 1.5f;  /* Reuses same variable name */
        }
        
        /* Pointer arithmetic creating memory dependencies */
        float* ptr = arr1 + i;
        *ptr = *ptr + shared_tmp;
    }
    
    *res = acc1 + acc2;
}

int main() {
    /* Initialize with different sizes to test various compilation paths */
    int sizes[] = {100, 500, 1000};
    float total_result = 0.0f;
    
    srand(time(NULL));
    
    for (int s = 0; s < 3; ++s) {
        int n = sizes[s];
        
        /* Allocate arrays with restrict qualifiers */
        float* a = (float*)malloc(n * sizeof(float));
        float* b = (float*)malloc(n * sizeof(float));
        float* c = (float*)malloc(n * sizeof(float));
        float* d = (float*)malloc(n * sizeof(float));
        float result1, result2;
        
        /* Initialize with pattern (not completely random for reproducibility) */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 1) % 100) * 0.2f;
            c[i] = (float)((i * 2) % 100) * 0.3f;
            d[i] = (float)((i * 3) % 100) * 0.4f;
        }
        
        /* Call compute functions multiple times */
        for (int iter = 0; iter < 3; ++iter) {
            compute(n, a, b, c, d, &result1);
            compute2(n, a, b, &result2);
            total_result += result1 + result2;
            
            /* Modify inputs slightly for next iteration */
            for (int i = 0; i < n; i += 10) {
                a[i] += 0.5f;
            }
        }
        
        free(a);
        free(b);
        free(c);
        free(d);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %f\n", total_result);
    
    return 0;
}
