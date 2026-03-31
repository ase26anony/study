/* modulo-sched-test.c
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 * For 32-bit: add -m32 -mtune=pentium4
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element to create dependence */
    float acc_float = *b;
    
    /* Complex control flow with computed goto */
    void* labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    for (int i = 1; i < n; i++) {
        /* Cross-iteration dependencies (distance-1) */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence: depends on previous iteration */
        acc_float = acc_float * b[i] + d[i];
        
        /* Mixed latency operations */
        int temp_int = acc_int;
        float temp_float = acc_float;
        
        /* Complex conditional execution based on i */
        int selector = i & 3;  /* 0, 1, 2, or 3 */
        
        /* Use computed goto for irreducible control flow */
        goto *labels[selector < 4 ? selector : 4];
        
    case0:
        /* Integer operations (low latency) */
        temp_int = temp_int * 7 + 13;
        /* Inline assembly to create artificial register dependencies */
        asm volatile ("" : "+r"(temp_int) : : "memory");
        break;
        
    case1:
        /* Floating point operations (higher latency) */
        temp_float = temp_float * 1.5f + 2.5f;
        /* Force memory store/load to create memory dependencies */
        local_volatile = temp_int;
        temp_int = local_volatile + i;
        break;
        
    case2:
        /* Mixed operations */
        temp_int = (temp_int & 0xFF) | (i << 8);
        temp_float = temp_float + (float)temp_int * 0.01f;
        /* Another inline assembly barrier */
        asm volatile ("" : : "r"(temp_int), "r"(temp_float) : "memory");
        break;
        
    case3:
        /* More complex operations with multiple dependencies */
        temp_int = (temp_int << 3) | (temp_int >> 29);  /* rotate */
        temp_float = temp_float / (float)(temp_int + 1);
        /* Volatile memory access */
        *(volatile int*)&a[i] = temp_int;
        temp_int = *(volatile int*)&a[i] + i;
        break;
        
    default_case:
        /* Should never reach here with selector & 3 */
        temp_int = -1;
        break;
    }
    
        /* Additional nested loop to create scheduling pressure */
        int inner_sum = 0;
        for (int j = 0; j < 2; j++) {
            inner_sum += (temp_int >> j) & 1;
        }
        temp_int += inner_sum;
        
        /* Store results to prevent elimination */
        if (i % 16 == 0) {
            global_sink = temp_int;
            global_float_sink = temp_float;
        }
    }
    
    /* Final store to global */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-unroll-loops")))
void second_loop(int *arr1, int *arr2, int n) {
    int sum1 = arr1[0];
    int sum2 = arr2[0];
    
    /* Loop with pointer chasing (strong distance-1 dependence) */
    for (int i = 1; i < n; i++) {
        /* Pointer chasing through arrays */
        sum1 = arr1[sum1 & (n-1)] + i;
        sum2 = arr2[sum2 & (n-1)] * sum1;
        
        /* Conditional with switch statement */
        switch (i % 5) {
            case 0:
                sum1 = sum1 ^ sum2;
                asm volatile ("" : "+r"(sum1) : : "memory");
                break;
            case 1:
                sum2 = sum2 + (sum1 << 2);
                break;
            case 2:
                sum1 = sum1 - (sum2 >> 1);
                /* Volatile access */
                *(volatile int*)&arr1[i] = sum1;
                break;
            case 3:
                sum2 = sum2 | 0x5555;
                asm volatile ("" : : "r"(sum2) : "memory");
                break;
            default:
                sum1 = sum1 & 0xFFFF;
                sum2 = sum2 & 0xFFFF;
                break;
        }
    }
    
    global_sink = sum1 + sum2;
}

int main(int argc, char *argv[]) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 16) n = 16;
    
    /* Allocate and initialize arrays with pattern */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    /* Simple PRNG for initialization */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = (rand() % 256) + 1;
        b[i] = (float)(rand() % 256) / 256.0f + 0.1f;
        c[i] = (rand() % 256) + 1;
        d[i] = (float)(rand() % 256) / 256.0f + 0.1f;
        arr1[i] = (rand() % n);
        arr2[i] = (rand() % n);
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    second_loop(arr1, arr2, n);
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d, %f\n", global_sink, global_float_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr1);
    free(arr2);
    
    return 0;
}
