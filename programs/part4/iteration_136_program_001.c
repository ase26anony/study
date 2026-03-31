/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to trigger modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *arr_a, int *arr_b, float *arr_f, 
                         int n, int seed) {
    volatile int vol_var = seed;
    int acc_int = seed;
    float acc_float = (float)seed;
    int i, j;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * arr_a[i] + arr_b[i];
        
        /* Artificial register pressure with inline asm */
        asm volatile ("" : "+r"(acc_int) : : "memory");
        
        /* Mixed operations with different latencies */
        switch (i % 4) {
            case 0:
                /* Integer arithmetic (low latency) */
                arr_a[i] = acc_int & 0xFF;
                /* Force memory dependency */
                vol_var = arr_a[i];
                break;
            case 1:
                /* Floating point (higher latency) */
                acc_float = acc_float * 1.01f + (float)arr_b[i];
                /* Store to memory */
                arr_f[i] = acc_float;
                break;
            case 2:
                /* Memory load with potential latency */
                acc_int = arr_b[i] + (int)(acc_float * 0.5f);
                /* Complex expression with multiple operations */
                arr_a[i] = (acc_int << 3) | (acc_int >> 5);
                break;
            case 3:
                /* Mixed integer/float with conditional */
                if (acc_int > 1000) {
                    acc_float = acc_float / 2.0f;
                } else {
                    acc_int = acc_int * 3 + 1;
                }
                /* Memory store */
                arr_b[i] = acc_int;
                break;
        }
        
        /* Additional nested loop for control flow complexity */
        for (j = 0; j < 2; j++) {
            /* Create register pressure */
            int tmp = acc_int + j;
            /* Inline asm to prevent optimization */
            asm volatile ("" : "+r"(tmp) : : );
            arr_f[i] += (float)tmp;
        }
        
        /* Pointer chasing to create memory dependencies */
        if (i > 0) {
            /* Distance-1 memory dependence */
            arr_a[i] += arr_a[i-1];
            arr_b[i] ^= arr_b[i-1];
        }
        
        /* Volatile access to prevent reordering */
        asm volatile ("" : : "r"(vol_var) : "memory");
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int + (int)acc_float;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_flow(int *data, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int state = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
label0:
    if (i >= n) goto end;
    data[i] = data[i] * 2 + 1;
    state = (state + 1) % 4;
    i++;
    goto *labels[state];
    
label1:
    if (i >= n) goto end;
    data[i] = data[i] ^ 0xAAAA;
    state = (state + 2) % 4;
    i++;
    goto *labels[state];
    
label2:
    if (i >= n) goto end;
    data[i] = data[i] >> 3;
    state = (state + 3) % 4;
    i++;
    goto *labels[state];
    
label3:
    if (i >= n) goto end;
    data[i] = data[i] << 1;
    state = (state + 1) % 4;
    i++;
    goto *labels[state];
    
end:
    return;
}

/* Main function with runtime-determined loop counts */
int main(int argc, char **argv) {
    int n = 1000;
    int seed = 42;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Allocate arrays with runtime size */
    int *arr_a = (int*)malloc(n * sizeof(int));
    int *arr_b = (int*)malloc(n * sizeof(int));
    float *arr_f = (float*)malloc(n * sizeof(float));
    
    if (!arr_a || !arr_b || !arr_f) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    srand(seed);
    for (int i = 0; i < n; i++) {
        arr_a[i] = rand() % 100;
        arr_b[i] = rand() % 100;
        arr_f[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call the stress function */
    modulo_sched_stress(arr_a, arr_b, arr_f, n, seed);
    
    /* Also test irreducible control flow */
    irreducible_flow(arr_a, n / 10);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < (n < 10 ? n : 10); i++) {
        sum += arr_a[i] + (int)arr_f[i];
    }
    printf("Result checksum: %d\n", sum + global_sink);
    
    free(arr_a);
    free(arr_b);
    free(arr_f);
    
    return 0;
}
