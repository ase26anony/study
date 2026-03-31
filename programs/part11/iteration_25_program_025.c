/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024

/* Helper function with inline hint to keep loop in context */
static void __attribute__((always_inline)) 
process_chunk(float *restrict a, float *restrict b, 
              float *restrict c, float *restrict d,
              int start, int end, float *restrict sum_out) {
    float sum = 0.0f;
    float tmp_reg;
    
    /* Loop with multiple dependency types to trigger DDG edge creation */
    for (int i = start; i < end; ++i) {
        /* 1. REGISTER FLOW DEPENDENCY (RAW on sum) */
        sum += a[i] * 1.5f;
        
        /* 2. MEMORY FLOW DEPENDENCY with cross-iteration dependency */
        /* b[i] depends on a[i] and a[i-1] (loop-carried when i>start) */
        if (i > start) {
            b[i] = a[i] + a[i-1] * 0.7f;
        } else {
            b[i] = a[i];
        }
        
        /* 3. REGISTER ANTI and OUTPUT DEPENDENCIES (swap pattern) */
        tmp_reg = c[i];
        c[i] = d[i];
        d[i] = tmp_reg;
        
        /* 4. CONDITION DEPENDENCY */
        /* The condition depends on sum computed above */
        if (sum > 100.0f) {
            /* 5. MEMORY OUTPUT DEPENDENCY (WAW on b[i]) */
            b[i] = b[i] * 0.5f;
            sum = sum * 0.9f; /* Another register flow dependency */
        }
        
        /* 6. Complex addressing for memory dependency analysis */
        int idx = i + (i % 3) - 1;
        if (idx >= start && idx < end) {
            /* MEMORY FLOW from previous iteration through complex index */
            d[i] += b[idx] * 0.3f;
        }
    }
    
    *sum_out = sum;
}

/* Main computation function with nested loops */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float total_sum = 0.0f;
    
    /* Outer loop with parameter bound */
    for (int outer = 0; outer < 3; ++outer) {
        float chunk_sum;
        
        /* Try to hint no loop-carried dependencies for outer loop */
        #pragma GCC ivdep
        for (int i = 0; i < n; ++i) {
            /* Modify arrays slightly each outer iteration */
            a[i] += outer * 0.1f;
        }
        
        /* Process in chunks to create more complex control flow */
        for (int chunk_start = 0; chunk_start < n; chunk_start += 256) {
            int chunk_end = chunk_start + 256;
            if (chunk_end > n) chunk_end = n;
            
            /* Inner loop with cross-iteration dependencies */
            process_chunk(a, b, c, d, chunk_start, chunk_end, &chunk_sum);
            total_sum += chunk_sum;
        }
        
        /* Additional loop with anti-dependencies */
        for (int i = 1; i < n; ++i) {
            /* ANTI-DEPENDENCY (WAR): reading b[i] after it was written earlier */
            float read_b = b[i];
            /* Then write to a different location */
            a[i-1] = read_b * 0.8f + c[i];
        }
    }
    
    /* Final reduction that depends on all previous computations */
    float final_sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use the result to prevent dead code elimination */
    if (final_sum > 10000.0f) {
        printf("Large result: %f\n", final_sum);
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant size to prevent complete loop unrolling */
    int n = SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > 10000) n = SIZE;
    }
    
    /* Allocate with dynamic size to avoid compile-time knowledge */
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constant to avoid optimization) */
    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        a[i] = (float)(rand() % 100) / 10.0f;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(n, a, b, c, d);
    
    /* Second call with different size pattern */
    if (n > 100) {
        compute(n/2, a, b, c, d);
    }
    
    /* Calculate checksum to ensure all computations are used */
    float checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += a[i] + b[i] * 0.3f + c[i] * 0.5f + d[i] * 0.7f;
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
