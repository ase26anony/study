/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_chunk(float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d,
                   int start, int end, float *restrict sum) {
    float local_sum = *sum;
    float tmp1, tmp2;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = start; i < end; ++i) {
        /* 1. REGISTER FLOW dependency (accumulator) */
        local_sum += a[i] * 1.5f;
        
        /* 2. MEMORY FLOW dependency with 1-element lag */
        if (i > 0) {
            b[i] = a[i] + a[i-1] * 0.7f;  /* True/flow dependency on a[i-1] */
        } else {
            b[i] = a[i];
        }
        
        /* 3. ANTI and OUTPUT dependencies via swap */
        tmp1 = c[i];
        c[i] = d[i];      /* Output dependency on c[i] */
        d[i] = tmp1;      /* Anti-dependency on tmp1 */
        
        /* 4. CONDITION dependency */
        float threshold = local_sum * 0.01f;
        if (c[i] > threshold) {  /* Condition depends on local_sum */
            /* 5. MEMORY ANTI dependency */
            b[i] += c[i] * 0.3f;  /* Anti-dependency on b[i] from line 2 */
        }
        
        /* 6. Complex addressing for memory dependencies */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < end) {
            /* Potential memory flow across iterations */
            d[i] += b[idx] * 0.2f;
        }
    }
    
    *sum = local_sum;
}

/* Main computation function with nested loops */
void __attribute__((noinline)) 
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d) {
    float sum = 0.0f;
    int i, j;
    
    /* Outer loop with parameter bound */
    for (i = 0; i < n; ++i) {
        /* Initialize arrays with values */
        a[i] = (float)(i % 100) * 0.1f;
        c[i] = (float)(i % 50) * 0.2f;
        d[i] = (float)(i % 70) * 0.15f;
    }
    
    /* Try to hint no loop-carried dependencies */
    #pragma GCC ivdep
    /* Nested loop where inner bound depends on outer index */
    for (i = 1; i < n - 1; ++i) {
        /* Multiple independent chains for potential parallelism */
        float chain1 = a[i] * 2.0f;
        float chain2 = c[i] + d[i];
        float chain3 = chain1 - chain2;
        
        /* Cross-iteration flow dependency */
        b[i] = chain3 + b[i-1] * 0.5f;  /* True dependency on b[i-1] */
        
        /* Inner loop with varying bound */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Register output dependency */
            float tmp = a[j];
            a[j] = tmp * 0.8f + c[j] * 0.2f;  /* Output on a[j] */
            
            /* Memory anti-dependency */
            c[j] = d[j] + tmp;  /* Anti on tmp (which was a[j]) */
        }
        
        /* Conditional with data-dependent branch */
        if (b[i] > 10.0f) {
            d[i] = b[i] * 0.3f;
        } else {
            d[i] = b[i] * 0.7f;
        }
    }
    
    /* Process in chunks to create more complex DDG */
    for (i = 0; i < n; i += 64) {
        int chunk_end = (i + 64 < n) ? i + 64 : n;
        process_chunk(a, b, c, d, i, chunk_end, &sum);
    }
    
    /* Use result to prevent elimination */
    a[0] += sum * 0.001f;
}

int main(int argc, char **argv) {
    const int sizes[] = {100, 500, 1000};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int s;
    
    for (s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        if (argc > 1) n = atoi(argv[1]) % 1000 + 100;
        
        /* Allocate with alignment hint */
        float *a = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *b = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *c = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *d = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        
        if (!a || !b || !c || !d) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }
        
        /* Initialize with simple pattern */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)i;
            b[i] = (float)(i * 2);
            c[i] = (float)(i * 3);
            d[i] = (float)(i * 4);
        }
        
        /* Perform computation */
        compute(n, a, b, c, d);
        
        /* Checksum to prevent dead code elimination */
        float checksum = 0.0f;
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] * 0.5f + c[i] * 0.3f + d[i] * 0.2f;
        }
        
        printf("Size %d: checksum = %f\n", n, checksum);
        
        free(a);
        free(b);
        free(c);
        free(d);
    }
    
    return 0;
}
