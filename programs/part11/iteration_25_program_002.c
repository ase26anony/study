/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_iteration(float *restrict a, float *restrict b, float *restrict c,
                  float *restrict x, float *restrict y, int i, 
                  float *sum, float *tmp_reg) {
    /* Register flow dependency: sum accumulates across iterations */
    *sum += a[i] * 3.14f;
    
    /* Memory flow dependency with 1-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1] * 2.0f;
    } else {
        b[i] = a[i];
    }
    
    /* Anti-dependency and output dependency via swap */
    *tmp_reg = x[i];      /* Read x[i] */
    x[i] = y[i];          /* Write x[i] - output dependency with previous read */
    y[i] = *tmp_reg;      /* Write y[i] - anti-dependency with previous write */
    
    /* Condition dependency */
    float cond_val = c[i] * *sum;
    if (cond_val > 100.0f) {
        c[i] = cond_val * 0.5f;  /* Memory flow dependency on c[i] */
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict x, float *restrict y) {
    float sum = 0.0f;
    float tmp_reg;
    
    /* Outer loop with parameterized bound */
    for (int i = 0; i < n; ++i) {
        /* Mixed address calculation to create memory dependencies */
        int idx = i + (i % 3);  /* Non-affine index */
        if (idx < n) {
            /* Memory dependency with non-linear index */
            a[idx] = a[idx] * 1.1f + sum;
        }
        
        /* Process main iteration */
        process_iteration(a, b, c, x, y, i, &sum, &tmp_reg);
        
        /* Inner loop with dependent bound - creates complex dependencies */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Cross-iteration memory dependency */
            b[j] += a[i] * 0.1f;
            
            /* Register dependency chain */
            tmp_reg = tmp_reg * 0.9f + c[j];
        }
    }
}

/* Alternative version with pragma to influence scheduling */
void __attribute__((noinline))
compute_ivdep(int n, float *restrict a, float *restrict b,
              float *restrict c, float *restrict x, float *restrict y) {
    float sum = 0.0f;
    float tmp_reg;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Independent operations that can be parallelized */
        float t1 = a[i] * 2.0f;
        float t2 = b[i] + 1.0f;
        float t3 = c[i] * 0.5f;
        
        /* Create dependencies between these operations */
        a[i] = t1 + sum;           /* Flow dependency on sum */
        b[i] = t2 + t3;            /* Anti-dependency on b[i] */
        c[i] = a[i] * t2;          /* Flow dependency on a[i], anti on c[i] */
        
        /* Register dependency chain */
        sum = sum + t1 + t2 + t3;
        
        /* Output dependency via swap pattern */
        tmp_reg = x[i];
        x[i] = y[i];
        y[i] = tmp_reg;
    }
}

int main(void) {
    const int sizes[] = {100, 200, 300};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    float checksum = 0.0f;
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate and initialize arrays */
        float *a = (float*)malloc(n * sizeof(float));
        float *b = (float*)malloc(n * sizeof(float));
        float *c = (float*)malloc(n * sizeof(float));
        float *x = (float*)malloc(n * sizeof(float));
        float *y = (float*)malloc(n * sizeof(float));
        
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(i % 100);
            b[i] = (float)((i + 1) % 100);
            c[i] = (float)((i * 2) % 100);
            x[i] = (float)(i);
            y[i] = (float)(n - i);
        }
        
        /* Call both versions to increase coverage chances */
        compute(n, a, b, c, x, y);
        compute_ivdep(n, a, b, c, x, y);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i] + x[i] + y[i];
        }
        
        free(a); free(b); free(c); free(x); free(y);
    }
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
