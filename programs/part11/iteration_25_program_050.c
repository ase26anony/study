/* Target: GCC's ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* Alternative: -O3 -funroll-loops -fno-peel-loops */

#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation with various dependency patterns */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* scalar) {
    int i, j;
    float acc = 0.0f;          /* Register accumulator - flow dependency */
    float tmp1, tmp2;          /* Temporaries for anti/output dependencies */
    
    /* Reduction with flow dependency on acc */
    for (i = 0; i < n; ++i) {
        acc += a[i] * (*scalar);  /* REGISTER flow edge on acc */
    }
    
    /* Array transformation with 1-element lag - MEMORY flow edges */
    #pragma GCC ivdep  /* Assert no loop-carried memory deps (may trigger DDG verification) */
    for (i = 1; i < n; ++i) {
        b[i] = a[i] + a[i-1];  /* MEMORY flow edge on a[i-1] */
    }
    b[0] = a[0];
    
    /* Swap pattern with anti and output dependencies */
    for (i = 0; i < n; ++i) {
        tmp1 = c[i];           /* REGISTER anti-edge on tmp1 */
        c[i] = d[i];           /* MEMORY output edge on c[i] */
        d[i] = tmp1;           /* REGISTER flow edge on tmp1 */
    }
    
    /* Nested loop with non-linear indexing - complex memory dependencies */
    for (i = 0; i < n; ++i) {
        float local_acc = 0.0f;
        /* Inner loop bound depends on outer index */
        for (j = i; j < n; ++j) {
            /* Non-affine index: creates harder-to-analyze memory dependencies */
            int idx = i + (j % 3);
            if (idx < n) {
                local_acc += a[idx] * b[j];  /* MEMORY flow edges */
            }
        }
        
        /* Conditional update based on computed value - CONDITION edges */
        if (local_acc > 100.0f) {           /* CONDITION edge from local_acc */
            a[i] = conditional_update(a[i], 50.0f);  /* Mixed dependencies */
        }
        
        /* Multiple independent chains for potential parallelism */
        float x = a[i] * (*scalar);         /* REGISTER flow edge on scalar */
        float y = b[i] + local_acc;         /* REGISTER flow edge on local_acc */
        float z = x + y;                    /* REGISTER flow edges on x,y */
        
        /* Write result with output dependency */
        d[i] = z;                           /* MEMORY output edge on d[i] */
    }
    
    /* Final reduction with cross-iteration dependency */
    float final_sum = 0.0f;
    for (i = 1; i < n; ++i) {
        /* True loop-carried dependency */
        final_sum = final_sum * 0.9f + d[i] * 0.1f;  /* REGISTER flow edge on final_sum */
        
        /* Anti-dependency through array */
        float old_val = c[i-1];             /* MEMORY anti-edge on c[i-1] */
        c[i-1] = final_sum * old_val;       /* MEMORY output edge on c[i-1] */
    }
    
    /* Store final result to prevent elimination */
    *scalar = acc + final_sum;
}

int main(int argc, char** argv) {
    const int max_n = 1000;
    float *a, *b, *c, *d;
    float scalar = 1.5f;
    int i;
    
    /* Allocate with alignment hint */
    a = (float*)__builtin_assume_aligned(malloc(max_n * sizeof(float)), 16);
    b = (float*)__builtin_assume_aligned(malloc(max_n * sizeof(float)), 16);
    c = (float*)__builtin_assume_aligned(malloc(max_n * sizeof(float)), 16);
    d = (float*)__builtin_assume_aligned(malloc(max_n * sizeof(float)), 16);
    
    /* Initialize with pattern (not all zeros) */
    for (i = 0; i < max_n; ++i) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = (float)((i * 3) % 100) * 0.3f;
        d[i] = (float)((i * 7) % 100) * 0.4f;
    }
    
    /* Call compute with different sizes to increase coverage chances */
    compute(100, a, b, c, d, &scalar);
    compute(500, a, b, c, d, &scalar);
    compute(max_n, a, b, c, d, &scalar);
    
    /* Checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (i = 0; i < max_n; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    checksum += scalar;
    
    printf("Result checksum: %f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
