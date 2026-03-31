/* modulo-sched-test.c
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -fno-tree-vectorize -fdump-rtl-sms -fdump-rtl-sched2 -dA modulo-sched-test.c -o modulo-sched-test
 * For 32-bit: add -m32 -mtune=pentium4
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile;  /* Force memory dependencies */
    int acc_int = seed;
    float acc_float = seed * 0.5f;
    double acc_double = seed * 0.25;
    
    /* Pointer chasing to create distance-1 dependencies */
    int *ptr = &acc_int;
    float *fptr = &acc_float;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 recurrence: current depends on previous iteration */
        acc_int = acc_int * a[i] + i;  /* Integer recurrence */
        acc_float = acc_float + b[i] * 1.5f;  /* Float recurrence */
        acc_double = acc_double * 0.9 + c[i];  /* Double recurrence */
        
        /* Mixed latency operations */
        int temp_int;
        float temp_float;
        double temp_double;
        
        /* Irreducible control flow using computed goto */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic with inline asm to prevent optimization */
                asm volatile ("/* Integer op */" : "+r" (acc_int));
                temp_int = acc_int * 3 + a[i];
                /* Memory store with volatile */
                local_volatile = temp_int;
                break;
                
            case 1:
                /* Floating point operation (higher latency) */
                temp_float = acc_float * 2.0f - b[i];
                /* Force register use with inline asm */
                asm volatile ("/* Float op */" : "+f" (temp_float));
                acc_float = temp_float;
                break;
                
            case 2:
                /* Double precision operation */
                temp_double = acc_double / 1.7 + c[i];
                /* Memory load with pointer chasing */
                acc_double = temp_double + *(&acc_double);
                break;
                
            case 3:
                /* Mixed type operations */
                temp_int = (int)acc_float + (int)acc_double;
                /* Conditional execution */
                if (temp_int % 7) {
                    acc_int = acc_int ^ temp_int;
                } else {
                    acc_int = acc_int | (a[i] << 3);
                }
                break;
                
            case 4:
                /* Complex memory access pattern */
                *ptr = acc_int + i;
                ptr = (i % 3) ? &acc_int : &local_volatile;
                
                *fptr = acc_float + i * 0.1f;
                fptr = (i % 2) ? &acc_float : (float*)&local_volatile;
                break;
        }
        
        /* Additional nested control flow for scheduling complexity */
        if (i % 11 == 0) {
            int j = i;
            while (j > 0 && j % 3 != 0) {
                /* Small inner loop with memory ops */
                local_volatile = j;
                j--;
            }
        }
        
        /* Cross-iteration pointer dependency */
        a[i] = acc_int;
        b[i] = acc_float;
        c[i] = acc_double;
    }
    
    /* Prevent dead code elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Helper to initialize arrays with pattern */
void init_arrays(int *a, float *b, double *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 17) % 101;
        b[i] = (i * 23) % 103 * 0.7f;
        c[i] = (i * 29) % 107 * 1.3;
    }
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Allocate arrays with dynamic size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(a, b, c, n);
    
    /* Call the stress function multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, n, seed + iter);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    /* Check for dump files */
    printf("Compile with -fdump-rtl-sms and -fdump-rtl-sched2 to see modulo scheduling debug output\n");
    printf("Look for lines like: '%%11d %%11d %%5d %%d --(T,%%d,%%d)--> %%d\\n' in dump files\n");
    
    return 0;
}
