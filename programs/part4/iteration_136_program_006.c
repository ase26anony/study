/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile long global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    int acc_int = seed;
    float acc_float = seed * 0.1f;
    double acc_double = seed * 0.01;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;  /* Integer recurrence */
        acc_float = acc_float + b[i] * 1.5f;  /* Float recurrence */
        acc_double = acc_double * 0.9 + c[i];  /* Double recurrence */
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                v1 = v1 * acc_int;
                asm volatile ("" : "+r"(v1) : : "memory");
                a[i] = v1 + i;
                break;
            case 1:
                /* Floating point operations */
                v2 = v2 + acc_float * 2.0f;
                asm volatile ("" : "+r"(v2) : : "memory");
                b[i] = v2 * 0.7f;
                break;
            case 2:
                /* Double precision with memory barrier */
                v3 = v3 * acc_double - 1.0;
                asm volatile ("" : "+r"(v3) : : "memory");
                c[i] = v3 / 3.0;
                break;
            case 3:
                /* Mixed operations */
                v1 = v1 ^ (acc_int & 0xFF);
                v2 = v2 + (float)v1 * 0.1f;
                asm volatile ("" : "+r"(v1), "+r"(v2) : : "memory");
                break;
            case 4:
                /* Pointer chasing creating memory dependence */
                {
                    volatile int *ptr = &a[i];
                    v1 = *ptr + v1;
                    asm volatile ("" : "+r"(v1) : : "memory");
                }
                break;
        }
        
        /* Additional nested loop for control flow complexity */
        for (int j = 0; j < 2; j++) {
            if ((i + j) % 3 == 0) {
                v1 += j;
                asm volatile ("" : "+r"(v1) : : "memory");
            }
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (long)acc_float + (long)acc_double + v1;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *arr, int n) {
    volatile int x = 0;
    volatile int y = 0;
    
    /* Create label array for computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    for (int i = 0; i < n; i++) {
        /* Cross-iteration dependence */
        x = x * 3 + arr[i];
        
        /* Computed goto creating irreducible flow */
        goto *labels[i % 4];
        
    L0:
        y = y + x * 2;
        asm volatile ("" : "+r"(y) : : "memory");
        continue;
    L1:
        x = x ^ y;
        asm volatile ("" : "+r"(x) : : "memory");
        continue;
    L2:
        y = y - x / 2;
        asm volatile ("" : "+r"(y) : : "memory");
        continue;
    L3:
        x = x | (y << 2);
        asm volatile ("" : "+r"(x) : : "memory");
        continue;
    }
    
    global_sink += x + y;
}

int main(int argc, char *argv[]) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) * 0.1f;
        c[i] = (double)(rand() % 100) * 0.01;
        arr[i] = rand() % 100;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, n, seed);
    irreducible_flow(arr, n);
    
    /* Print result to prevent elimination */
    printf("Result: %ld\n", global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
