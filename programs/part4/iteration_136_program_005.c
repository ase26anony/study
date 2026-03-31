/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent loop unrolling and keep dependencies */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 12345;

/* Complex loop with cross-iteration dependencies */
NO_UNROLL
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n) {
    volatile int local_volatile = global_source;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    
    /* Pointer chasing to create memory dependencies */
    int *ptr = &acc_int;
    float *fptr = &acc_float;
    
    /* Main loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Another distance-1 dependence */
        acc_float = acc_float * c[i] + d[i];
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                *ptr = (*ptr) * 3 + a[i];
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int) : : "memory");
                break;
            case 1:
                /* Floating point operations */
                *fptr = (*fptr) * 1.5f + c[i];
                /* Mix with integer */
                acc_int += (int)(*fptr);
                break;
            case 2:
                /* Memory operations with pointer chasing */
                ptr = &a[i];
                fptr = &d[i];
                /* Create data dependency */
                acc_int = *ptr + acc_int;
                break;
            case 3:
                /* Nested loop to create scheduling pressure */
                {
                    int temp = 0;
                    for (int j = 0; j < 3; j++) {
                        temp += b[i - j > 0 ? i - j : 0];
                    }
                    acc_int ^= temp;
                }
                break;
            case 4:
                /* Conditional execution path */
                if (acc_int % 7 == 0) {
                    acc_float = acc_float / 2.0f;
                } else {
                    acc_int = acc_int * 2 - 1;
                }
                /* Another asm barrier */
                asm volatile ("" : "+r" (acc_int), "+r" (acc_float) : : "memory");
                break;
        }
        
        /* Store to volatile to prevent dead code elimination */
        if (i % 13 == 0) {
            global_sink = acc_int;
        }
    }
    
    /* Final store */
    global_sink = acc_int + (int)acc_float;
}

/* Irreducible control flow using computed goto */
NO_UNROLL
void irreducible_flow(int *arr, int n) {
    volatile int counter = global_source;
    
    /* Label array for computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    int state = 0;
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[state];
        
        L0:
            arr[i] = counter * 2;
            counter = arr[i] + i;
            state = (state + 1) % 4;
            continue;
        L1:
            arr[i] = counter / 3;
            counter = arr[i] - i;
            state = (state + 2) % 4;
            continue;
        L2:
            arr[i] = counter ^ 0x55AA;
            counter = arr[i] * i;
            state = (state + 3) % 4;
            continue;
        L3:
            arr[i] = counter + 0x1000;
            counter = arr[i] % 137;
            state = (state + 1) % 4;
            continue;
    }
}

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, float *c, float *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 5) % 113;
        c[i] = (float)((i * 7) % 131) / 10.0f;
        d[i] = (float)((i * 11) % 151) / 10.0f;
    }
}

int main(int argc, char **argv) {
    /* Use command line or default size */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(a, b, c, d, n);
    
    /* Seed random for variability */
    srand(42);
    global_source = rand();
    
    /* Call the stress function */
    modulo_sched_stress(a, b, c, d, n);
    
    /* Also test irreducible control flow */
    irreducible_flow(arr, n);
    
    /* Print result to prevent optimization */
    printf("Result: %d (from global_sink)\n", global_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}
