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
    int acc_int = *a;  /* Start with first element to create dependency */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Cross-iteration recurrence (distance-1 dependence) */
        acc_int = acc_int * a[i] + c[i];  /* Integer recurrence */
        acc_float = acc_float * b[i] + d[i];  /* Float recurrence */
        
        /* Mixed operations with different latencies */
        switch (i & 3) {  /* Complex control flow */
            case 0: {
                /* Integer arithmetic path */
                int temp = acc_int * 3;
                /* Inline asm to create artificial use */
                asm volatile ("# case0 %0" : "+r" (temp));
                a[i] = temp;
                break;
            }
            case 1: {
                /* Floating point path */
                float ftemp = acc_float * 2.5f;
                /* Memory operations */
                local_volatile = i;
                d[i] = ftemp + (float)local_volatile;
                break;
            }
            case 2: {
                /* Mixed integer/float with memory */
                int itemp = acc_int + a[i-1];  /* Another distance-1 dep */
                float ftemp = acc_float - b[i-1];
                /* Force register usage */
                asm volatile ("# case2 %0 %1" : "+r" (itemp), "+r" (ftemp));
                c[i] = itemp;
                d[i] = ftemp;
                break;
            }
            case 3: {
                /* Complex path with pointer chasing */
                int *ptr = &a[i];
                float *fptr = &b[i];
                /* Create aliasing */
                *ptr = *ptr + acc_int;
                *fptr = *fptr + acc_float;
                /* Conditional store */
                if (acc_int > 1000) {
                    c[i] = 1;
                } else {
                    c[i] = 0;
                }
                break;
            }
        }
        
        /* Additional nested loop to create scheduling pressure */
        int inner_sum = 0;
        for (int j = 0; j < 3; j++) {
            inner_sum += (i * j) & 0xFF;
        }
        /* Use the result to prevent elimination */
        asm volatile ("# innersum %0" : "+r" (inner_sum));
        
        /* Volatile access to prevent reordering */
        local_volatile = acc_int;
    }
    
    /* Store results to prevent dead code elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr, int n) {
    int sum = arr[0];
    volatile int v __attribute__((unused));
    
    /* Loop with computed goto (irreducible flow) */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    for (int i = 1; i < n; i++) {
        int idx = i & 3;
        goto *labels[idx];
        
    label0:
        sum = sum + arr[i] * 2;
        v = sum;
        goto cont;
        
    label1:
        sum = sum - arr[i];
        /* Memory barrier */
        asm volatile ("# label1" : : : "memory");
        goto cont;
        
    label2:
        sum = sum ^ (arr[i] << 1);
        v = sum;
        goto cont;
        
    label3:
        sum = sum | arr[i];
        /* Create artificial dependency chain */
        for (int k = 0; k < 2; k++) {
            sum += k;
        }
        goto cont;
        
    cont:
        /* Continue loop */
        ;
    }
    
    global_sink += sum;
}

int main(int argc, char **argv) {
    /* Use command line arg for loop count to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pattern (not constant) */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = (rand() % 100) + 1;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (rand() % 50) + 1;
        d[i] = (float)(rand() % 50) / 5.0f;
    }
    
    /* Call the stress function */
    modulo_sched_stress(a, b, c, d, n);
    
    /* Call second function */
    second_loop(a, n);
    
    /* Print result to ensure execution */
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
