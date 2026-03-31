/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, float *b, int *c, float *d, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = (float)seed;
    
    /* Cross-iteration dependencies with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Another distance-1 dependence with floating point */
        acc_float = acc_float * b[i] + d[i];
        
        /* Complex control flow using switch on i % 4 */
        switch (i & 3) {
            case 0: {
                /* Integer operations with memory access */
                int temp = a[i] * c[i];
                /* Inline assembly to create artificial use */
                asm volatile ("" : "+r" (temp));
                acc_int += temp;
                /* Volatile memory operation */
                local_volatile = temp;
                break;
            }
            case 1: {
                /* Floating point operations */
                float ftemp = b[i] * d[i];
                /* Force register use with inline asm */
                asm volatile ("" : "+r" (ftemp));
                acc_float += ftemp;
                /* Memory store with dependency */
                d[i] = ftemp;
                break;
            }
            case 2: {
                /* Mixed integer/float with pointer chasing */
                int idx = (acc_int & 0xFF) % (n > 1 ? n : 1);
                if (idx < n) {
                    float fval = b[idx] + d[idx];
                    acc_float = acc_float * 0.9f + fval;
                    /* Another inline asm barrier */
                    asm volatile ("" ::: "memory");
                }
                break;
            }
            case 3: {
                /* Complex recurrence with conditional */
                if (acc_int > 1000) {
                    acc_int = acc_int / 2;
                    acc_float = acc_float * 0.5f;
                } else {
                    acc_int = acc_int * 3 + 1;
                    acc_float = acc_float * 1.5f;
                }
                /* Memory load with potential aliasing */
                int mem_val = a[i % n];
                acc_int ^= mem_val;
                break;
            }
        }
        
        /* Additional nested loop to create scheduling pressure */
        for (int j = 0; j < 2; j++) {
            /* Simple operation that can be pipelined */
            int inner_temp = acc_int + j;
            asm volatile ("" : "+r" (inner_temp));
            if (j == 0) {
                acc_int = inner_temp ^ 0x55;
            }
        }
        
        /* Cross-iteration store with dependency */
        if (i > 0) {
            a[i-1] = acc_int % 256;
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
}

/* Another function with different pattern */
__attribute__((optimize("no-unroll-loops")))
void second_stress_loop(short *arr1, int *arr2, int n, int init) {
    int sum1 = init;
    int sum2 = init * 2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple interleaved recurrences */
        sum1 = sum1 + arr1[i] * (i % 8);
        sum2 = sum2 ^ (arr2[i] + sum1);
        
        /* Computed goto simulation */
        static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
        int label_idx = i & 3;
        
        goto *labels[label_idx];
        
        label0:
            sum1 = sum1 * 2 - 1;
            asm volatile ("" : "+r" (sum1));
            continue;
        label1:
            sum2 = sum2 + (sum1 >> 4);
            asm volatile ("" : "+r" (sum2));
            continue;
        label2:
            arr1[i] = (short)(sum1 & 0xFFFF);
            asm volatile ("" ::: "memory");
            continue;
        label3:
            arr2[i] = sum2;
            asm volatile ("" ::: "memory");
            continue;
    }
    
    global_sink += sum1 + sum2;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    /* Use command line or random for runtime values */
    int n = 1000;
    int seed = 12345;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Allocate arrays with runtime size */
    int *a = (int*)malloc(n * sizeof(int));
    float *b = (float*)malloc(n * sizeof(float));
    int *c = (int*)malloc(n * sizeof(int));
    float *d = (float*)malloc(n * sizeof(float));
    short *arr1 = (short*)malloc(n * sizeof(short));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(seed);
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = rand() % 100;
        d[i] = (float)(rand() % 100) / 10.0f;
        arr1[i] = (short)(rand() % 1000);
        arr2[i] = rand() % 1000;
    }
    
    /* Call the stress functions */
    modulo_sched_stress(a, b, c, d, n, seed);
    second_stress_loop(arr1, arr2, n / 2, seed);
    
    /* Print something to prevent complete optimization */
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
