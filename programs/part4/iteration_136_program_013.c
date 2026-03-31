/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdlib.h>
#include <stdio.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;

/* Function with complex loop for modulo scheduling */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        acc_float = acc_float * b[i] + d[i];
        
        /* Mixed latency operations */
        int temp_int = acc_int;
        float temp_float = acc_float;
        
        /* Irreducible control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer operations */
                temp_int = temp_int * 3 + 7;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                break;
            case 1:
                /* Floating point operations (higher latency) */
                temp_float = temp_float * 1.5f + 2.0f;
                /* Memory store/load */
                local_volatile = temp_int;
                temp_int = local_volatile + 1;
                break;
            case 2:
                /* Mixed operations */
                temp_int = (temp_int << 2) | (temp_int >> 30);
                temp_float = temp_float / 2.0f;
                /* Another asm barrier */
                asm volatile ("" : "+r" (temp_int) : "r" (temp_float));
                break;
            case 3:
                /* Pointer chasing style dependency */
                if (i > 1) {
                    temp_int = a[i-1] + a[i-2];
                }
                temp_float = b[i] * temp_float;
                break;
            case 4:
                /* Complex integer math */
                temp_int = (temp_int * 1103515245 + 12345) & 0x7fffffff;
                temp_float = temp_float + (float)temp_int * 0.0001f;
                /* Force memory dependency */
                c[i] = temp_int;
                temp_int = c[i] + i;
                break;
        }
        
        /* Cross-iteration store with dependency */
        a[i] = temp_int + acc_int;
        b[i] = temp_float + acc_float;
        
        /* Additional nested loop to create pressure */
        for (int j = 0; j < 2; j++) {
            /* Simple operation that depends on outer loop */
            int nested_temp = a[i] + j;
            /* Inline asm to prevent optimization */
            asm volatile ("" : "+r" (nested_temp));
            if (j == 0) {
                c[i] += nested_temp;
            }
        }
    }
    
    /* Store results to prevent dead code elimination */
    global_sink = acc_int;
    float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr1, int *arr2, int n) {
    int sum1 = arr1[0];
    int sum2 = arr2[0];
    
    for (int i = 1; i < n; i++) {
        /* Different recurrence patterns */
        sum1 = (sum1 * 13 + arr1[i]) % 1000;
        sum2 = sum2 ^ (arr2[i] << (i % 4));
        
        /* Conditional with computed goto */
        void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
        goto *labels[i % 4];
        
        label0:
            sum1 += sum2 * 3;
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            goto end_switch;
        label1:
            sum2 ^= sum1;
            /* Force register use */
            asm volatile ("" : "+r" (sum1), "+r" (sum2));
            goto end_switch;
        label2:
            sum1 = sum1 - sum2;
            arr1[i] = sum1;
            goto end_switch;
        label3:
            sum2 = sum2 + (arr2[i] >> 2);
            /* Another memory op */
            volatile int v = sum2;
            sum2 = v + 1;
            goto end_switch;
        
        end_switch:
            /* Continue loop */
            ;
    }
    
    global_sink += sum1 + sum2;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random size to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate and initialize arrays with pattern */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 1103515245 + 12345) & 0x7fff;
        b[i] = (float)a[i] * 0.001f;
        c[i] = (i * 1664525 + 1013904223) & 0x7fff;
        d[i] = (float)c[i] * 0.002f;
        arr1[i] = a[i] ^ c[i];
        arr2[i] = a[i] + c[i];
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    second_loop(arr1, arr2, n);
    
    /* Print something to prevent optimization */
    printf("Result: %d %f\n", global_sink, float_sink);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}
