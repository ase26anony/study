/* Target: Trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_chunk(float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d,
                   int start, int end, float k) {
    float local_sum = 0.0f;
    float tmp_reg;
    
    /* Loop with multiple dependency types */
    for (int i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        local_sum += a[i] * k;
        
        /* 2. Flow dependency (memory) - array copy with one-element shift */
        if (i > 0) {
            b[i] = a[i] + a[i-1];  /* RAW on a[] */
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies - swap with temporary */
        tmp_reg = c[i];      /* Read c[i] */
        c[i] = d[i];         /* Write c[i] - output dep on c[i] */
        d[i] = tmp_reg;      /* Write d[i] - anti dep on tmp_reg */
        
        /* 4. Condition dependency - based on computed value */
        if (local_sum > 100.0f) {
            /* 5. Complex memory access pattern */
            int idx = i + (i % 3);  /* Non-affine index */
            if (idx < end) {
                a[idx] *= 0.5f;     /* Potential flow dep on a[] */
            }
        }
        
        /* 6. Independent computation chain for parallelism potential */
        float x = b[i] * 1.5f;
        float y = c[i] + d[i];
        float z = x + y;
        /* Use z to prevent dead code elimination */
        a[i] += z * 0.01f;
    }
    
    /* Store result to prevent elimination */
    if (end > 0) {
        a[end-1] += local_sum;
    }
}

/* Main computation function with nested loops */
void compute(int n, float *restrict arr1, float *restrict arr2,
             float *restrict arr3, float *restrict arr4) {
    float k = 1.618f;  /* Golden ratio */
    
    /* Outer loop with parameter bound */
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Inner loop with dependent bounds */
        int limit = n - i;
        if (limit > 10) limit = 10;
        
        /* Call inline function with chunk processing */
        process_chunk(arr1 + i, arr2 + i, arr3 + i, arr4 + i, 
                      0, limit, k + i * 0.01f);
        
        /* Additional computation with cross-iteration dependency */
        if (i > 0) {
            arr1[i] += arr2[i-1] * 0.3f;  /* Loop-carried memory dep */
        }
    }
    
    /* Second pass with different access pattern */
    for (int i = 1; i < n; ++i) {
        /* Output dependency chain */
        float old_val = arr3[i];
        arr3[i] = arr4[i-1] + arr1[i];
        arr4[i-1] = old_val - arr2[i];
        
        /* Nested conditional for control dependencies */
        for (int j = i; j < n && j < i + 5; ++j) {
            if (arr1[j] > arr2[i]) {
                arr3[j] = arr3[j] * arr4[i] + k;
            }
        }
    }
}

/* Initialize arrays with pseudo-random values */
void init_arrays(float *a, float *b, float *c, float *d, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (i * 1.0f) / n;
        b[i] = (i * 2.0f) / n;
        c[i] = (i * 3.0f) / n;
        d[i] = (i * 4.0f) / n;
    }
}

/* Checksum to prevent dead code elimination */
float checksum(float *a, float *b, float *c, float *d, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += a[i] + b[i] * 0.5f + c[i] * 0.33f + d[i] * 0.25f;
    }
    return sum;
}

int main() {
    const int sizes[] = {100, 200, 500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate and initialize arrays */
        float *arr1 = (float*)malloc(n * sizeof(float));
        float *arr2 = (float*)malloc(n * sizeof(float));
        float *arr3 = (float*)malloc(n * sizeof(float));
        float *arr4 = (float*)malloc(n * sizeof(float));
        
        init_arrays(arr1, arr2, arr3, arr4, n);
        
        /* Perform computation multiple times */
        for (int iter = 0; iter < 3; ++iter) {
            compute(n, arr1, arr2, arr3, arr4);
            
            /* Modify parameters slightly each iteration */
            arr1[0] += 0.1f;
            arr2[0] -= 0.1f;
        }
        
        /* Calculate and print checksum */
        float sum = checksum(arr1, arr2, arr3, arr4, n);
        printf("Size %d: checksum = %f\n", n, sum);
        
        /* Free memory */
        free(arr1);
        free(arr2);
        free(arr3);
        free(arr4);
    }
    
    return 0;
}
