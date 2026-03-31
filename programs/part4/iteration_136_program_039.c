/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loop */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
NO_UNROLL
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;  /* Force memory access */
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    
    /* Cross-iteration recurrence with distance-1 dependence */
    int acc_int = v1;
    float acc_float = v2;
    double acc_double = v3;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        /* Base recurrence: a[i] depends on previous iteration */
        acc_int = acc_int * a[i] + i;
        
        /* Switch creates irreducible control flow */
        switch (i & 3) {  /* i % 4 */
            case 0:
                /* Integer operations with artificial latency */
                acc_int += (a[i] << 2) | 1;
                asm volatile ("" : "+r" (acc_int));  /* Prevent optimization */
                break;
                
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * b[i] + 1.0f;
                /* Force register use */
                asm volatile ("" : "+r" (i), "+r" (acc_float));
                break;
                
            case 2:
                /* Mixed operations with memory access */
                acc_double = acc_double + c[i] * 0.5;
                a[i] = acc_int ^ (int)acc_double;  /* Store with dependency */
                /* Memory barrier effect */
                asm volatile ("" : : "r" (a[i]), "r" (c[i]) : "memory");
                break;
                
            case 3:
                /* Complex operation chain */
                {
                    int temp = acc_int;
                    temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                    acc_int = temp;
                    acc_float += (float)temp * 0.001f;
                    acc_double -= (double)temp * 0.0001;
                }
                /* Multiple register constraints */
                asm volatile ("" : "+r" (acc_int), "+r" (acc_float), "+r" (acc_double));
                break;
        }
        
        /* Additional conditional inside loop */
        if (i & 1) {
            /* Pointer chasing creates memory dependence */
            volatile int *ptr = &a[i & (n-1)];
            acc_int ^= *ptr;
        } else {
            /* Another recurrence */
            acc_float = acc_float + b[i & (n-1)] * 2.0f;
        }
        
        /* Nested loop for additional pressure */
        for (int j = 0; j < 2; j++) {
            /* Small computation to create more moves */
            acc_int += j;
            asm volatile ("" : "+r" (acc_int));
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with different pattern */
NO_UNROLL
void second_loop(int *arr, int n, int init) {
    volatile int carry = init;
    
    /* Different recurrence pattern */
    for (int i = 0; i < n; i++) {
        /* Fibonacci-like recurrence with distance-2 */
        if (i >= 2) {
            arr[i] = arr[i-1] + arr[i-2] + carry;
        } else if (i == 1) {
            arr[i] = arr[0] + carry;
        } else {
            arr[i] = carry;
        }
        
        /* Computed goto simulation */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
        goto *labels[i & 3];
        
        L0:
            carry += arr[i] * 3;
            continue;
        L1:
            carry -= arr[i] / 2;
            continue;
        L2:
            carry ^= arr[i] | 0xFF;
            continue;
        L3:
            carry = (carry << 3) | (carry >> 29);  /* Rotate */
            continue;
    }
    
    global_sink += carry;
}

int main(int argc, char **argv) {
    /* Use arguments to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    if (n < 16) n = 16;  /* Ensure minimum size */
    
    /* Allocate arrays with different types */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    if (!int_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constant) */
    srand(seed);
    for (int i = 0; i < n; i++) {
        int_arr[i] = rand() % 100;
        float_arr[i] = (float)(rand() % 100) * 0.1f;
        double_arr[i] = (double)(rand() % 100) * 0.01;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(int_arr, float_arr, double_arr, n, seed + iter);
        second_loop(int_arr, n, seed + iter * 7);
    }
    
    /* Use results */
    printf("Result: %d (seed: %d, n: %d)\n", global_sink, seed, n);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
