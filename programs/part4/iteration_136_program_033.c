/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdlib.h>
#include <stdio.h>

/* Global volatile to prevent optimization */
volatile int global_result = 0;
volatile int global_counter = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile = seed;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    double acc_double = (double)local_volatile;
    
    /* Pointer chasing variable - creates distance-1 dependence */
    int *chase_ptr = &local_volatile;
    int chase_val = 0;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        chase_val = *chase_ptr + i;
        chase_ptr = &chase_val;
        
        /* Recurrence relation with mixed operations */
        acc_int = acc_int * a[i] + (i % 256);
        
        /* Conditional execution based on i */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                acc_int += b[i] * 2;
                asm volatile ("" : "+r"(acc_int) : : "memory");
                break;
            case 1:
                /* Floating point operations */
                acc_float = acc_float * 0.99f + b[i % n];
                asm volatile ("" : "+r"(acc_float) : : "memory");
                break;
            case 2:
                /* Double precision with conversion */
                acc_double = acc_double * 0.999 + c[i % n];
                acc_int += (int)acc_double;
                break;
            case 3:
                /* Memory operations with pointer chasing */
                a[i] = chase_val + acc_int;
                chase_val = a[i] * 3;
                break;
            case 4:
                /* Mixed operations with inline assembly */
                int temp = acc_int;
                asm volatile ("addl $1, %0" : "+r"(temp));
                acc_float += temp;
                acc_double = acc_double * 0.5 + temp;
                break;
        }
        
        /* Additional control flow within loop */
        if (i % 7 == 0) {
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; j++) {
                acc_int += j * (i % 11);
                asm volatile ("" : "+r"(acc_int));
            }
        } else if (i % 3 == 0) {
            /* Alternative path */
            acc_float = acc_float / 1.5f;
            asm volatile ("" : "+r"(acc_float));
        }
        
        /* Volatile store to prevent elimination */
        global_counter = i;
    }
    
    /* Combine results to prevent dead code elimination */
    global_result = acc_int + (int)acc_float + (int)acc_double + chase_val;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_loop(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int state = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible control flow */
        goto *labels[state];
        
        label0:
            sum += arr[i] * 2;
            state = (state + 1) % 4;
            continue;
        label1:
            sum -= arr[i] / 3;
            state = (state + 2) % 4;
            continue;
        label2:
            sum ^= arr[i] << 1;
            state = (state + 3) % 4;
            continue;
        label3:
            sum |= arr[i] >> 1;
            state = (state + 1) % 4;
            continue;
    }
    
    global_result ^= sum;
}

/* Main function with data initialization */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < n; i++) {
        int_arr[i] = (i * 1103515245 + 12345) & 0x7FFF;
        float_arr[i] = (float)((i * 1664525 + 1013904223) % 100) / 10.0f;
        double_arr[i] = (double)((i * 214013 + 2531011) % 200) / 20.0;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(int_arr, float_arr, double_arr, n, iter * 100);
        irreducible_loop(int_arr, n);
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d (counter: %d)\n", global_result, global_counter);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
