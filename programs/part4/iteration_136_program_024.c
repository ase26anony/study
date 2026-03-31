/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_result = 0;
volatile int global_counter = 0;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n, int seed) {
    volatile int acc_int = seed;
    volatile float acc_float = (float)seed;
    volatile int temp;
    volatile float ftemp;
    
    /* Pointer chasing variable */
    int *ptr = a;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Cross-iteration recurrence: a[i] depends on a[i-1] */
        acc_int = acc_int * (*ptr) + b[i];
        
        /* Mixed latency operations */
        ftemp = c[i] * 1.5f + d[i];
        acc_float = acc_float + ftemp;
        
        /* Volatile memory access with artificial latency */
        asm volatile("" : "+r"(acc_int), "+r"(acc_float));
        
        /* Complex control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer multiply with memory */
                temp = a[i] * b[i];
                acc_int += temp;
                /* Memory store/load sequence */
                global_counter = temp;
                temp = global_counter + 1;
                break;
            case 1:
                /* Floating point operations */
                ftemp = c[i] * d[i];
                acc_float = acc_float * 0.99f + ftemp;
                /* Inline assembly for register pressure */
                asm volatile("" : "+r"(ftemp));
                break;
            case 2:
                /* Pointer chasing with distance-1 dependence */
                if (i > 0) {
                    temp = *(ptr - 1) + b[i];
                    acc_int ^= temp;
                }
                ptr = &a[i];
                break;
            case 3:
                /* Mixed integer/float with conditional */
                if (acc_int % 3 == 0) {
                    ftemp = (float)acc_int * 0.5f;
                    acc_float += ftemp;
                } else {
                    temp = acc_int >> 2;
                    acc_int = temp * b[i];
                }
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                temp = (a[i] + b[i]) * (acc_int % 7);
                ftemp = (c[i] - d[i]) * (acc_float * 0.1f);
                acc_int = temp + (int)ftemp;
                acc_float = ftemp + (float)temp * 0.01f;
                /* Force memory barrier */
                asm volatile("" ::: "memory");
                break;
        }
        
        /* Additional nested loop for control flow complexity */
        int j = i % 8;
        while (j > 0) {
            /* Simple operation to create loop-carried dependence */
            temp = j * 3;
            acc_int += temp;
            j--;
        }
    }
    
    /* Store results to prevent elimination */
    global_result = acc_int + (int)acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_test(int *arr, int n, int init) {
    volatile int carry = init;
    volatile int prev = init;
    
    for (int i = 0; i < n; i++) {
        /* Strong distance-1 dependence */
        int curr = arr[i] + prev;
        prev = curr;
        
        /* Use in complex expression */
        carry = carry * 13 + curr;
        
        /* Conditional with irreducible flow */
        if (i % 4 == 0) {
            /* goto-based control flow */
            if (carry > 1000) {
                carry = carry / 2;
            }
        }
        
        /* Inline assembly for hardware register use */
        asm volatile("" : "+r"(carry), "+r"(prev));
    }
    
    global_result += carry;
}

int main(int argc, char **argv) {
    /* Use command line or random size to prevent constant folding */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    srand(time(NULL));
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    float *c = (float*)malloc(n * sizeof(float));
    float *d = (float*)malloc(n * sizeof(float));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create dependencies */
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
        arr[i] = rand() % 50;
    }
    
    int seed = rand() % 100;
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n, seed);
    recurrence_test(arr, n, seed);
    
    printf("Result: %d\n", global_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}
