/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n, int seed) {
    volatile int local_volatile = seed;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile * 0.5f;
    
    /* Complex loop with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i-1];  /* Recurrence with distance 1 */
        
        /* Mixed latency operations */
        float temp_float = b[i] * 1.5f;
        acc_float = acc_float + temp_float;  /* FP operation with higher latency */
        
        /* Volatile memory access to prevent optimization */
        volatile int *volatile_ptr = &c[i];
        int mem_val = *volatile_ptr;
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer arithmetic path */
                acc_int += (mem_val << 2) | 0x1;
                /* Inline assembly to create artificial use */
                asm volatile ("" : "+r" (acc_int) : : "memory");
                break;
            case 1:
                /* Floating-point path */
                acc_float = acc_float * 0.99f - (float)mem_val;
                asm volatile ("" : "+r" (i) : : "memory");
                break;
            case 2:
                /* Memory-intensive path */
                d[i] = acc_float * b[i-1];  /* Another distance-1 dependence */
                a[i] = acc_int ^ mem_val;
                break;
            case 3:
                /* Mixed operations */
                acc_int = (acc_int & 0xFF) | (mem_val << 8);
                acc_float = acc_float / (b[i] + 1.0f);
                /* Force register pressure */
                asm volatile ("# dummy asm" : : "r" (acc_int), "r" (acc_float));
                break;
            default:
                /* Complex computation with multiple dependencies */
                int t1 = acc_int * 3;
                float t2 = acc_float * 2.0f;
                c[i] = t1 + (int)t2;
                asm volatile ("" : : "r" (t1), "r" (t2) : "memory");
                break;
        }
        
        /* Additional nested control flow within loop */
        if (i % 7 == 0) {
            /* Create additional scheduling pressure */
            for (int j = 0; j < 3; j++) {
                acc_int += j * local_volatile;
                asm volatile ("# inner loop" : : "r" (j));
            }
        } else if (i % 11 == 0) {
            /* Alternative path with goto for irreducible flow */
            void *labels[] = { &&label1, &&label2, &&label3 };
            goto *labels[i % 3];
            
            label1:
                acc_int >>= 1;
                goto end_label;
            label2:
                acc_float = acc_float * acc_float;
                goto end_label;
            label3:
                acc_int = ~acc_int;
                goto end_label;
            end_label:
                asm volatile ("# computed goto" : : );
        }
        
        /* Store to volatile to maintain dependency chain */
        local_volatile = acc_int;
    }
    
    /* Final stores to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Helper to initialize arrays */
void init_arrays(int *a, float *b, int *c, float *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 37) % 101;
        b[i] = (float)((i * 19) % 53) * 0.1f;
        c[i] = (i * 73) % 127;
        d[i] = (float)((i * 29) % 71) * 0.01f;
    }
}

int main(int argc, char **argv) {
    /* Use runtime values to prevent constant propagation */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    int seed = time(NULL);
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    srand(seed);
    
    /* Allocate arrays */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_arrays(a, b, c, d, n);
    
    /* Run the stress test */
    modulo_sched_stress(a, b, c, d, n, seed);
    
    /* Print result to ensure computation happens */
    printf("Result: int=%d float=%f\n", global_sink, global_float_sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
