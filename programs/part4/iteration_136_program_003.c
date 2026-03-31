/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile globals to prevent optimization */
volatile int global_result = 0;
volatile float global_float = 0.0f;

/* Function with complex loop for modulo scheduling */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Cross-iteration dependence variable */
    float acc_float = *b;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  /* Recurrence relation */
        
        /* Mixed latency operations */
        float temp_float = b[i] * 1.5f;
        acc_float = acc_float + temp_float;  /* FP operation with higher latency */
        
        /* Conditional execution with irreducible control flow */
        switch (i & 3) {  /* i % 4 */
            case 0:
                /* Integer operations */
                c[i] = acc_int >> 2;
                /* Inline asm to create artificial use */
                asm volatile ("# case0 %0" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point operations */
                d[i] = acc_float * 0.5f;
                /* Memory barrier-like asm */
                asm volatile ("# case1 %0" : "+r" (acc_float));
                break;
            case 2:
                /* Mixed operations */
                a[i] = acc_int + (int)acc_float;
                /* Multiple asm statements to increase complexity */
                asm volatile ("# case2a %0" : "+r" (acc_int));
                asm volatile ("# case2b %0" : "+r" (acc_float));
                break;
            case 3:
                /* Memory operations */
                local_volatile = a[i] * c[i];
                /* Complex asm with multiple constraints */
                asm volatile ("# case3 %0 %1" : "+r" (acc_int), "+r" (acc_float));
                break;
        }
        
        /* Additional nested loop to create scheduling pressure */
        int inner_sum = 0;
        for (int j = 0; j < 2; j++) {
            inner_sum += (i * j) & 0xFF;
            /* Prevent optimization of inner loop */
            asm volatile ("# inner %0" : "+r" (inner_sum));
        }
        c[i] += inner_sum;
    }
    
    /* Store results to volatile globals to prevent DCE */
    global_result = acc_int;
    global_float = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void pointer_chasing_loop(int *arr, int n) {
    volatile int chase = arr[0];
    
    /* Pointer chasing with distance-1 dependence */
    for (int i = 1; i < n; i++) {
        /* Complex addressing with recurrence */
        int idx = (chase + i) % n;
        chase = arr[idx] * chase + i;
        
        /* Conditional goto to create irreducible flow */
        if (chase & 1) {
            /* Label address computation */
            void *labels[] = { &&label1, &&label2, &&label3 };
            goto *labels[i % 3];
            
            label1:
                arr[i] = chase * 2;
                asm volatile ("# label1 %0" : "+r" (chase));
                continue;
            label2:
                arr[i] = chase / 3;
                asm volatile ("# label2 %0" : "+r" (chase));
                continue;
            label3:
                arr[i] = chase + 100;
                asm volatile ("# label3 %0" : "+r" (chase));
                continue;
        }
        
        /* Default path */
        arr[i] = chase;
    }
    
    global_result += chase;
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 10) n = 1000;
    
    /* Allocate arrays with dynamic size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    int *arr = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = (rand() % 100) + 1;
        b[i] = (rand() % 100) / 10.0f + 0.1f;
        c[i] = (rand() % 100) + 1;
        d[i] = (rand() % 100) / 10.0f + 0.1f;
        arr[i] = (rand() % 100) + 1;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n);
    pointer_chasing_loop(arr, n);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f\n", global_result, global_float);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}
