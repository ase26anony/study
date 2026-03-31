/* Test program for modulo scheduling debug output coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Cross-iteration dependence variable */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: acc_int from previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Mixed latency operations */
        float temp_float = b[i] * 1.5f;
        acc_float = acc_float + temp_float;
        
        /* Irreducible control flow using switch */
        switch (i % 4) {
            case 0:
                /* Integer operations */
                c[i] = acc_int >> 2;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point operations */
                d[i] = acc_float * 0.5f;
                /* Memory barrier effect */
                asm volatile ("" : : "memory");
                break;
            case 2:
                /* Mixed operations with memory access */
                a[i] = acc_int + i;
                acc_float = d[i-1] + 1.0f;  /* Another distance-1 dependence */
                break;
            case 3:
                /* Complex operation chain */
                int tmp = a[i-1] * 3;  /* Distance-1 memory dependence */
                c[i] = tmp + acc_int;
                /* Force register use */
                asm volatile ("" : "+r" (tmp), "+r" (acc_int));
                break;
        }
        
        /* Additional conditional execution */
        if (i % 3 == 0) {
            /* Nested loop for additional pressure */
            int inner_acc = 0;
            for (int j = 0; j < 2; j++) {
                inner_acc += c[i-j];
            }
            local_volatile = inner_acc;
        } else if (i % 5 == 0) {
            /* Alternative path */
            acc_float = acc_float * 2.0f - 1.0f;
        }
        
        /* Pointer chasing to create memory dependencies */
        volatile int *ptr = &c[i];
        *ptr = *ptr + 1;
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_relation(int *x, int *y, int n) {
    int carry = x[0];
    float fp_carry = (float)y[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple recurrence relations */
        carry = (carry * 3 + x[i]) % 1000;
        fp_carry = fp_carry * 0.9f + (float)y[i];
        
        /* Unpredictable control flow using computed goto */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
        goto *labels[i % 4];
        
        L0:
            x[i] = carry + i;
            /* Artificial latency */
            asm volatile ("nop" : : : "memory");
            goto cont;
        L1:
            y[i] = (int)fp_carry ^ carry;
            goto cont;
        L2:
            /* Memory operation with potential latency */
            volatile int mem_op = x[i-1] + y[i-1];  /* Distance-1 */
            x[i] = mem_op;
            goto cont;
        L3:
            /* Complex dependency chain */
            fp_carry = fp_carry + (float)(x[i-1] * y[i-1]);
            goto cont;
        
        cont:
        /* Additional operation to increase ILP */
        if (i & 1) {
            asm volatile ("" : "+r" (carry));
        }
    }
    
    global_sink = carry;
}

int main(int argc, char **argv) {
    /* Use arguments to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 10) n = 1000;
    
    /* Allocate arrays with dynamic size */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *arr3 = (float*)malloc(n * sizeof(float));
    float *arr4 = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pattern */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = (float)(rand() % 100) / 10.0f;
        arr4[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(arr1, arr3, arr2, arr4, n);
    recurrence_relation(arr1, arr2, n);
    
    /* Use results */
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
