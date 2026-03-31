/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loop */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Complex loop with cross-iteration dependencies */
NO_UNROLL
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n) {
    volatile int local_volatile = global_source;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    
    /* Pointer chasing variable */
    int *ptr = &a[0];
    
    /* Main loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Another distance-1 dependence */
        acc_float = acc_float * c[i] + d[i];
        
        /* Mixed latency operations */
        int temp_int;
        float temp_float;
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer operations */
                temp_int = acc_int * 3;
                /* Inline assembly to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                a[i] = temp_int + b[i-1];  /* Distance-1 memory dependence */
                break;
                
            case 1:
                /* Floating point operations (higher latency) */
                temp_float = acc_float * 2.5f;
                /* Force register use */
                asm volatile ("" : "+r" (temp_float));
                c[i] = temp_float + d[i-1];  /* Distance-1 dependence */
                break;
                
            case 2:
                /* Memory operations with pointer chasing */
                *ptr = acc_int + i;
                ptr = &a[i % n];  /* Create aliasing */
                /* Memory barrier */
                asm volatile ("" : : "m" (*ptr));
                break;
                
            case 3:
                /* Mixed integer/float with conditional */
                if (acc_int > 1000) {
                    temp_float = (float)acc_int * 0.5f;
                    d[i] = temp_float;
                } else {
                    temp_int = acc_int / 2;
                    b[i] = temp_int;
                }
                break;
                
            case 4:
                /* Complex expression with multiple uses */
                temp_int = (acc_int * 7) / 3;
                temp_float = acc_float * 1.7f;
                /* Create artificial dependency chain */
                asm volatile ("" : "+r" (temp_int), "+r" (temp_float));
                a[i] = temp_int + (int)temp_float;
                c[i] = temp_float + (float)temp_int;
                break;
        }
        
        /* Additional nested control flow */
        if (i % 7 == 0) {
            /* Create additional pressure */
            for (int j = 0; j < 3; j++) {
                volatile int inner = acc_int + j;
                asm volatile ("" : : "r" (inner));
            }
        }
        
        /* Pointer arithmetic with distance-1 dependence */
        int *ptr2 = &b[i];
        *ptr2 = *ptr2 + acc_int;
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float;
}

/* Another loop with irreducible control flow using computed goto */
NO_UNROLL
void irreducible_loop(int *arr, int n) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    int state = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[state % 4];
        
        L0:
            sum = sum * 3 + arr[i];
            state = (state + 1) % 4;
            continue;
            
        L1:
            sum = sum / 2 - arr[i];
            state = (state * 2) % 4;
            continue;
            
        L2:
            sum = sum + arr[i] * arr[i-1];  /* Distance-1 */
            state = (state + 3) % 4;
            continue;
            
        L3:
            sum = sum ^ arr[i];
            state = (state + 2) % 4;
            continue;
    }
    
    global_sink += sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 10) n = 1000;
    
    /* Allocate arrays with volatile to prevent optimizations */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constant) */
    srand(42);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Run the stress test multiple times */
    for (int iter = 0; iter < 3; iter++) {
        modulo_sched_stress(a, b, c, d, n);
        irreducible_loop(a, n);
        
        /* Modify inputs slightly */
        for (int i = 0; i < n; i++) {
            a[i] += iter;
        }
    }
    
    /* Print result to ensure computation happens */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
