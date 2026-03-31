/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile = seed;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    double acc_double = (double)local_volatile;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  // Integer recurrence
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                acc_int += a[i] * 2;
                /* Inline asm to create artificial register pressure */
                asm volatile ("" : "+r"(acc_int) : : "memory");
                break;
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * b[i] + 1.5f;
                /* Force memory dependency */
                local_volatile = (int)acc_float;
                break;
            case 2:
                /* Double precision operations */
                acc_double = acc_double * c[i % 8] + 2.5;
                /* Memory store with volatile */
                *(volatile double *)&c[i % 8] = acc_double;
                break;
            case 3:
                /* Mixed integer/float with pointer chasing */
                {
                    int temp = a[i] + acc_int;
                    /* Create artificial dependency chain */
                    for (int j = 0; j < 3; j++) {
                        temp = temp * 3 - j;
                    }
                    acc_int = temp;
                    /* Memory barrier */
                    asm volatile ("" ::: "memory");
                }
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                acc_int = (acc_int << 2) | (a[i] & 0xF);
                acc_float = acc_float + b[i % 8] * 0.5f;
                /* Conditional store */
                if (acc_int & 1) {
                    local_volatile = acc_int;
                }
                break;
        }
        
        /* Additional nested loop for control flow complexity */
        int k = i & 3;
        while (k > 0) {
            /* Simple operation to create additional moves */
            acc_int += k;
            k--;
        }
        
        /* Volatile access to prevent optimization */
        if (local_volatile > 1000) {
            local_volatile = 0;
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_test(int *arr, int n) {
    int sum = 0;
    int prod = 1;
    
    /* Multiple interleaved recurrences */
    for (int i = 0; i < n; i++) {
        /* Two independent distance-1 dependencies */
        sum = sum + arr[i];
        prod = prod * (arr[i] + 1);
        
        /* Conditional with goto to create irreducible flow */
        if (i % 7 == 0) {
            /* Use computed goto via label array */
            static void *labels[] = { &&L0, &&L1, &&L2 };
            goto *labels[i % 3];
        L0:
            sum += 2;
            continue;
        L1:
            prod -= 1;
            continue;
        L2:
            sum ^= prod;
            continue;
        }
        
        /* Memory operation with volatile */
        volatile int *vp = &arr[i % 16];
        *vp = sum;
    }
    
    global_sink += sum + prod;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random for runtime values */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate and initialize arrays */
    int *int_arr = (int *)malloc(n * sizeof(int));
    float *float_arr = (float *)malloc(8 * sizeof(float));
    double *double_arr = (double *)malloc(8 * sizeof(double));
    
    if (!int_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not compile-time constant) */
    srand(42);
    for (int i = 0; i < n; i++) {
        int_arr[i] = rand() % 100;
    }
    for (int i = 0; i < 8; i++) {
        float_arr[i] = (float)(rand() % 100) / 10.0f;
        double_arr[i] = (double)(rand() % 100) / 5.0;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(int_arr, float_arr, double_arr, n, rand());
    recurrence_test(int_arr, n / 2);
    
    /* Print result to ensure execution */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
