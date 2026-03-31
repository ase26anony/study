/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;  /* Force memory access */
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    
    /* Cross-iteration recurrence variables */
    int acc_int = v1;
    float acc_float = v2;
    double acc_double = v3;
    
    /* Pointer chasing variable */
    int *ptr = &a[0];
    
    /* Main loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + (i % 256);  /* Recurrence relation */
        
        /* Mixed latency operations */
        float temp_float = b[i] * 1.5f;
        acc_float = acc_float + temp_float;    /* FP operation */
        
        /* Memory load with potential variable latency */
        double load_val = c[i];
        acc_double = acc_double * 0.99 + load_val;
        
        /* Pointer chasing creates memory dependence chain */
        int chase_val = *ptr;
        ptr = &a[i];
        a[i] = chase_val + i;
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic path */
                acc_int += (acc_int >> 3) & 0x1F;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Floating-point path */
                acc_float = acc_float * 0.9f + b[i-1];
                asm volatile ("" : "+r" (acc_float));
                break;
            case 2:
                /* Memory operations path */
                b[i] = acc_float;
                c[i] = acc_double;
                asm volatile ("" : : "m" (b[i]), "m" (c[i]));
                break;
            case 3:
                /* Mixed operations */
                acc_int ^= (int)acc_float;
                acc_double += (double)acc_int;
                asm volatile ("" : "+r" (acc_int), "+r" (acc_double));
                break;
            default:
                /* Conditional store */
                if (acc_int > 1000) {
                    a[i] = acc_int % 256;
                }
                /* Nested loop for additional pressure */
                for (int j = 0; j < 2; j++) {
                    acc_int += j;
                }
                break;
        }
        
        /* Volatile store to prevent optimization */
        volatile int barrier = acc_int;
        (void)barrier;
    }
    
    /* Store results to global to prevent elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Helper function to initialize arrays */
void init_arrays(int *a, float *b, double *c, int n, int seed) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (double)(rand() % 100) / 20.0;
    }
}

int main(int argc, char *argv[]) {
    /* Use runtime values to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    int seed = 42;
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Dynamically allocate to avoid stack overflow */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, n, seed);
    
    /* Call the stress function multiple times to increase scheduling attempts */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, n, seed + iter);
    }
    
    /* Print result to ensure computation happens */
    printf("Result: %d\n", global_sink);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
