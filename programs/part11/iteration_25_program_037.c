/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -o ddg_test ddg_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_element(float *restrict a, float *restrict b, float *restrict c,
                     float *restrict d, int i, int prev, float *sum, 
                     float *tmp_reg, int *cond_counter) {
    /* Multiple independent arithmetic chains for potential parallelism */
    float x = a[i] * 1.5f;          /* Memory read + arithmetic */
    float y = b[i] + 2.0f;          /* Independent memory read + arithmetic */
    
    /* Flow dependency on prev (loop-carried through function arg) */
    float z = x + y + d[prev];      /* Uses value from previous iteration */
    
    /* Register flow dependency on sum */
    *sum += z;                      /* Reduction - creates register flow edge */
    
    /* Memory flow dependency with one-element lag */
    c[i] = a[i] + a[prev];          /* Uses a[prev] from previous iteration */
    
    /* Anti and output dependencies through swap operation */
    *tmp_reg = d[i];                /* Read d[i] */
    d[i] = d[prev] * 0.5f;          /* Write d[i] - output dep with above */
    d[prev] = *tmp_reg;             /* Write d[prev] - anti dep with read of d[prev] above */
    
    /* Condition edge creation */
    if (z > 100.0f) {               /* Condition depends on z computed earlier */
        (*cond_counter)++;
        b[i] *= 1.1f;               /* Memory write guarded by condition */
    }
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;
    int cond_counter = 0;
    
    /* Ensure n > 1 for dependencies */
    if (n <= 1) return;
    
    /* Initialize first element dependencies */
    c[0] = a[0];
    d[0] = b[0];
    
    /* Main loop with cross-iteration dependencies */
    #pragma GCC ivdep  /* Assert no loop-carried memory deps (will be analyzed) */
    for (i = 1; i < n; ++i) {
        int prev = i - 1;
        
        /* Process element with multiple dependency types */
        process_element(a, b, c, d, i, prev, &sum, &tmp_reg, &cond_counter);
        
        /* Additional independent operations to increase instruction mix */
        float t1 = a[i] * b[i];     /* Memory-memory operation */
        float t2 = c[prev] + d[i];  /* Mixed memory access */
        a[i] = t1 + t2;             /* Memory write with flow deps on t1, t2 */
        
        /* Nested loop with varying bound to prevent unrolling */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Non-affine array access to create complex memory dependencies */
            int idx = i + (j % 2);  /* Non-simple affine function */
            if (idx < n) {
                b[idx] += a[i] * 0.3f;  /* Flow dep on a[i], output dep on b[idx] */
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    a[0] = sum / n;
    b[0] = cond_counter;
}

/* Secondary function with different loop structure */
void compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    int i, j;
    float acc = 0.0f;
    
    /* Double nested loop with dependencies */
    for (i = 0; i < m; ++i) {
        float row_sum = 0.0f;
        int prev_j = 0;
        
        for (j = 0; j < n; ++j) {
            /* Flow dependency through row_sum */
            row_sum += arr1[i * n + j];
            
            /* Anti-dependency: read then write */
            float old_val = arr2[i * n + j];
            arr2[i * n + j] = row_sum + (j > 0 ? arr2[i * n + j - 1] : 0);
            
            /* Output dependency on temporary */
            float tmp = old_val;
            arr1[i * n + j] = tmp * 0.5f;
            tmp = arr2[i * n + j];  /* Reuse tmp - output dependency */
            
            /* Condition edge */
            if (row_sum > acc) {
                acc = row_sum;
            }
            
            prev_j = j;
        }
    }
    
    /* Final reduction */
    arr2[0] = acc;
}

int main(int argc, char *argv[]) {
    int size1 = 1000;
    int size2 = 500;
    
    /* Allocate arrays with restrict to help alias analysis */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *c = (float*)malloc(size1 * sizeof(float));
    float *d = (float*)malloc(size1 * sizeof(float));
    
    float *arr1 = (float*)malloc(size1 * size2 * sizeof(float));
    float *arr2 = (float*)malloc(size1 * size2 * sizeof(float));
    
    /* Initialize with pattern (not random for reproducibility) */
    for (int i = 0; i < size1; ++i) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = 0.0f;
        d[i] = i * 0.3f;
    }
    
    for (int i = 0; i < size1 * size2; ++i) {
        arr1[i] = (i % 100) * 0.05f;
        arr2[i] = (i % 50) * 0.1f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d);
    compute(size1 / 2, a + 100, b + 100, c + 100, d + 100);
    
    compute2(50, 20, arr1, arr2);
    compute2(30, 40, arr1 + 1000, arr2 + 1000);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < size1 * size2; ++i) {
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
