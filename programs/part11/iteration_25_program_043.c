/* Test program to cover DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_chunk(float *restrict a, float *restrict b, float *restrict c,
                   float *restrict d, int start, int end, float *sum) {
    float local_sum = *sum;
    float tmp1, tmp2;
    int i;
    
    /* Loop with various dependency patterns */
    for (i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        local_sum += a[i] * 1.5f;
        
        /* 2. Flow dependency (memory) - array copy with one-element shift */
        if (i > 0) {
            b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti-dependency and output dependency - swap with temporary */
        tmp1 = c[i];
        c[i] = d[i];
        d[i] = tmp1;
        
        /* 4. Condition dependency - based on computed value */
        if (local_sum > 100.0f) {
            /* 5. Complex memory access with non-affine index */
            int idx = i + (i % 3);  /* Non-linear index calculation */
            if (idx < end) {
                a[idx] *= 0.9f;  /* Creates memory dependencies */
            }
        }
        
        /* 6. Independent computation chain for potential parallelism */
        tmp2 = b[i] * 2.0f;
        float tmp3 = tmp2 + c[i];
        d[i] = tmp3 * 0.5f;
    }
    
    *sum = local_sum;
}

/* Main computation function with nested loops */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float total_sum = 0.0f;
    int i, j;
    
    /* Outer loop with parameter bound */
    for (i = 0; i < n; ++i) {
        /* Inner loop with dependent bound - increases scheduling complexity */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (for analysis) */
        for (j = i; j < n; ++j) {
            /* Mixed operations creating various dependencies */
            
            /* Register flow dependency */
            float acc = a[j];
            
            /* Memory flow dependency with conditional */
            if (j > 0 && (j % 2 == 0)) {
                b[j] = acc + b[j-1];  /* Loop-carried dependency */
            }
            
            /* Anti-dependency through temporary */
            float old_c = c[j];
            c[j] = d[j] * old_c;
            d[j] = old_c;
            
            /* Update accumulator with non-linear computation */
            acc = acc * acc - 2.0f * b[j];
            total_sum += acc;
            
            /* Conditional with data-dependent branch */
            if (total_sum > 0) {
                /* Create output dependency */
                a[j] = total_sum * 0.01f;
            }
        }
        
        /* Process chunks to trigger inline function's DDG creation */
        if (i % 64 == 0) {
            process_chunk(a, b, c, d, i, (i + 32 < n) ? i + 32 : n, &total_sum);
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    a[0] = total_sum;
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

int main(int argc, char *argv[]) {
    /* Use different sizes to test various compilation paths */
    int sizes[] = {100, 200, 500, 1000};
    float checksum = 0.0f;
    
    for (int s = 0; s < 4; ++s) {
        int n = sizes[s];
        if (argc > 1) {
            n = atoi(argv[1]);
            if (n <= 0) n = 100;
        }
        
        /* Allocate with alignment hint for better optimization */
        float *a = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *b = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *c = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *d = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        
        if (!a || !b || !c || !d) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        init_arrays(a, b, c, d, n);
        
        /* Call compute multiple times to increase optimization opportunities */
        for (int iter = 0; iter < 3; ++iter) {
            compute(n, a, b, c, d);
        }
        
        /* Calculate checksum to prevent dead code elimination */
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
        
        printf("Size %d: checksum = %f\n", n, checksum);
        
        free(a);
        free(b);
        free(c);
        free(d);
    }
    
    return 0;
}
