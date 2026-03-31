/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

/* Helper function with loop that will be inlined */
static void __attribute__((always_inline)) 
process_chunk(float *restrict a, float *restrict b, float *restrict c, 
               float *restrict d, int start, int end, float k) {
    float temp1, temp2;
    float accum = 0.0f;
    
    /* Loop with multiple dependency types */
    for (int i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        accum += a[i] * k;
        
        /* 2. Flow dependency (memory) - with one-element lag */
        if (i > 0) {
            b[i] = a[i] + a[i-1] * 0.5f;
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies - swap with temporary */
        temp1 = c[i];
        temp2 = d[i];
        c[i] = temp2 * 1.1f;      /* Output dependency on c[i] */
        d[i] = temp1 * 0.9f;      /* Output dependency on d[i] */
        
        /* 4. Condition dependency - based on computed value */
        if (accum > 100.0f) {
            /* Creates condition edge from accum calculation */
            b[i] *= 0.5f;
            accum = accum * 0.8f; /* Anti-dependency on accum */
        }
        
        /* 5. Complex addressing to prevent simple analysis */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < end) {
            /* Creates memory flow dependency with non-linear access */
            c[i] += b[idx] * 0.3f;
        }
    }
    
    /* Use accum to prevent dead code elimination */
    d[start] += accum;
}

/* Main computation function with nested loops */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d, float k) {
    float total = 0.0f;
    
    /* Outer loop with parameterized bound */
    for (int i = 0; i < n; i += 64) {
        int chunk_size = (n - i) > 64 ? 64 : (n - i);
        
        /* Try to hint no loop-carried dependencies for outer loop */
        #pragma GCC ivdep
        for (int j = 0; j < chunk_size; ++j) {
            /* Inner loop with dependent bound */
            for (int k = j; k < chunk_size; ++k) {
                /* Nested loop with flow dependency */
                int idx = i + k;
                if (idx < n) {
                    /* Memory flow dependency with stride */
                    a[idx] = b[idx] * 2.0f - c[idx];
                    
                    /* Register anti-dependency via temporary */
                    float tmp = d[idx];
                    d[idx] = a[idx] + tmp * 0.5f;
                    c[idx] = tmp * 0.7f;  /* Output dependency on c[idx] */
                    
                    /* Reduction with loop-carried dependency */
                    total += d[idx];
                }
            }
        }
        
        /* Process chunk with mixed dependencies */
        process_chunk(a, b, c, d, i, i + chunk_size, k);
    }
    
    /* Use total to prevent elimination */
    a[0] = total * 0.01f;
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

/* Checksum to verify computation and prevent dead code elimination */
float checksum(float *a, float *b, float *c, float *d, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += a[i] + b[i] * 0.5f + c[i] * 0.3f + d[i] * 0.2f;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use variable sizes to prevent constant propagation */
    int sizes[] = {256, 512, 768, SIZE};
    int num_sizes = 4;
    
    /* Allocate arrays */
    float *a = (float*)malloc(SIZE * sizeof(float));
    float *b = (float*)malloc(SIZE * sizeof(float));
    float *c = (float*)malloc(SIZE * sizeof(float));
    float *d = (float*)malloc(SIZE * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    float total_checksum = 0.0f;
    
    /* Run computation with different sizes to increase coverage chance */
    for (int run = 0; run < num_sizes; ++run) {
        int n = sizes[run];
        
        /* Initialize with different patterns each run */
        init_arrays(a, b, c, d, n);
        
        /* Vary the multiplier to create different dependency patterns */
        float k = 1.0f + 0.1f * run;
        
        /* Perform computation - this should trigger DDG construction */
        compute(n, a, b, c, d, k);
        
        /* Calculate and accumulate checksum */
        float cs = checksum(a, b, c, d, n);
        total_checksum += cs;
        
        printf("Run %d (n=%d): checksum = %f\n", run, n, cs);
    }
    
    printf("Total checksum: %f\n", total_checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    
    return 0;
}
