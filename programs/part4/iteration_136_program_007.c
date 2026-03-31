/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = seed * 0.5f;
    double acc_double = seed * 0.25;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  // Integer recurrence
        
        /* Mixed latency operations */
        acc_float = acc_float + b[i] * 1.5f;  // Float operation
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer operations with inline asm to prevent optimization */
                {
                    int temp = acc_int & 0xFF;
                    asm volatile ("" : "+r"(temp) : :);
                    acc_int = temp + a[i];
                }
                break;
            case 1:
                /* Floating point with memory access */
                acc_double = acc_double * 0.9 + c[i];
                {
                    double temp_d = acc_double;
                    asm volatile ("" : "+r"(temp_d) : :);
                    acc_double = temp_d;
                }
                break;
            case 2:
                /* Mixed operations with volatile access */
                local_volatile = a[i];
                acc_int = acc_int ^ local_volatile;
                acc_float = acc_float - b[i];
                break;
            case 3:
                /* Pointer chasing style dependence */
                if (i > 0) {
                    acc_int = acc_int + a[i-1];  // Explicit distance-1 use
                }
                /* Inline asm with multiple outputs */
                {
                    int out1, out2;
                    asm volatile ("mov %1, %0\n\t"
                                  "add $1, %0" 
                                  : "=r"(out1), "=r"(out2) 
                                  : "r"(acc_int));
                    acc_int = out1;
                }
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        {
            int j = i & 3;
            while (j > 0) {
                acc_int += j;
                j--;
            }
        }
        
        /* Conditional store with computed goto (irreducible flow) */
        static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
        if (i % 5 == 0) {
            goto *labels[i % 4];
        }
        
        label0:
            a[i] = acc_int;
            continue;
        label1:
            b[i] = acc_float;
            continue;
        label2:
            c[i] = acc_double;
            continue;
        label3:
            /* Do nothing */
            continue;
    }
    
    /* Prevent dead code elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with different recurrence pattern */
__attribute__((noinline))
void complex_recurrence(int *arr, int n, int init) {
    int carry1 = init;
    int carry2 = init * 2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple interleaved recurrences */
        int temp1 = carry1 + arr[i];
        int temp2 = carry2 - arr[i];
        
        /* Create anti-dependencies */
        asm volatile ("" : : "r"(temp1), "r"(temp2) :);
        
        /* Swap and update with conditional */
        if (i % 3 == 0) {
            carry1 = temp2;
            carry2 = temp1;
        } else {
            carry1 = temp1 * 2;
            carry2 = temp2 / 2;
        }
        
        /* Memory barrier effect */
        asm volatile ("" : : : "memory");
        
        arr[i] = carry1 + carry2;
    }
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    if (n <= 0) n = 1000;
    
    /* Allocate arrays with dynamic sizes */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    if (!int_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(seed);
    for (int i = 0; i < n; i++) {
        int_arr[i] = rand() % 100;
        float_arr[i] = (rand() % 100) * 0.1f;
        double_arr[i] = (rand() % 100) * 0.01;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(int_arr, float_arr, double_arr, n, seed + iter);
        complex_recurrence(int_arr, n / 2, seed + iter * 10);
    }
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += int_arr[i] + (int)float_arr[i] + (int)double_arr[i];
    }
    
    printf("Result: %d (global_sink: %d)\n", sum, global_sink);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
