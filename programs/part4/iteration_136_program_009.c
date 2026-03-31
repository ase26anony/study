/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    int acc_int = seed;
    float acc_float = seed * 1.5f;
    double acc_double = seed * 2.5;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + (i % 256);
        
        /* Complex control flow to create multiple scheduling paths */
        switch (i % 4) {
            case 0: {
                /* Integer operations with memory access */
                int temp = a[i] * 3;
                asm volatile ("" : "+r"(temp) : : "memory");
                acc_int += temp;
                
                /* Volatile memory access */
                v1 = acc_int;
                a[i] = v1;
                break;
            }
            case 1: {
                /* Floating point operations (higher latency) */
                float ftemp = b[i] * 2.0f;
                asm volatile ("" : "+f"(ftemp) : : "memory");
                acc_float = acc_float * 0.9f + ftemp;
                
                /* Mixed-type operations */
                v2 = acc_float;
                b[i] = v2;
                break;
            }
            case 2: {
                /* Double precision operations */
                double dtemp = c[i] * 1.5;
                asm volatile ("" : "+r"(dtemp) : : "memory");
                acc_double = acc_double * 0.8 + dtemp;
                
                /* Memory store with dependency */
                v3 = acc_double;
                c[i] = v3;
                break;
            }
            case 3: {
                /* Mixed operations creating complex dependencies */
                int itemp = a[i] ^ acc_int;
                float ftemp = b[i] + acc_float;
                double dtemp = c[i] - acc_double;
                
                asm volatile ("" : "+r"(itemp), "+f"(ftemp), "+r"(dtemp) : : "memory");
                
                /* Cross-type accumulation */
                acc_int += itemp;
                acc_float += ftemp;
                acc_double += dtemp;
                
                /* All volatile stores to enforce ordering */
                v1 = acc_int;
                v2 = acc_float;
                v3 = acc_double;
                break;
            }
        }
        
        /* Additional nested control flow within loop */
        if (i % 3 == 0) {
            /* Pointer chasing to create memory dependencies */
            int *ptr = &a[i];
            asm volatile ("" : "+r"(ptr) : : "memory");
            *ptr = *ptr + 1;
        } else if (i % 3 == 1) {
            /* Another recurrence relation */
            acc_int = (acc_int << 3) | (acc_int >> 29);
        } else {
            /* Complex arithmetic with multiple dependencies */
            acc_float = acc_float * acc_float - b[i];
            asm volatile ("" : "+f"(acc_float) : : "memory");
        }
        
        /* Artificial label-based control flow (computed goto) */
        static void *labels[] = { &&label1, &&label2, &&label3 };
        goto *labels[i % 3];
        
    label1:
        /* Some computation */
        acc_int ^= 0xAAAAAAAA;
        continue;
        
    label2:
        /* Different computation */
        acc_float *= 0.75f;
        continue;
        
    label3:
        /* Yet another computation */
        acc_double /= 1.1;
        continue;
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Helper to initialize arrays with pattern */
void init_arrays(int *a, float *b, double *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        b[i] = (float)a[i] / 1000.0f;
        c[i] = (double)a[i] / 500.0;
    }
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Dynamic allocation to prevent constant propagation */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    init_arrays(a, b, c, n);
    
    /* Run the stress test multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, n, iter * 12345 + 67890);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", global_sink);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
