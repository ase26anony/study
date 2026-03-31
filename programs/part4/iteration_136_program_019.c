/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *fa, float *fb, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = (float)seed;
    int i, j;
    
    /* Cross-iteration dependencies with mixed operations */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer operations with artificial register pressure */
                {
                    int temp = acc_int & 0xFF;
                    asm volatile ("" : "+r" (temp));
                    acc_int = temp * 3 - 1;
                }
                break;
                
            case 1:
                /* Floating-point operations (higher latency) */
                acc_float = acc_float * fa[i] + fb[i];
                {
                    float ftemp = acc_float * 0.5f;
                    asm volatile ("" : "+f" (ftemp));
                    acc_float = ftemp;
                }
                break;
                
            case 2:
                /* Memory operations with volatile */
                local_volatile = a[i] * 2;
                acc_int = acc_int + local_volatile;
                /* Inline asm to create artificial dependency */
                asm volatile ("# dummy asm" : : "r" (acc_int));
                break;
                
            case 3:
                /* Mixed integer/float with control dependency */
                if (acc_int > 1000) {
                    acc_float = acc_float * 2.0f;
                } else {
                    acc_int = acc_int * 2;
                }
                /* Another asm to prevent optimization */
                asm volatile ("# case 3" : : "r" (acc_int), "f" (acc_float));
                break;
        }
        
        /* Additional nested loop for pressure */
        for (j = 0; j < 2; j++) {
            /* Small computation to create more scheduling complexity */
            int tmp = (acc_int + j) * (i % 8);
            asm volatile ("" : "+r" (tmp));
            if (j == 0) {
                acc_int += tmp;
            }
        }
        
        /* Pointer chasing style dependency */
        if (i > 0) {
            a[i] = a[i] + a[i-1] % 256;
        }
    }
    
    /* Store to global volatile to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_relation(int *arr, float *farr, int n, int init) {
    int i;
    int carry = init;
    float fcarry = (float)init;
    
    for (i = 0; i < n; i++) {
        /* Classic recurrence: a[i] = a[i-1] * b[i] + c[i] pattern */
        carry = carry * (arr[i] % 16 + 1) + (i % 32);
        
        /* Conditional with computed goto for irreducible flow */
        static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
        goto *labels[i % 4];
        
        label0:
            fcarry = fcarry * 1.1f + farr[i];
            asm volatile ("# label0" : : "f" (fcarry));
            goto cont;
        label1:
            carry = carry ^ (arr[i] << 3);
            asm volatile ("# label1" : : "r" (carry));
            goto cont;
        label2:
            fcarry = fcarry - 0.5f;
            carry = carry + (int)fcarry;
            asm volatile ("# label2" : : "r" (carry), "f" (fcarry));
            goto cont;
        label3:
            /* Memory barrier effect */
            asm volatile ("# label3" : : : "memory");
            goto cont;
        cont:
        
        /* Volatile memory access */
        volatile int *volatile_ptr = &arr[i];
        carry += *volatile_ptr;
    }
    
    global_sink += carry;
}

int main(int argc, char **argv) {
    int n = 1000;
    int seed = time(NULL);
    
    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) seed = atoi(argv[2]);
    
    srand(seed);
    
    /* Allocate and initialize arrays with runtime data */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    float *fb = (float*)malloc(n * sizeof(float));
    int *arr = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !fa || !fb || !arr || !farr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        fb[i] = (float)(rand() % 100) / 10.0f;
        arr[i] = rand() % 256;
        farr[i] = (float)(rand() % 1000) / 100.0f;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, fa, fb, n, seed);
    recurrence_relation(arr, farr, n, seed);
    
    /* Print result to ensure computation happens */
    printf("Result: %d, %f\n", global_sink, global_float_sink);
    
    /* Cleanup */
    free(a); free(b); free(fa); free(fb);
    free(arr); free(farr);
    
    return 0;
}
