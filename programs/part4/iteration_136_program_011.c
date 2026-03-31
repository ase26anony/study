/* modulo-sched-test.c
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 * For 32-bit: add -m32 -mtune=pentium4
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations */
volatile int global_sink;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = seed * 0.5f;
    double acc_double = seed * 0.25;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  /* Integer recurrence */
        
        /* Mixed operations with different latencies */
        switch (i & 3) {  /* Complex control flow */
            case 0:
                /* Integer operations (lower latency) */
                acc_int += (a[i] & 0xFF) * 3;
                /* Artificial assembly to prevent optimization */
                asm volatile ("" : "+r"(acc_int));
                break;
                
            case 1:
                /* Floating-point operations (higher latency) */
                acc_float = acc_float * 0.9f + b[i];
                /* Force memory dependency */
                local_volatile = acc_int;
                break;
                
            case 2:
                /* Mixed integer/float with memory access */
                acc_double = acc_double * 0.8 + c[i];
                /* Volatile access creates memory barrier */
                global_sink = i;
                break;
                
            case 3:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int >> 2) + (a[i] * acc_int);
                acc_float = acc_float + (float)acc_int * 0.1f;
                /* Inline assembly for register pressure */
                asm volatile ("# dummy asm" : : "r"(acc_int), "r"(acc_float));
                break;
        }
        
        /* Additional nested control flow within loop */
        if (i & 1) {
            /* Pointer chasing creates memory dependence */
            int *ptr = &a[i];
            acc_int += *ptr;
        } else {
            /* Different path with float operations */
            acc_float += 1.0f;
        }
        
        /* Computed goto-like structure using switch */
        int target = (i * 13) % 4;
        switch (target) {
            case 0: acc_int ^= 0x55; break;
            case 1: acc_float *= 1.1f; break;
            case 2: acc_double -= 0.1; break;
            case 3: acc_int = ~acc_int; break;
        }
    }
    
    /* Prevent dead code elimination */
    global_sink = acc_int;
    *(volatile float *)&global_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Strong distance-1 dependence */
        arr1[i] = arr1[i-1] * 2 + arr2[i];
        
        /* Conditional with both paths used */
        if (arr1[i] > 1000) {
            sum1 += arr1[i] / 3;
            /* Memory operation */
            asm volatile ("# mem op" : : "m"(arr1[i]));
        } else {
            sum2 += arr1[i] * 2;
            /* Different operation mix */
            float temp = (float)arr1[i] * 0.7f;
            asm volatile ("# float use" : : "r"(temp));
        }
        
        /* Loop-variant switch */
        switch (i % 5) {
            case 0: arr1[i] += sum1; break;
            case 1: arr1[i] -= sum2; break;
            case 2: arr1[i] ^= 0xFF; break;
            case 3: arr1[i] *= 3; break;
            case 4: arr1[i] = ~arr1[i]; break;
        }
    }
    
    global_sink = sum1 + sum2;
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Dynamic allocation prevents compile-time analysis */
    int *arr_int = malloc(n * sizeof(int));
    float *arr_float = malloc(n * sizeof(float));
    double *arr_double = malloc(n * sizeof(double));
    int *arr1 = malloc(n * sizeof(int));
    int *arr2 = malloc(n * sizeof(int));
    
    if (!arr_int || !arr_float || !arr_double || !arr1 || !arr2) {
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    int seed = 42;
    srand(seed);
    
    for (int i = 0; i < n; i++) {
        arr_int[i] = rand() % 1000;
        arr_float[i] = (rand() % 1000) * 0.01f;
        arr_double[i] = (rand() % 1000) * 0.001;
        arr1[i] = rand() % 500;
        arr2[i] = rand() % 500;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(arr_int, arr_float, arr_double, n, seed);
    second_loop(arr1, arr2, n);
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", global_sink);
    
    free(arr_int);
    free(arr_float);
    free(arr_double);
    free(arr1);
    free(arr2);
    
    return 0;
}
