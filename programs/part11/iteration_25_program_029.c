/* Test program to cover DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with loop that should be inlined */
static inline __attribute__((always_inline)) 
void process_chunk(float * restrict a, float * restrict b, 
                   float * restrict c, float * restrict d,
                   int start, int end, float *sum) {
    float local_sum = 0.0f;
    float tmp1, tmp2;
    int i;
    
    /* Complex loop with multiple dependency types */
    #pragma GCC ivdep
    for (i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        local_sum += a[i] * 1.5f;
        
        /* 2. Flow dependency (memory) - array with one-element shift */
        if (i > 0) {
            b[i] = a[i] + a[i-1] * 0.7f;
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies - swap operation */
        tmp1 = c[i];
        c[i] = d[i];
        d[i] = tmp1;
        
        /* 4. Condition dependency */
        if (local_sum > 100.0f) {
            /* 5. Another flow dependency chain */
            tmp2 = b[i] * 2.0f;
            c[i] += tmp2;
            local_sum -= 50.0f;  /* Creates register dependency */
        }
        
        /* 6. Complex addressing for memory dependency analysis */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < end) {
            d[i] += a[idx] * 0.3f;  /* Memory flow dependency */
        }
    }
    
    *sum += local_sum;
}

/* Main computation function with nested loops */
void compute(int n, float * restrict a, float * restrict b, 
             float * restrict c, float * restrict d) {
    float total_sum = 0.0f;
    int i, j;
    
    /* Outer loop with parameter bound */
    for (i = 0; i < n; i += 100) {
        int chunk_size = (n - i) < 100 ? (n - i) : 100;
        
        /* Process chunk with complex dependencies */
        process_chunk(a + i, b + i, c + i, d + i, 0, chunk_size, &total_sum);
        
        /* Inner loop with dependent bound - creates more scheduling complexity */
        for (j = i; j < n && j < i + 50; ++j) {
            /* Cross-iteration dependency with non-linear access */
            float temp = a[j] * b[j];
            c[j] = temp + (j > 0 ? c[j-1] * 0.5f : 0.0f);  /* Loop-carried dependency */
            
            /* Multiple independent chains for potential parallelism */
            float chain1 = a[j] * 1.1f;
            float chain2 = b[j] + 2.2f;
            d[j] = chain1 + chain2;
            
            /* Conditional that depends on computed values */
            if (temp > 50.0f) {
                a[j] *= 0.9f;  /* Creates output dependency on a[j] */
                total_sum += temp;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = total_sum * 0.001f;
}

/* Initialize arrays with values */
void init_arrays(float *a, float *b, float *c, float *d, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = (float)((i + 2) % 100) * 0.3f;
        d[i] = (float)((i + 3) % 100) * 0.4f;
    }
}

int main() {
    /* Allocate arrays */
    float *a = malloc(SIZE * sizeof(float));
    float *b = malloc(SIZE * sizeof(float));
    float *c = malloc(SIZE * sizeof(float));
    float *d = malloc(SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(a, b, c, d, SIZE);
    
    /* Call compute multiple times with different sizes to increase
       chance of DDG construction in different contexts */
    compute(SIZE, a, b, c, d);
    compute(SIZE / 2, a, b, c, d);
    compute(SIZE * 3 / 4, a, b, c, d);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += a[i] + b[i] * 0.5f + c[i] * 0.3f + d[i] * 0.2f;
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
