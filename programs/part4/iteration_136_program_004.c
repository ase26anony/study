/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float + b[i] * d[i];  /* Floating-point recurrence */
        
        /* Artificial register pressure with inline asm */
        asm volatile ("" : "+r"(acc_int), "+r"(acc_float));
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                local_volatile = a[i] * 3;
                acc_int += local_volatile;
                break;
            case 1:
                /* Floating point operations */
                acc_float = acc_float * 1.5f - d[i];
                break;
            case 2:
                /* Mixed operations with pointer chasing */
                acc_int = c[i] ^ (acc_int >> 2);
                acc_float = acc_float + (float)acc_int * 0.1f;
                break;
            case 3:
                /* Memory operations with different latencies */
                local_volatile = a[i] + b[i];  /* Mixed type operation */
                c[i] = local_volatile ^ acc_int;
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int * 7 + a[i]) / 3;
                acc_float = (acc_float + b[i] * 2.0f) * 0.8f;
                /* Additional asm to create scheduling constraints */
                asm volatile ("# Artificial dependency" : : "r"(acc_int));
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        int temp = 0;
        for (int j = 0; j < 3; j++) {
            temp += a[(i + j) % n] * j;
        }
        acc_int ^= temp;
    }
    
    /* Store results to volatile globals to prevent elimination */
    global_sink = acc_int;
    float_sink = acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *arr, int n) {
    int state = 0;
    int result = 0;
    
    /* Labels for computed goto */
    void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible control flow */
        goto *labels[state];
        
        L0:
            result += arr[i] * 2;
            state = (state + 1) % 4;
            continue;
        L1:
            result -= arr[i] / 3;
            state = (state * 3) % 4;
            continue;
        L2:
            result ^= arr[i] << 1;
            state = (state + 2) % 4;
            continue;
        L3:
            result = result * arr[i] + 1;
            state = (state + 3) % 4;
            continue;
    }
    
    global_sink += result;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random size to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 1000;
    if (size <= 0) size = 1000;
    
    /* Initialize with pattern (not compile-time constant) */
    int *int_arr = malloc(size * sizeof(int));
    float *float_arr = malloc(size * sizeof(float));
    int *int_arr2 = malloc(size * sizeof(int));
    float *float_arr2 = malloc(size * sizeof(float));
    
    if (!int_arr || !float_arr || !int_arr2 || !float_arr2) {
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < size; i++) {
        int_arr[i] = (i * 37) % 101;
        float_arr[i] = (float)(i % 47) * 0.7f;
        int_arr2[i] = (i * 19) % 73;
        float_arr2[i] = (float)(i % 31) * 1.3f;
    }
    
    /* Call the stress function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(int_arr, float_arr, int_arr2, float_arr2, size);
        irreducible_flow(int_arr, size);
    }
    
    /* Print result to ensure computation happens */
    printf("Result: %d, %f\n", global_sink, float_sink);
    
    free(int_arr);
    free(float_arr);
    free(int_arr2);
    free(float_arr2);
    
    return 0;
}
