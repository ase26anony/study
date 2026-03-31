/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = (float)seed;
    
    /* Cross-iteration dependencies with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Another distance-1 dependence */
        acc_float = acc_float * b[i] + d[i];
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                a[i] = acc_int & 0xFF;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
                
            case 1:
                /* Floating point operations */
                b[i] = acc_float * 1.5f;
                /* Memory barrier effect */
                asm volatile ("" : : "memory");
                break;
                
            case 2:
                /* Mixed operations with pointer chasing */
                if (i > 0) {
                    c[i] = c[i-1] + a[i];  /* Another distance-1 dependence */
                }
                /* Force register usage */
                asm volatile ("" : "+r" (c[i]));
                break;
                
            case 3:
                /* Complex expression with multiple dependencies */
                d[i] = (acc_float + b[i]) * (float)acc_int;
                /* Artificial latency */
                for (int j = 0; j < 2; j++) {
                    asm volatile ("nop" : : : "cc");
                }
                break;
                
            case 4:
                /* Memory operations with volatile */
                local_volatile = a[i] + c[i];
                acc_int ^= local_volatile;
                /* Explicit latency hint */
                asm volatile ("mfence" : : : "memory");
                break;
        }
        
        /* Additional nested loop for pressure */
        int temp = 0;
        for (int k = 0; k < 3; k++) {
            temp += k * i;
        }
        acc_int += temp;
    }
    
    /* Store to global volatile to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_control_flow(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[i % 4];
        
    label0:
        sum += arr[i] * 2;
        arr[i] = sum;
        continue;
        
    label1:
        sum -= arr[i];
        /* Memory dependency */
        if (i > 0) arr[i] += arr[i-1];  /* Distance-1 */
        continue;
        
    label2:
        sum ^= arr[i];
        /* Artificial operation */
        asm volatile ("" : "+r" (sum));
        continue;
        
    label3:
        sum = sum * 3 + arr[i];
        /* Volatile access */
        volatile int v = sum;
        arr[i] = v;
        continue;
    }
    
    global_sink += sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = rand() % 100;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Run the stress test multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n, rand());
        irreducible_control_flow(a, n);
    }
    
    /* Print something to prevent optimization */
    printf("Result: %d, %f\n", global_sink, global_float_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
