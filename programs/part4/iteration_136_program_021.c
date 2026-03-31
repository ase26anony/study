/* modulo-sched-test.c
 * Test case for GCC modulo scheduler debug output coverage
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -std=c99 -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loop */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop for modulo scheduling analysis */
NO_UNROLL
void modulo_sched_stress(int *a, float *b, double *c, int n, int seed) {
    volatile int v1 = seed;  /* Force memory access */
    volatile float v2 = seed * 0.5f;
    volatile double v3 = seed * 0.25;
    
    /* Cross-iteration recurrence with distance-1 dependence */
    int acc_int = v1;
    float acc_float = v2;
    double acc_double = v3;
    
    /* Pointer chasing setup */
    int *ptr = a;
    int prev_val = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        prev_val = acc_int;
        
        /* Mixed latency operations */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic (low latency) */
                acc_int = acc_int * a[i] + b[i];  /* b[i] cast to int */
                /* Artificial register pressure */
                asm volatile ("# case0" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point (higher latency) */
                acc_float = acc_float * 1.1f + b[i % (n ? n : 1)];
                /* Memory store/load */
                c[i % (n ? n : 1)] = acc_float;
                asm volatile ("# case1" : "+r" (acc_float));
                break;
            case 2:
                /* Double precision FP */
                acc_double = acc_double * 1.01 + c[i % (n ? n : 1)];
                /* Pointer chasing with distance-1 */
                if (i > 0) {
                    *ptr = prev_val + a[i-1];  /* Distance-1 use */
                    ptr = &a[i];
                }
                asm volatile ("# case2" : "+r" (acc_double));
                break;
            case 3:
                /* Mixed operations */
                acc_int = (acc_int + (int)acc_float) * 2;
                acc_float = acc_float + (float)acc_double;
                /* Conditional with irreducible flow */
                if (i % 7 == 0) {
                    goto special_label;
                }
                asm volatile ("# case3" : "+r" (acc_int), "+r" (acc_float));
                break;
            case 4:
                /* All accumulators */
                acc_int += (int)(acc_double * 100);
                acc_float += (float)acc_int;
                acc_double += (double)acc_float;
                asm volatile ("# case4" : "+r" (acc_int), "+r" (acc_float), "+r" (acc_double));
                break;
        }
        
        /* Common label for irreducible flow */
        special_label:
        /* Nested loop for additional pressure */
        for (int j = 0; j < 2; j++) {
            volatile int temp = a[i % (n ? n : 1)] + j;
            asm volatile ("# inner" : "+r" (temp));
        }
        
        /* Recurrence with explicit distance-1 */
        if (i > 0) {
            a[i] = a[i] + a[i-1];  /* Another distance-1 dependence */
        }
    }
    
    /* Force results to be used */
    global_sink = acc_int + (int)acc_float + (int)acc_double;
    
    /* Additional irreducible control flow */
    void *labels[] = { &&label1, &&label2, &&label3 };
    int idx = acc_int % 3;
    goto *labels[idx];
    
    label1:
    asm volatile ("# label1" : : );
    return;
    
    label2:
    asm volatile ("# label2" : : );
    return;
    
    label3:
    asm volatile ("# label3" : : );
    return;
}

/* Another loop with different pattern */
NO_UNROLL
void second_loop(int *x, int *y, int n) {
    volatile int carry = 0;
    
    /* Multiple interleaved recurrences */
    for (int i = 1; i < n; i++) {
        /* Three parallel distance-1 recurrences */
        int t1 = x[i-1] + y[i];      /* Distance-1 from x */
        int t2 = y[i-1] * 3;         /* Distance-1 from y */
        int t3 = carry + t1;         /* Carried scalar */
        
        /* Complex dependence web */
        x[i] = (t1 + t2) ^ t3;
        y[i] = (t2 - t1) | carry;
        carry = t3 % 17;
        
        /* Inline asm for register pressure */
        asm volatile ("# second_loop" 
                     : "+r" (t1), "+r" (t2), "+r" (t3), "+r" (carry));
        
        /* Conditional with unpredictable branch */
        switch ((x[i] + y[i]) % 4) {
            case 0: x[i] += 1; break;
            case 1: y[i] -= 1; break;
            case 2: carry ^= 0xFF; break;
            case 3: x[i] = y[i] ^ x[i-1]; break;  /* Another distance-1 */
        }
    }
    
    global_sink += carry;
}

int main(int argc, char **argv) {
    /* Runtime-determined sizes to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Dynamic allocation prevents compile-time analysis */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    double *c = (double*)malloc(n * sizeof(double));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !x || !y) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(42);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (rand() % 100) * 0.1f;
        c[i] = (rand() % 100) * 0.01;
        x[i] = rand() % 50;
        y[i] = rand() % 50;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, n, rand());
    second_loop(x, y, n);
    
    /* Use results to prevent elimination */
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y);
    
    return 0;
}
