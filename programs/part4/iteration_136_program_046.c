/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 123456789;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *arr_a, int *arr_b, float *arr_f, 
                         int n, int *result) 
{
    volatile int vol_var1 = global_source;
    volatile int vol_var2 = global_source + 1;
    volatile float vol_float = 3.14159f;
    
    /* Cross-iteration dependency variable */
    int acc_int = vol_var1;
    float acc_float = vol_float;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * arr_a[i] + arr_b[i];
        
        /* Mixed latency operations */
        float temp_float = acc_float * 1.5f + arr_f[i];
        
        /* Artificial inline assembly to create register dependencies */
        int asm_var = acc_int;
        asm volatile ("# Artificial dependency" : "+r" (asm_var));
        acc_int = asm_var;
        
        /* Complex control flow using switch */
        switch (i % 4) {
            case 0:
                /* Integer operations */
                arr_a[i] = acc_int & 0xFF;
                acc_float = temp_float * 0.9f;
                break;
            case 1:
                /* Floating point operations */
                arr_f[i] = temp_float + acc_float;
                acc_int += (int)(temp_float * 100);
                break;
            case 2:
                /* Memory operations with volatile */
                vol_var1 = arr_b[i] * 2;
                acc_float = vol_float + arr_f[i];
                break;
            case 3:
                /* Mixed operations */
                arr_b[i] = acc_int / (arr_a[i] + 1);
                acc_float = acc_float - arr_f[i];
                /* Another inline assembly to create scheduling complexity */
                asm volatile ("# Complex operation %0" : "+r" (acc_int));
                break;
        }
        
        /* Additional nested control flow for irreducible graph */
        if (i % 3 == 0) {
            /* Pointer chasing to create memory dependencies */
            int *ptr = &arr_a[i];
            for (int j = 0; j < 2; j++) {
                *ptr += (*ptr) * j;
                ptr = &arr_b[i];
            }
        } else if (i % 5 == 0) {
            /* Another recurrence */
            acc_int = (acc_int << 3) | (acc_int >> 29);
        }
        
        /* Store to volatile to prevent dead code elimination */
        global_sink = acc_int;
    }
    
    /* Final results with cross-iteration dependency */
    *result = acc_int + (int)acc_float;
    
    /* Force another volatile operation */
    asm volatile ("# Final barrier" : : : "memory");
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr, float *farr, int n, int *out) 
{
    volatile double vol_double = 2.71828;
    double acc_double = vol_double;
    int acc = global_source;
    
    /* Loop with pointer-based recurrence */
    for (int i = 1; i < n; i++) {
        /* Strong distance-1 dependence */
        arr[i] = arr[i-1] * 3 + arr[i];
        
        /* Floating point recurrence */
        farr[i] = farr[i-1] * 1.1 + farr[i];
        
        /* Mixed operation with conditional */
        acc = (acc * 1103515245 + 12345) & 0x7fffffff;
        
        if (i % 7 == 0) {
            /* Use computed goto for irreducible flow */
            static void *labels[] = { &&label1, &&label2, &&label3 };
            goto *labels[i % 3];
            
            label1:
                acc_double += farr[i];
                goto end_label;
            label2:
                acc_double *= 0.99;
                goto end_label;
            label3:
                acc_double = acc_double / (farr[i] + 1.0);
                goto end_label;
            end_label:;
        }
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    *out = acc + (int)acc_double;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, float *f, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        f[i] = (seed & 0xFF) / 255.0f * 100.0f;
    }
}

int main(int argc, char **argv) {
    /* Use command line or default size */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays */
    int *arr_a = malloc(n * sizeof(int));
    int *arr_b = malloc(n * sizeof(int));
    float *arr_f = malloc(n * sizeof(float));
    int result1, result2;
    
    if (!arr_a || !arr_b || !arr_f) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(arr_a, arr_b, arr_f, n);
    
    /* Call the stress functions */
    modulo_sched_stress(arr_a, arr_b, arr_f, n, &result1);
    second_loop(arr_a, arr_f, n, &result2);
    
    /* Use results to prevent optimization */
    printf("Results: %d %d\n", result1, result2);
    printf("Global sink: %d\n", global_sink);
    
    /* Clean up */
    free(arr_a);
    free(arr_b);
    free(arr_f);
    
    return 0;
}
