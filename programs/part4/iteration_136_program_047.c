/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent loop unrolling and keep dependencies */
__attribute__((optimize("no-unroll-loops")))
__attribute__((noinline))
static void modulo_sched_stress(int *a, float *b, volatile int *c, 
                                volatile float *d, int n, int seed) {
    volatile int acc_int = seed;
    volatile float acc_float = (float)seed;
    int temp_int;
    float temp_float;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + i;
        
        /* Mixed latency operations */
        temp_float = b[i] * 1.5f;
        acc_float = acc_float + temp_float;
        
        /* Irreducible control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic with memory access */
                *c = acc_int;
                temp_int = *c + a[i];
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                acc_int = temp_int ^ (i * 3);
                break;
                
            case 1:
                /* Floating point with higher latency */
                temp_float = acc_float * 2.0f;
                *d = temp_float;
                acc_float = *d / 1.7f;
                /* Force register use */
                asm volatile ("" : "+r" (acc_int), "+r" (temp_float));
                break;
                
            case 2:
                /* Pointer chasing creating memory dependence */
                {
                    volatile int *ptr = c;
                    temp_int = *ptr;
                    ptr = (volatile int *)((uintptr_t)ptr ^ 0);
                    asm volatile ("" : "+r" (ptr));
                    acc_int = temp_int + acc_int;
                }
                break;
                
            case 3:
                /* Mixed operations */
                temp_int = acc_int >> 2;
                temp_float = (float)temp_int + acc_float;
                acc_float = temp_float * 0.9f;
                /* Artificial dependency chain */
                asm volatile ("" : "+r" (temp_int), "+r" (temp_float));
                break;
                
            case 4:
                /* Complex recurrence */
                acc_int = (acc_int * 1103515245 + 12345) & 0x7fffffff;
                acc_float = acc_float * 0.99f + (float)(acc_int & 0xff);
                /* Memory barrier effect */
                asm volatile ("" ::: "memory");
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        {
            int j = i & 3;
            do {
                /* Small operation with register pressure */
                temp_int = acc_int + j;
                asm volatile ("" : "+r" (temp_int));
                j--;
            } while (j > 0);
        }
        
        /* Store to volatile to prevent elimination */
        *c = acc_int;
        *d = acc_float;
    }
    
    /* Final store to ensure side effects */
    *c = acc_int;
    *d = acc_float;
}

/* Another variant with different dependency pattern */
__attribute__((optimize("no-unroll-loops")))
__attribute__((noinline))
static void modulo_sched_stress2(double *arr, int *mask, int n) {
    volatile double sum = 0.0;
    volatile double prod = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Strong distance-1 recurrence */
        double prev = sum;
        
        /* Conditional execution based on mask */
        if (mask[i] & 1) {
            sum = sum + arr[i] * 0.5;
            /* Inline asm for latency */
            asm volatile ("" : "+r" (i));
        } else {
            sum = sum - arr[i];
        }
        
        /* Second recurrence chain */
        prod = prod * (arr[i] + 1.0);
        
        /* Cross-dependency between chains */
        if (i % 7 == 0) {
            sum = sum + prod;
            prod = prod * 0.8;
        }
        
        /* Memory access with potential alias */
        arr[i-1] = prev;
        
        /* Control flow with computed goto (simulated) */
        switch (i % 3) {
            case 0: asm volatile ("" : "+r" (sum)); break;
            case 1: asm volatile ("" : "+r" (prod)); break;
            case 2: asm volatile ("" ::: "memory"); break;
        }
    }
    
    /* Ensure values are used */
    arr[0] = sum + prod;
}

int main(int argc, char *argv[]) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Dynamic allocation prevents compile-time optimization */
    int *a = (int *)malloc(n * sizeof(int));
    float *b = (float *)malloc(n * sizeof(float));
    double *arr = (double *)malloc(n * sizeof(double));
    int *mask = (int *)malloc(n * sizeof(int));
    
    volatile int c = 0;
    volatile float d = 0.0f;
    
    /* Initialize with pseudo-random pattern */
    srand(42);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        arr[i] = (double)(rand() % 100) / 5.0;
        mask[i] = rand();
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, &c, &d, n, rand());
    modulo_sched_stress2(arr, mask, n);
    
    /* Use results to prevent dead code elimination */
    printf("Result: c=%d, d=%.2f, arr[0]=%.2f\n", c, d, arr[0]);
    
    free(a);
    free(b);
    free(arr);
    free(mask);
    
    return 0;
}
