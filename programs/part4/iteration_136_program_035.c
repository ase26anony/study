/* Test program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimization */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n, int seed) {
    volatile int local_volatile = seed;
    int acc_int = local_volatile;
    float acc_float = (float)local_volatile;
    
    /* Cross-iteration dependencies with different latencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: current iteration depends on previous */
        acc_int = acc_int * a[i] + c[i];  // Integer recurrence
        
        /* Mixed latency operations */
        acc_float = acc_float * b[i] + d[i];  // Float recurrence
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0: {
                /* Integer operations with memory access */
                int temp = acc_int & 0xFF;
                asm volatile ("" : "+r" (temp) : : "memory");
                a[i] = temp * 3;
                break;
            }
            case 1: {
                /* Floating point operations */
                float ftemp = acc_float * 1.5f;
                asm volatile ("" : "+f" (ftemp) : : );
                b[i] = ftemp + 2.0f;
                break;
            }
            case 2: {
                /* Mixed operations with volatile */
                int v = local_volatile;
                asm volatile ("" : "+r" (v) : : );
                acc_int = (acc_int ^ v) + i;
                break;
            }
            case 3: {
                /* Memory intensive */
                volatile int* ptr = &c[i];
                int loaded = *ptr;
                asm volatile ("" : "+r" (loaded) : : "memory");
                d[i] = (float)loaded * 0.25f;
                break;
            }
            default: {
                /* Complex computation */
                float f1 = acc_float * 0.7f;
                int i1 = acc_int % 256;
                asm volatile ("" : "+r" (i1), "+f" (f1) : : );
                acc_int = i1 + (int)f1;
                acc_float = f1 + (float)i1;
                break;
            }
        }
        
        /* Additional nested loop for pressure */
        int inner_sum = 0;
        for (int j = 0; j < 3; j++) {
            inner_sum += (i * j) & 0xF;
            asm volatile ("" : "+r" (inner_sum) : : );
        }
        acc_int ^= inner_sum;
    }
    
    /* Prevent dead code elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *arr, int n) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Computed goto creates irreducible flow */
        goto *labels[i % 4];
        
    label0:
        sum += arr[i] * 2;
        asm volatile ("" : "+r" (sum) : : );
        continue;
        
    label1:
        sum -= arr[i] / 3;
        asm volatile ("" : "+r" (sum) : : );
        continue;
        
    label2:
        sum ^= arr[i] << 1;
        asm volatile ("" : "+r" (sum) : : );
        continue;
        
    label3:
        sum |= arr[i] >> 2;
        asm volatile ("" : "+r" (sum) : : );
        continue;
    }
    
    global_sink += sum;
}

int main(int argc, char **argv) {
    /* Use runtime arguments to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = rand() % 100;
        d[i] = (float)(rand() % 100) / 5.0f;
    }
    
    /* Call the stress function */
    modulo_sched_stress(a, b, c, d, n, seed);
    
    /* Call function with irreducible flow */
    irreducible_flow(a, n);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("Result: %d %f\n", global_sink, global_float_sink);
    return 0;
}
