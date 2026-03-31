/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper with always_inline to increase DDG scope */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float *restrict tmp) {
    float local = a * 2.5f;
    *tmp = local + b;
    return local - b;
}

/* Main computation with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d, float *restrict result) {
    int i, j;
    float acc = 0.0f;          /* Register flow dependency */
    float tmp_reg;             /* For anti/output dependencies */
    
    /* Outer loop with parameterized bound */
    for (i = 1; i < n; ++i) {
        /* 1. FLOW dependency (register): reduction pattern */
        acc += a[i] * 1.5f;
        
        /* 2. FLOW dependency (memory): array with one-element lag */
        b[i] = a[i] + a[i-1] * 0.7f;
        
        /* 3. ANTI and OUTPUT dependencies: swap-like operation */
        tmp_reg = c[i];
        c[i] = d[i] * 2.0f;
        d[i] = tmp_reg + 1.0f;
        
        /* Mixed index for memory dependency complexity */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < n) {
            /* 4. CONDITION dependency: guarded update */
            if (acc > 100.0f) {
                a[idx] = acc * 0.01f;
            }
        }
        
        /* 5. Multiple independent chains for parallelism potential */
        float x = a[i] * 3.0f;
        float y = b[i] + 4.0f;
        float z = x + y;
        
        /* Use result to prevent elimination */
        result[i] = z * 0.5f;
        
        /* 6. Nested loop with dependent bound - increases complexity */
        #pragma GCC ivdep  /* Assert no loop-carried memory deps for this inner loop */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Memory edge with non-affine index */
            int alt_idx = j % 2;
            d[j] += c[j + alt_idx] * 0.3f;
        }
        
        /* 7. Call to inline function for register/memory edges */
        float func_tmp;
        result[i] += process_element(a[i], b[i], &func_tmp);
    }
    
    /* Final reduction result */
    result[0] = acc;
}

/* Secondary computation with different pattern */
void compute2(int m, float *restrict x, float *restrict y) {
    float local_acc = x[0];
    
    #pragma GCC ivdep
    for (int i = 1; i < m; ++i) {
        /* Write-after-write (output) dependency */
        local_acc = x[i] * y[i];
        
        /* Read-after-write (anti) dependency */
        y[i-1] = local_acc + x[i];
        
        /* Flow dependency with distance > 0 */
        x[i] = y[i-1] * 0.8f + x[i-1];
    }
}

int main(void) {
    /* Initialize with different sizes to trigger various DDG constructions */
    float *a = malloc(SIZE * sizeof(float));
    float *b = malloc(SIZE * sizeof(float));
    float *c = malloc(SIZE * sizeof(float));
    float *d = malloc(SIZE * sizeof(float));
    float *result = malloc(SIZE * sizeof(float));
    float *x = malloc(SIZE * sizeof(float));
    float *y = malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize arrays with varied values */
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)(rand() % 100) * 0.1f;
        b[i] = (float)(rand() % 100) * 0.2f;
        c[i] = (float)(rand() % 100) * 0.3f;
        d[i] = (float)(rand() % 100) * 0.4f;
        x[i] = (float)(rand() % 100) * 0.5f;
        y[i] = (float)(rand() % 100) * 0.6f;
    }
    
    /* Call compute with different sizes to increase coverage chance */
    compute(SIZE, a, b, c, d, result);
    compute(SIZE/2, a, b, c, d, result);
    compute(SIZE*3/4, a, b, c, d, result);
    
    compute2(SIZE, x, y);
    compute2(SIZE/3, x, y);
    
    /* Checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += result[i] + a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a); free(b); free(c); free(d); free(result); free(x); free(y);
    return 0;
}
