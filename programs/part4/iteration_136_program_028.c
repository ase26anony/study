/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int local_volatile = seed;  /* Force memory dependencies */
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    double acc_double = (double)local_volatile;
    
    /* Cross-iteration dependencies with different recurrence types */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  /* Integer recurrence */
        
        /* Complex control flow with switch */
        switch (i & 3) {  /* i % 4 */
            case 0:
                /* Mixed operations with different latencies */
                acc_float = acc_float + b[i] * 1.5f;
                /* Inline assembly to create artificial dependencies */
                asm volatile ("" : "+r" (acc_int) : : "memory");
                break;
                
            case 1:
                /* Memory operations with potential higher latency */
                acc_double = acc_double * c[i] - 2.0;
                /* Force register use */
                asm volatile ("" : "+r" (acc_float) : : "memory");
                break;
                
            case 2:
                /* Integer operations with conditional */
                if (acc_int & 1) {
                    acc_int = acc_int ^ a[i];
                } else {
                    acc_int = acc_int | (i << 3);
                }
                /* More inline assembly */
                asm volatile ("" : "+r" (acc_double) : : "memory");
                break;
                
            case 3:
                /* Mixed type operations */
                acc_float = acc_float - (float)acc_int;
                acc_double = acc_double + (double)acc_float;
                /* Multiple dependencies */
                asm volatile ("" : "+r" (acc_int), "+r" (acc_float) : : "memory");
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        int temp = acc_int;
        for (int j = 0; j < 2; j++) {
            temp = (temp << 1) | (temp >> 31);  /* Rotate */
            asm volatile ("" : "+r" (temp) : : "memory");
        }
        acc_int = temp;
        
        /* Pointer chasing to create memory dependencies */
        volatile int *ptr = &local_volatile;
        *ptr = acc_int;
    }
    
    /* Store results to prevent dead code elimination */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
}

/* Another function with irreducible control flow using computed goto */
__attribute__((optimize("no-unroll-loops")))
void irreducible_control_flow(int *arr, int n, int init) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    int state = init & 3;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[state];
        
        L0:
            sum = sum * arr[i] + 1;
            state = (sum >> 1) & 3;
            asm volatile ("" : "+r" (sum) : : "memory");
            continue;
            
        L1:
            sum = sum + (arr[i] << 2);
            state = (sum >> 2) & 3;
            asm volatile ("" : "+r" (sum) : : "memory");
            continue;
            
        L2:
            sum = sum ^ arr[i];
            state = (sum >> 3) & 3;
            asm volatile ("" : "+r" (sum) : : "memory");
            continue;
            
        L3:
            sum = sum - arr[i];
            state = (sum >> 4) & 3;
            asm volatile ("" : "+r" (sum) : : "memory");
            continue;
    }
    
    global_sink += sum;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random for runtime values */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with runtime size */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    if (!int_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not compile-time constant) */
    srand(42);
    for (int i = 0; i < n; i++) {
        int_arr[i] = rand() % 100;
        float_arr[i] = (float)(rand() % 100) / 10.0f;
        double_arr[i] = (double)(rand() % 100) / 5.0;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(int_arr, float_arr, double_arr, n, rand());
    irreducible_control_flow(int_arr, n / 2, rand());
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
