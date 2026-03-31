/* test_ddg_coverage.c
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 * Also try: gcc -O3 -funroll-loops -fno-peel-loops test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void compute_inner(int n, float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d, float *scalar) {
    float tmp_reg = 0.0f;
    int i, j;
    
    /* Outer loop with parameterized bound */
    for (i = 1; i < n; ++i) {
        /* 1. Reduction with flow dependency on register */
        *scalar += a[i] * 1.5f;
        
        /* 2. Memory flow dependency with one-element lag */
        b[i] = a[i] + a[i-1] * 2.0f;
        
        /* 3. Anti and output dependencies via swap pattern */
        float tmp = c[i];
        c[i] = d[i];
        d[i] = tmp;
        tmp_reg = tmp;  /* Register anti-dependency */
        
        /* 4. Nested loop with dependent bounds */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Complex addressing to create memory dependencies */
            int idx = j + (i % 2);
            if (idx < n) {
                /* Conditional creating control dependency */
                if (b[i] > 0.5f) {
                    a[idx] = b[i] * c[j] - d[idx];
                }
            }
        }
        
        /* 5. Mixed operations for varied edge types */
        float x = a[i] * *scalar;
        float y = b[i] + tmp_reg;
        d[i] = x + y;  /* Output dependency on d[i] */
        
        /* 6. Pointer arithmetic for non-affine access */
        float *ptr = &a[i + (i % 4)];
        *ptr = *ptr * 0.9f;  /* Memory output dependency */
    }
}

/* Main computation function */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float scalar_acc = 0.0f;
    
    /* Use pragma to influence dependency analysis */
    #pragma GCC ivdep
    compute_inner(n, a, b, c, d, &scalar_acc);
    
    /* Final reduction to prevent elimination */
    a[0] = scalar_acc;
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, float *c, float *d, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = (float)((i * 2) % 100) * 0.3f;
        d[i] = (float)((i * 3) % 100) * 0.4f;
    }
}

/* Checksum to prevent dead code elimination */
float checksum(float *a, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += a[i];
    }
    return sum;
}

int main() {
    float *a = malloc(SIZE * sizeof(float));
    float *b = malloc(SIZE * sizeof(float));
    float *c = malloc(SIZE * sizeof(float));
    float *d = malloc(SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test with different sizes to trigger various DDG constructions */
    int sizes[] = {100, 200, 500, SIZE};
    
    for (int test_case = 0; test_case < 4; ++test_case) {
        int n = sizes[test_case];
        
        init_arrays(a, b, c, d, n);
        
        /* Call compute multiple times */
        for (int iter = 0; iter < 3; ++iter) {
            compute(n, a, b, c, d);
        }
        
        /* Calculate and print checksum */
        float sum_a = checksum(a, n);
        float sum_b = checksum(b, n);
        printf("Test %d (n=%d): checksum a=%.2f, b=%.2f\n", 
               test_case, n, sum_a, sum_b);
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
