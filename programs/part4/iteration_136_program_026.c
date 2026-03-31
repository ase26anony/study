/* Test program to trigger modulo scheduling debug output in GCC */
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
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float + b[i] * d[i];  /* Floating-point recurrence */
        
        /* Artificial use to prevent optimization */
        asm volatile ("" : "+r"(acc_int), "+r"(acc_float));
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                a[i] = acc_int >> 2;
                local_volatile = a[i-1];  /* Another distance-1 use */
                break;
            case 1:
                /* Floating point operations */
                d[i] = acc_float * 1.5f;
                /* Force memory barrier */
                asm volatile ("" ::: "memory");
                break;
            case 2:
                /* Mixed operations */
                acc_int += (int)(acc_float * 100.0f);
                c[i] = acc_int & 0xFF;
                break;
            case 3:
                /* Pointer chasing style dependency */
                acc_int = a[acc_int % n] + c[i];
                /* Inline asm to simulate latency */
                asm volatile ("nop; nop; nop" ::: "memory");
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                acc_float = (acc_float + b[i-1]) * (d[i] - 0.5f);
                acc_int = (acc_int * 3) / 2;
                break;
        }
        
        /* Additional conditional inside loop */
        if (i % 7 == 0) {
            /* Nested loop for additional pressure */
            int temp = 0;
            for (int j = 0; j < 3; j++) {
                temp += a[(i + j) % n];
            }
            acc_int ^= temp;
        } else if (i % 11 == 0) {
            /* Alternative path */
            acc_float = acc_float / 2.0f;
            asm volatile ("" : "+r"(acc_float));
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_control_flow(int *arr, int n) {
    int state = 0;
    int sum = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[state];
        
        L0:
            sum += arr[i] * 2;
            state = (state + 1) % 5;
            /* Distance-1 dependence */
            arr[i] = sum + (i > 0 ? arr[i-1] : 0);
            continue;
        L1:
            sum -= arr[i];
            state = (state * 3) % 5;
            asm volatile ("" : "+r"(sum));
            continue;
        L2:
            sum = sum ^ arr[i];
            state = (state + 2) % 5;
            /* Memory operation with latency */
            volatile int* vptr = &arr[i];
            *vptr = sum;
            continue;
        L3:
            sum = sum << 1;
            state = (state + 3) % 5;
            continue;
        L4:
            sum = sum >> 1;
            state = (state + 4) % 5;
            /* Force dependency chain */
            asm volatile ("nop" ::: "memory");
            continue;
    }
    
    global_sink = sum;
}

int main(int argc, char **argv) {
    /* Use runtime value to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 10) n = 1000;
    
    /* Allocate arrays with dynamic size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pseudo-random pattern */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = rand() % 100;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    irreducible_control_flow(a, n/10);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
