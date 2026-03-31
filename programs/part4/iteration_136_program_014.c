/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = global_source;
    float acc_float = 3.14159f;
    int i, j;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Mixed latency operations with control flow */
        switch (i & 3) {  /* Creates irreducible control flow */
            case 0:
                /* Integer operations */
                acc_int += (a[i] * b[i]) >> 2;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * c[i] + d[i];
                /* Memory barrier effect */
                local_volatile = acc_float;
                break;
            case 2:
                /* Mixed operations with memory access */
                acc_int = (acc_int & 0xFF) | (b[i] << 8);
                acc_float = acc_float + (float)acc_int * 0.5f;
                /* Force memory dependency */
                asm volatile ("" : : "m" (*a), "m" (*b));
                break;
            case 3:
                /* Complex recurrence with pointer chasing */
                if (i > 0) {
                    /* Create anti-dependence through memory */
                    a[i] = b[i-1] + acc_int;
                }
                /* Volatile access to prevent optimization */
                local_volatile = a[i] + b[i];
                break;
        }
        
        /* Additional nested control flow for scheduling complexity */
        if (acc_int & 1) {
            /* Conditional memory store */
            c[i] = acc_float;
            /* Artificial dependency through asm */
            asm volatile ("# dependency marker" : : "r" (acc_int));
        } else {
            /* Different operation mix */
            d[i] = acc_float * 2.0f;
            /* Another asm to create register pressure */
            asm volatile ("" : "+r" (acc_int));
        }
        
        /* Small inner loop to create additional pressure */
        for (j = 0; j < 2; j++) {
            /* Create register dependencies */
            acc_int += j;
            /* Memory operation with potential latency */
            local_volatile = acc_int;
        }
    }
    
    /* Store result to prevent dead code elimination */
    global_sink = acc_int + (int)acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_test(int *arr, int n) {
    int i;
    int sum = 0;
    int prev = 1;
    
    /* Strong distance-1 dependence chain */
    for (i = 0; i < n; i++) {
        /* Fibonacci-like recurrence */
        int next = prev + arr[i];
        arr[i] = next;
        prev = next;
        
        /* Cross-iteration floating point */
        float f = (float)next * 0.3f;
        
        /* Control flow with computed goto (simulated) */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
        goto *labels[i & 3];
        
        L0:
            sum += next * 2;
            asm volatile ("" : "+r" (sum));
            continue;
        L1:
            sum += next * 3;
            /* Memory dependency */
            asm volatile ("" : : "m" (arr[i]));
            continue;
        L2:
            sum += next * 4;
            /* Create artificial latency */
            for (int k = 0; k < 1; k++) {
                asm volatile ("nop" : : );
            }
            continue;
        L3:
            sum += next * 5;
            /* Volatile access */
            volatile int v = sum;
            (void)v;
            continue;
    }
    
    global_sink = sum;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with runtime size */
    int *a = malloc(n * sizeof(int));
    int *b = malloc(n * sizeof(int));
    float *c = malloc(n * sizeof(float));
    float *d = malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constant) */
    srand(42);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    
    /* Second test with different pattern */
    recurrence_test(a, n);
    
    /* Verify some computation happened */
    printf("Result: %d (from global_sink=%d)\n", 
           a[n-1] + (int)c[n-1], global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
