/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_chunk(float *restrict a, float *restrict b, 
              float *restrict c, float *restrict d,
              int start, int end, float k) {
    float acc = 0.0f;
    float tmp;
    
    /* Loop with multiple dependency types */
    for (int i = start; i < end; ++i) {
        /* 1. Flow/true dependency on acc (register edge) */
        acc += a[i] * k;
        
        /* 2. Memory flow dependency with one-element lag */
        if (i > start) {
            b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies via swap operation */
        tmp = c[i];
        c[i] = d[i];
        d[i] = tmp;
        
        /* 4. Condition edge - depends on computed value */
        if (acc > 100.0f) {
            /* 5. Another flow dependency with non-linear indexing */
            int idx = i + (i % 3);
            if (idx < end) {
                a[idx] = acc * 0.5f;  /* Creates memory output dependency */
            }
            acc = 0.0f;  /* Reset creates register output dependency */
        }
    }
}

/* Main computation function with nested loops */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d, float k) {
    float total = 0.0f;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with dependent bounds */
        #pragma GCC ivdep  /* Assert no loop-carried memory deps (for testing) */
        for (int j = i; j < n; ++j) {
            /* Mixed memory and register dependencies */
            float t1 = a[j] * k;
            float t2 = b[j] + t1;
            
            /* Memory flow dependency with stride */
            int idx = j + (j % 2);
            if (idx < n) {
                c[idx] = t2;
            }
            
            /* Register anti-dependency via reuse */
            t1 = d[j] - t2;
            d[j] = t1 * 0.8f;
            
            /* Accumulator with flow dependency */
            total += t1;
        }
        
        /* Process chunks with the helper function */
        int chunk_size = (n - i) / 4;
        if (chunk_size > 0) {
            process_chunk(a + i, b + i, c + i, d + i, 0, chunk_size, k);
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = total;
}

/* Another variant with different access pattern */
void __attribute__((noinline))
compute2(int m, int n, float *restrict x, float *restrict y) {
    /* Two-dimensional access pattern */
    for (int i = 0; i < m; ++i) {
        float row_acc = 0.0f;
        for (int j = 0; j < n; ++j) {
            int idx = i * n + j;
            
            /* Multiple interleaved dependencies */
            float val = x[idx];
            row_acc += val;
            
            /* Memory output dependency */
            x[idx] = row_acc;
            
            /* Anti-dependency through temporary */
            float old = y[idx];
            y[idx] = val;
            val = old;
            
            /* Conditional with data-dependent branch */
            if (row_acc > 1000.0f) {
                row_acc *= 0.9f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 100;
    
    /* Allocate and initialize arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    float *x = (float*)malloc(M * N * sizeof(float));
    float *y = (float*)malloc(M * N * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 10) * 0.2f;
        c[i] = (float)(i % 5) * 0.3f;
        d[i] = (float)(i % 7) * 0.4f;
    }
    
    for (int i = 0; i < M * N; ++i) {
        x[i] = (float)(i % 20) * 0.25f;
        y[i] = (float)(i % 15) * 0.35f;
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    compute(N, a, b, c, d, 1.5f);
    compute(N/2, a, b, c, d, 0.8f);
    compute(N*3/4, a, b, c, d, 1.2f);
    
    compute2(M, N, x, y);
    compute2(M/2, N, x, y);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < M * N; ++i) {
        checksum += x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
