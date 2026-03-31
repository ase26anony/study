/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations */
volatile int global_sink;

/* Function with complex loop for modulo scheduling */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    int acc_int = seed;
    float acc_float = seed * 0.1f;
    double acc_double = seed * 0.01;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  // Integer recurrence
        
        /* Mixed latency operations */
        acc_float = acc_float + b[i] * 1.5f;  // FP operation
        
        /* Memory access with volatile to prevent optimization */
        v1 = a[i];
        v2 = b[i];
        v3 = c[i];
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer arithmetic with inline asm to create register pressure */
                asm volatile ("" : "+r" (acc_int));
                acc_int = (acc_int * 3) / 2;
                break;
            case 1:
                /* Floating point operation */
                acc_float = acc_float * 2.0f - 1.0f;
                /* Inline asm for artificial latency */
                asm volatile ("# FP operation case" : : "r" (acc_float));
                break;
            case 2:
                /* Mixed integer/float with memory */
                acc_double = acc_double + (double)a[i] * 0.5;
                v3 = acc_double;
                break;
            case 3:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int + a[i]) ^ (i & 0xFF);
                acc_float = acc_float + (float)acc_int * 0.01f;
                asm volatile ("# Complex case" : : "r" (acc_int), "r" (acc_float));
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        int temp = 0;
        for (int j = 0; j < 3; j++) {
            temp += a[(i + j) % n] * j;
            asm volatile ("" : "+r" (temp));
        }
        acc_int += temp;
        
        /* Store to volatile to prevent dead code elimination */
        global_sink = acc_int;
    }
    
    /* Final store to prevent optimization */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void pointer_chasing(int *arr, int n, int start) {
    volatile int index = start;
    int sum = 0;
    
    /* Pointer chasing with distance-1 dependence */
    for (int i = 0; i < n; i++) {
        /* Each iteration depends on previous index calculation */
        index = (index * 1103515245 + 12345) & 0x7FFFFFFF;
        int idx = index % n;
        
        /* Complex addressing with multiple dependencies */
        sum = sum + arr[idx] - arr[(idx + 1) % n];
        
        /* Conditional execution */
        if (idx % 3 == 0) {
            sum = sum * 2;
            asm volatile ("# Branch taken" : : "r" (sum));
        } else if (idx % 3 == 1) {
            sum = sum / 2;
            asm volatile ("# Branch not taken" : : "r" (sum));
        } else {
            sum = sum ^ arr[(idx + 2) % n];
        }
        
        /* Memory barrier */
        asm volatile ("" : : "r" (arr), "r" (idx));
    }
    
    global_sink = sum;
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) * 0.1f;
        c[i] = (double)(rand() % 100) * 0.01;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, n, rand());
    
    /* Another call with different parameters */
    pointer_chasing(a, n / 2, rand());
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", global_sink);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
