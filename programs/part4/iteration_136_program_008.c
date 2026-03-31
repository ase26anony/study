/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;

/* Function with complex loop for modulo scheduling */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Cross-iteration dependence */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Another distance-1 dependence */
        acc_float = acc_float + b[i] * d[i];
        
        /* Mixed latency operations */
        int temp_int = acc_int >> 2;
        float temp_float = acc_float * 1.5f;
        
        /* Irreducible control flow using switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic (low latency) */
                temp_int = temp_int * 3 + 7;
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (temp_int));
                break;
            case 1:
                /* Floating point (higher latency) */
                temp_float = temp_float / 2.0f + 1.0f;
                /* Memory operation */
                local_volatile = a[i % n];
                break;
            case 2:
                /* Mixed operations */
                temp_int = temp_int ^ c[i];
                temp_float = temp_float - b[i];
                /* Another asm to force register use */
                asm volatile ("" : "+r" (temp_int), "+r" (temp_float));
                break;
            case 3:
                /* Pointer chasing creating memory dependence */
                if (i > 0) {
                    a[i] = a[i-1] + 1;  /* Another distance-1 dependence */
                }
                /* Complex expression */
                temp_int = (temp_int * 2) | (c[i] & 0xFF);
                break;
            case 4:
                /* Nested conditional */
                if (temp_int % 3 == 0) {
                    temp_float = temp_float * 3.14159f;
                } else {
                    temp_int = temp_int + 111;
                }
                /* Memory store with volatile */
                local_volatile = temp_int;
                break;
        }
        
        /* Cross-iteration store creating anti-dependence */
        if (i < n - 1) {
            c[i+1] = c[i+1] + temp_int;  /* Distance-1 anti-dependence */
        }
        
        /* Final accumulation with volatile to prevent elimination */
        global_sink += temp_int;
        float_sink += temp_float;
    }
    
    /* Ensure results are used */
    asm volatile ("" :: "r" (acc_int), "r" (acc_float));
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void recurrence_loop(int *arr1, int *arr2, int n) {
    int sum = arr1[0];
    int prod = 1;
    
    for (int i = 1; i < n; i++) {
        /* Multiple recurrences */
        sum = sum + arr1[i] * arr2[i];
        prod = prod * (arr1[i] + 1);
        
        /* Conditional with goto to create irreducible flow */
        if (i % 7 == 0) {
            goto special_case;
        }
        
        /* Normal path */
        arr1[i] = sum ^ prod;
        continue;
        
    special_case:
        /* Alternative path */
        asm volatile ("" : "+r" (sum), "+r" (prod));
        arr2[i] = prod - sum;
        
        /* Computed goto-like structure */
        static void *labels[] = { &&label1, &&label2, &&label3 };
        goto *labels[i % 3];
        
    label1:
        sum += 100;
        goto end_case;
    label2:
        prod *= 2;
        goto end_case;
    label3:
        sum -= prod;
        /* fall through */
    end_case:
        /* continue loop */
        ;
    }
    
    global_sink += sum + prod;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random size to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > 100000) n = 1000;
    }
    
    /* Initialize with pattern (not constant) */
    srand(time(NULL));
    
    int *arr1 = malloc(n * sizeof(int));
    int *arr2 = malloc(n * sizeof(int));
    float *arr3 = malloc(n * sizeof(float));
    float *arr4 = malloc(n * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = (rand() % 100) / 10.0f;
        arr4[i] = (rand() % 100) / 10.0f;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(arr1, arr3, arr2, arr4, n);
    recurrence_loop(arr1, arr2, n / 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d, %f\n", global_sink, float_sink);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
