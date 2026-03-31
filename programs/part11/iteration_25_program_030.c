/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_chunk(float *restrict a, float *restrict b, float *restrict c,
                   int start, int end, float *restrict sum, float scalar) {
    float local_sum = *sum;
    float tmp1, tmp2;
    
    /* Complex loop with multiple dependency types */
    for (int i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register & memory) */
        /* Reduction pattern with loop-carried dependency on local_sum */
        local_sum += a[i] * scalar;
        
        /* 2. Memory flow dependency with one-element lag */
        /* b[i] depends on a[i] and a[i-1] from previous iteration */
        if (i > start) {
            b[i] = a[i] + a[i-1] + b[i-1] * 0.5f;
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies via swap operation */
        /* Uses temporaries to create register anti/output dependencies */
        tmp1 = c[i];
        tmp2 = c[i + 1];  /* May create memory anti-dependency */
        c[i] = tmp2 * 0.7f;
        c[i + 1] = tmp1 * 1.3f;
        
        /* 4. Conditional creating control/condition dependencies */
        /* Condition depends on value computed in current iteration */
        if (local_sum > 1000.0f) {
            local_sum *= 0.99f;  /* Register output dependency */
            b[i] += 1.0f;        /* Memory output dependency */
        }
        
        /* 5. Independent arithmetic chains for potential parallelism */
        /* Creates multiple DDG edges with different latencies */
        float x = a[i] * 2.0f;
        float y = b[i] + 3.0f;
        float z = x * y;
        c[i] += z * 0.1f;
    }
    
    *sum = local_sum;
}

/* Main computation function with nested loops */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float scalar) {
    float total_sum = 0.0f;
    
    /* Outer loop with non-constant bound */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with dependency on outer index */
        /* Creates complex address calculations */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (may trigger DDG verification) */
        for (int j = i; j < n; ++j) {
            /* Memory dependency with non-affine index */
            int idx = j + (i % 3);
            if (idx < n) {
                /* Flow dependency through memory */
                a[idx] = a[idx] * scalar + b[j];
                
                /* Anti-dependency: read then write */
                float temp = c[j];
                c[j] = a[idx] * temp;
                
                /* Output dependency */
                b[j] = b[j] * 2.0f - c[j];
            }
        }
        
        /* Process chunks to create more DDG opportunities */
        if (i % 4 == 0) {
            process_chunk(a, b, c, 0, n, &total_sum, scalar);
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    for (int i = 0; i < n; ++i) {
        total_sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    if (total_sum < 0) {
        printf("Unexpected negative sum\n");
    }
}

/* Initialize arrays with pattern */
void init_arrays(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = (float)((i + 2) % 100) * 0.3f;
    }
}

int main() {
    const int sizes[] = {100, 200, 300};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate with extra space for safety */
        float *a = (float*)malloc((n + 10) * sizeof(float));
        float *b = (float*)malloc((n + 10) * sizeof(float));
        float *c = (float*)malloc((n + 10) * sizeof(float));
        
        if (!a || !b || !c) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        init_arrays(a, b, c, n + 10);
        
        /* Call compute with different scalars to create varying patterns */
        compute(n, a, b, c, 1.5f);
        compute(n / 2, a, b, c, 0.75f);
        compute(n, a, b, c, 1.1f);
        
        /* Calculate checksum to prevent dead code elimination */
        float checksum = 0.0f;
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i];
        }
        
        printf("Size %d: checksum = %f\n", n, checksum);
        
        free(a);
        free(b);
        free(c);
    }
    
    return 0;
}
