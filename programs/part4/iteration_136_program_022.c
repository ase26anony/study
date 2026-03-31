/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loop */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Volatile variables to enforce dependencies */
volatile int global_sink = 0;
volatile int global_source = 123456789;

/* Complex loop with cross-iteration dependencies */
NO_UNROLL
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n) {
    /* Carried scalar dependency - creates distance-1 dependences */
    int acc_int = global_source;
    float acc_float = (float)global_source;
    
    /* Volatile locals to prevent optimization */
    volatile int v1 = 0;
    volatile float v2 = 0.0f;
    
    /* Pointer chasing variable */
    int *ptr = a;
    
    for (int i = 0; i < n; i++) {
        /* Cross-iteration recurrence: a[i] depends on a[i-1] */
        acc_int = acc_int * a[i] + b[i];
        
        /* Mixed latency operations */
        float temp_float = c[i] * d[i];
        acc_float = acc_float + temp_float;
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer operations */
                acc_int = (acc_int << 3) | (acc_int >> 29); /* rotate */
                /* Inline asm to create artificial dependencies */
                asm volatile ("" : "+r" (acc_int) : : "memory");
                break;
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * 1.01f;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 2:
                /* Memory operations with pointer chasing */
                v1 = *ptr;
                ptr = &a[(i + 1) % n];
                acc_int ^= v1;
                break;
            case 3:
                /* Mixed integer/float with volatile */
                v2 = acc_float;
                acc_int += (int)v2;
                /* Artificial dependency through asm */
                asm volatile ("" : "+r" (acc_int), "+r" (acc_float) : : );
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int * 1103515245 + 12345) & 0x7fffffff;
                acc_float = acc_float * 0.99f + (float)acc_int * 0.001f;
                /* Force register usage */
                asm volatile ("" : : "r" (acc_int), "r" (acc_float) : );
                break;
        }
        
        /* Additional nested conditional to create control flow pressure */
        if (i % 7 == 0) {
            /* Small inner loop with carried dependency */
            int inner_acc = acc_int;
            for (int j = 0; j < 3; j++) {
                inner_acc = (inner_acc * 13 + 17) % 1000;
                /* Memory operation */
                v1 = inner_acc;
            }
            acc_int ^= inner_acc;
        } else if (i % 3 == 0) {
            /* Alternative path with float operations */
            acc_float = acc_float - (float)acc_int * 0.5f;
        }
        
        /* Store to volatile to prevent elimination */
        if (i % 11 == 0) {
            v1 = acc_int;
            v2 = acc_float;
        }
    }
    
    /* Final store to global volatile */
    global_sink = acc_int + (int)acc_float;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, float *c, float *d, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = (float)(seed & 0xFF) * 0.01f;
        d[i] = (float)((seed >> 16) & 0xFF) * 0.01f;
    }
}

int main(int argc, char *argv[]) {
    /* Use runtime-determined size to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 10;
        if (n > 10000) n = 10000;
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
    
    /* Initialize with pattern */
    init_arrays(a, b, c, d, n);
    
    /* Call the critical loop function */
    modulo_sched_stress(a, b, c, d, n);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
