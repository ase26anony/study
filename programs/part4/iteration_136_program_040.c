/* Test program for modulo scheduling debug output coverage */
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
    int i;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float * b[i] + d[i];
        
        /* Mixed latency operations */
        int temp_int;
        float temp_float;
        
        /* Irreducible control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic (low latency) */
                temp_int = acc_int * 3;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                a[i] = temp_int;
                break;
                
            case 1:
                /* Floating point (higher latency) */
                temp_float = acc_float * 2.5f;
                /* Memory store/load sequence */
                d[i] = temp_float;
                temp_float = d[i-1];  /* Load with potential cache miss */
                acc_float = temp_float + 1.0f;
                break;
                
            case 2:
                /* Mixed operations */
                temp_int = acc_int >> 2;
                temp_float = (float)temp_int * acc_float;
                /* Volatile access to prevent optimization */
                local_volatile = temp_int;
                b[i] = temp_float;
                break;
                
            case 3:
                /* Pointer chasing creating memory dependence */
                c[i] = c[i-1] + a[i];  /* Another distance-1 dependence */
                /* Complex expression with multiple operations */
                acc_int = (acc_int & 0xFF) | (c[i] << 8);
                break;
                
            case 4:
                /* All operations combined */
                temp_int = acc_int + a[i-1];  /* Another distance-1 use */
                temp_float = acc_float * b[i-1];
                /* Inline asm for both registers */
                asm volatile ("" : "+r" (temp_int), "+r" (temp_float));
                a[i] = temp_int;
                b[i] = temp_float;
                break;
        }
        
        /* Additional conditional inside loop */
        if (i % 7 == 0) {
            /* Nested loop to create scheduling pressure */
            int j;
            for (j = 0; j < 3; j++) {
                acc_int += j;
                asm volatile ("" : "+r" (acc_int));
            }
        }
    }
    
    /* Store results to prevent dead code elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr1, int *arr2, int n) {
    int sum1 = arr1[0];
    int sum2 = arr2[0];
    int i;
    
    /* Loop with multiple recurrences */
    for (i = 1; i < n; i++) {
        /* Multiple distance-1 dependencies */
        sum1 = sum1 * 7 + arr1[i];
        sum2 = sum2 * 13 - arr2[i];
        
        /* Use computed goto for irreducible flow */
        static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
        goto *labels[i % 4];
        
        case0:
            arr1[i] = sum1 ^ sum2;
            continue;
        case1:
            arr2[i] = sum1 + sum2;
            continue;
        case2:
            sum1 = sum1 >> 1;
            asm volatile ("" : "+r" (sum1));
            continue;
        case3:
            sum2 = sum2 << 1;
            asm volatile ("" : "+r" (sum2));
            continue;
    }
    
    global_sink += sum1 + sum2;
}

int main(int argc, char **argv) {
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
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with pattern (not constant) */
    for (int i = 0; i < n; i++) {
        a[i] = (rand() % 100) + 1;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (rand() % 100) + 1;
        d[i] = (float)(rand() % 100) / 5.0f;
        arr1[i] = rand() % 50;
        arr2[i] = rand() % 50;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    second_loop(arr1, arr2, n);
    
    /* Print something to prevent optimization */
    printf("Result: %d %f\n", global_sink, global_float_sink);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr1);
    free(arr2);
    
    return 0;
}
