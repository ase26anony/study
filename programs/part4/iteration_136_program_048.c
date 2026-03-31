/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    float acc_f = 1.0f;
    int acc_i = 1;
    
    /* Cross-iteration dependencies with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_i = acc_i * a[i] + c[i];  /* Integer recurrence */
        acc_f = acc_f * b[i] + d[i];  /* Float recurrence */
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                local_volatile = acc_i;
                a[i] = (a[i] * 3) / 2;
                /* Inline asm to create register pressure */
                asm volatile ("" : "+r" (acc_i) : : "memory");
                break;
                
            case 1:
                /* Floating point operations */
                b[i] = acc_f * 2.5f - 1.0f;
                /* Force memory barrier */
                asm volatile ("" : : : "memory");
                break;
                
            case 2:
                /* Mixed integer/float with pointer chasing */
                c[i] = acc_i + (int)acc_f;
                d[i] = (float)acc_i * 0.7f;
                /* Artificial dependency chain */
                asm volatile ("# dependency chain" : "+r" (acc_i), "+r" (acc_f));
                break;
                
            case 3:
                /* Conditional store with volatile */
                if (acc_i > 1000) {
                    local_volatile = a[i % n];
                }
                /* Complex expression with multiple uses */
                acc_i = (acc_i << 2) | (acc_i >> 30);
                break;
                
            case 4:
                /* Nested loop to create additional pressure */
                for (int j = 0; j < 3; j++) {
                    acc_f = acc_f + (float)j * 0.1f;
                }
                /* Memory operation with side effect */
                asm volatile ("# side effect" : "=m" (c[i]) : "r" (acc_i));
                break;
        }
        
        /* Additional cross-iteration dependency */
        if (i > 0) {
            a[i] += a[i-1] % 7;  /* Another distance-1 dependence */
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_i;
    global_float_sink = acc_f;
}

/* Helper to initialize arrays with pattern */
void init_arrays(int *a, float *b, int *c, float *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 17) % 23;
        b[i] = (float)((i * 13) % 19) * 0.7f;
        c[i] = (i * 11) % 29;
        d[i] = (float)((i * 7) % 31) * 0.3f;
    }
}

int main(int argc, char **argv) {
    /* Use runtime value to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(a, b, c, d, n);
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < n; i++) {
            a[i] += rand() % 5;
            b[i] += (float)(rand() % 3) * 0.1f;
        }
    }
    
    /* Print a result to prevent dead code elimination */
    printf("Result: %d, %f\n", global_sink, global_float_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
