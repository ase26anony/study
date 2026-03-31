/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would eliminate the loop */
volatile int global_sink;

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
                /* Integer operations with memory access */
                a[i] = acc_int & 0xFF;
                /* Inline asm to create artificial register pressure */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Float operations */
                b[i] = acc_float * 2.0f;
                /* Memory barrier effect */
                asm volatile ("" : : "memory");
                break;
            case 2:
                /* Double precision with volatile */
                local_volatile = i;
                acc_double = acc_double * c[i] + local_volatile;
                break;
            case 3:
                /* Mixed operations with pointer chasing */
                if (i > 0) {
                    a[i] = a[i-1] + a[i];  // Another distance-1 dependence
                }
                /* Force register usage */
                asm volatile ("" : "+r" (acc_int), "+r" (acc_float));
                break;
        }
        
        /* Additional nested conditional to create control flow complexity */
        if (i % 7 == 0) {
            /* More operations with dependencies */
            for (int j = 0; j < 3; j++) {
                acc_int += j;
                asm volatile ("" : "+r" (acc_int));
            }
        } else if (i % 5 == 0) {
            acc_float = acc_float - 1.0f;
        }
        
        /* Store to volatile to prevent dead code elimination */
        if (i % 13 == 0) {
            global_sink = acc_int;
        }
    }
    
    /* Final store to prevent optimization */
    global_sink = acc_int + (int)acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_test(int *arr, int n, int init) {
    int carry = init;
    volatile int temp;
    
    for (int i = 0; i < n; i++) {
        /* Strong distance-1 dependence chain */
        carry = carry * 3 + arr[i];
        
        /* Conditional with irreducible control flow */
        void *labels[] = { &&label1, &&label2, &&label3, &&label4 };
        goto *labels[i % 4];
        
        label1:
            arr[i] = carry & 1;
            asm volatile ("" : "+r" (carry));
            goto end_switch;
        label2:
            temp = i;
            carry += temp;
            goto end_switch;
        label3:
            /* Memory operation with latency */
            arr[i] = arr[i] + carry;
            asm volatile ("" : : "memory");
            goto end_switch;
        label4:
            /* Complex expression */
            carry = (carry << 2) | (carry >> 30);
            asm volatile ("" : "+r" (carry));
            goto end_switch;
            
        end_switch:
        
        /* Additional operation to create more moves */
        if (i % 3 == 0) {
            asm volatile ("" : "+r" (carry));
        }
    }
    
    global_sink = carry;
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    if (n < 10) n = 1000;  /* Ensure sufficient iterations */
    
    /* Allocate arrays with runtime size */
    int *arr1 = (int*)malloc(n * sizeof(int));
    float *arr2 = (float*)malloc(n * sizeof(float));
    double *arr3 = (double*)malloc(n * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create varied values */
    srand(seed);
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = (rand() % 100) * 0.1f;
        arr3[i] = (rand() % 100) * 0.01;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(arr1, arr2, arr3, n, seed);
    
    /* Call second function with different data pattern */
    int *arr4 = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr4[i] = rand() % 50;
    }
    recurrence_test(arr4, n, seed);
    
    /* Use results to prevent dead code elimination */
    printf("Result check: %d %d\n", arr1[n-1], arr4[n-1]);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
