/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent loop unrolling and keep dependencies */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Volatile variables to prevent optimization */
volatile int global_result = 0;
volatile int *volatile global_ptr = NULL;

/* Function with complex loop for modulo scheduling */
NO_UNROLL
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n, int seed) {
    volatile int local_acc = seed;
    volatile float fp_acc = (float)seed;
    volatile int temp;
    
    /* Cross-iteration dependencies with recurrence */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        local_acc = local_acc * a[i] + b[i-1];  /* Recurrence with distance 1 */
        
        /* Mixed latency operations */
        fp_acc = fp_acc * c[i] + d[i-1];  /* Floating-point recurrence */
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations */
                temp = a[i] * b[i] + local_acc;
                /* Inline asm to create artificial dependencies */
                asm volatile ("" : "+r" (temp) : : "memory");
                local_acc = temp ^ (local_acc >> 3);
                break;
            case 1:
                /* Floating-point operations (higher latency) */
                fp_acc = fp_acc * 1.1f + (float)local_acc;
                /* Memory store/load */
                global_ptr = &temp;
                temp = *global_ptr + i;
                break;
            case 2:
                /* Mixed operations with pointer chasing */
                if (i > 2) {
                    local_acc = a[local_acc % (i-1)] + b[i];
                }
                /* Another asm barrier */
                asm volatile ("" : "+r" (local_acc) : : "memory");
                break;
            case 3:
                /* Conditional execution path */
                if (local_acc % 7 == 0) {
                    fp_acc = fp_acc / 2.0f;
                } else {
                    local_acc = local_acc * 3 + 1;
                }
                break;
            case 4:
                /* Nested loop to create scheduling pressure */
                for (int j = 0; j < 2; j++) {
                    temp = (local_acc << j) | (local_acc >> (32 - j));
                    asm volatile ("" : "+r" (temp));
                }
                /* Memory dependency */
                a[i] = temp + i;
                break;
        }
        
        /* Additional cross-iteration dependency */
        if (i > 1) {
            b[i] = b[i-2] + local_acc;  /* Distance-2 dependence */
        }
        
        /* Volatile store to prevent dead code elimination */
        global_result = local_acc + (int)fp_acc;
    }
    
    /* Final store with asm to ensure all operations complete */
    asm volatile ("" : : "r" (local_acc), "r" (fp_acc) : "memory");
}

/* Helper function to initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, float *c, float *d, int n, int seed) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
}

/* Main function with runtime-determined loop count */
int main(int argc, char **argv) {
    int n = 1000;
    int seed = 42;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Allocate arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(a, b, c, d, n, seed);
    
    /* Call the stress function - loop count not known at compile time */
    modulo_sched_stress(a, b, c, d, n, seed);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (from global: %d)\n", a[n-1] + b[n-1], global_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
