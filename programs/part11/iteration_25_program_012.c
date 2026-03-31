/* Test program to cover DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_chunk(float *restrict a, float *restrict b, float *restrict c, 
              float *restrict d, int start, int end, float *sum_acc) {
    float local_sum = *sum_acc;
    float tmp_swap;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        local_sum += a[i] * 1.5f;
        
        /* 2. Flow dependency (memory) - array copy with one-element lag */
        if (i > 0) {
            b[i] = a[i] + a[i-1];  /* RAW on a[i-1] from previous iteration */
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies - swap operation */
        tmp_swap = c[i];
        c[i] = d[i];      /* WAW on c[i], WAR on d[i] */
        d[i] = tmp_swap;  /* WAW on d[i] */
        
        /* 4. Condition dependency - based on computed value */
        if (local_sum > 1000.0f) {
            /* Control dependency edge */
            b[i] *= 0.5f;
            local_sum *= 0.9f;  /* Additional register dependency */
        }
        
        /* 5. Complex addressing for memory dependency analysis */
        int idx = i + (i % 3);  /* Non-affine index */
        if (idx < end) {
            /* Creates memory dependency edges */
            a[idx] = b[i] * 0.8f;
        }
    }
    
    *sum_acc = local_sum;
}

/* Main computation function with nested loops */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d) {
    float total_sum = 0.0f;
    int i, j;
    
    /* Outer loop with parameter bound */
    for (i = 0; i < n; ++i) {
        /* Inner loop with dependent bound - increases scheduling complexity */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (for analysis) */
        for (j = i; j < n; ++j) {
            /* Mixed operations creating various dependencies */
            float temp = a[j] * c[i];  /* Memory + register dependencies */
            
            /* Flow dependency chain */
            b[j] = temp + d[j];
            d[j] = b[j] * 0.7f;
            
            /* Anti-dependency: read after write */
            float read_after = b[j];  /* Read b[j] after it was written */
            c[j] = read_after * 0.3f;
            
            /* Output dependency: write after write */
            a[j] = c[j] + 1.0f;  /* WAW on a[j] from previous iteration */
        }
        
        /* Process chunk with inline function */
        int chunk_size = (n - i) / 2;
        if (chunk_size > 0) {
            process_chunk(a + i, b + i, c + i, d + i, 0, chunk_size, &total_sum);
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = total_sum;
}

/* Alternate computation with different patterns */
void __attribute__((noinline))
compute2(int n, float *restrict x, float *restrict y) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    int i;
    
    /* Loop designed for modulo scheduling */
    for (i = 1; i < n - 1; ++i) {
        /* Independent chains that can be parallelized */
        float chain1 = x[i-1] * 2.0f;   /* Flow dep on x[i-1] */
        float chain2 = y[i+1] * 3.0f;   /* Flow dep on y[i+1] */
        float chain3 = x[i] * y[i];     /* Flow dep on x[i], y[i] */
        
        /* Cross-chain dependencies */
        acc1 += chain1 + chain3;        /* Register flow dep on acc1 */
        acc2 = chain2 - acc1;           /* Register flow dep on acc1, chain2 */
        acc3 = acc1 * acc2;             /* Register flow dep on acc1, acc2 */
        
        /* Memory dependencies with distance */
        x[i] = acc3 + 0.5f;             /* WAW on x[i] */
        y[i] = acc1 - acc2;             /* WAW on y[i] */
        
        /* Conditional with data-dependent branch */
        if (acc3 > acc1) {
            x[i] *= 1.1f;               /* Control + data dependency */
            acc2 += 0.1f;               /* Register output dependency */
        }
    }
    
    /* Prevent elimination */
    x[0] = acc1 + acc2 + acc3;
}

int main(int argc, char **argv) {
    const int size = 1000;
    float *a = (float*)malloc(size * sizeof(float));
    float *b = (float*)malloc(size * sizeof(float));
    float *c = (float*)malloc(size * sizeof(float));
    float *d = (float*)malloc(size * sizeof(float));
    float *x = (float*)malloc(size * sizeof(float));
    float *y = (float*)malloc(size * sizeof(float));
    
    /* Initialize with pattern to avoid simple analysis */
    for (int i = 0; i < size; ++i) {
        a[i] = (i % 10) * 1.0f;
        b[i] = (i % 7) * 0.5f;
        c[i] = (i % 5) * 1.5f;
        d[i] = (i % 3) * 2.0f;
        x[i] = (float)i;
        y[i] = (float)(size - i);
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    
    compute(n, a, b, c, d);
    compute2(n, x, y);
    
    /* Additional calls with different parameters */
    compute(n/2, a, b, c, d);
    compute2(n/2, x, y);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}
