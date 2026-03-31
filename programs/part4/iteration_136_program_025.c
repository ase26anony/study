/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile float global_float_sink = 0.0f;

/* Function with complex loop for modulo scheduling analysis */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *fa, float *fb, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = (float)seed;
    int i, j;
    
    /* Cross-iteration dependencies with mixed operations */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer operations with memory access */
                acc_int += (a[i] & 0xFF) | (b[i] << 8);
                /* Inline assembly to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
                
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * fa[i] + fb[i];
                /* Mix with integer */
                acc_int ^= (int)acc_float;
                break;
                
            case 2:
                /* Memory operations with volatile */
                local_volatile = a[i] * b[i];
                /* Recurrence with different distance pattern */
                if (i > 1) {
                    acc_int += a[i-2] - b[i-1];
                }
                break;
                
            case 3:
                /* Nested loop for additional pressure */
                for (j = 0; j < 2; j++) {
                    acc_int += (i * j) & 0xF;
                }
                /* Floating-point to integer conversion */
                acc_int += (int)(acc_float * 2.0f);
                break;
        }
        
        /* Additional cross-iteration dependence */
        if (i > 0) {
            fa[i] = fa[i-1] * 0.9f + fb[i];
        }
        
        /* Pointer chasing pattern */
        if (i % 8 == 0 && i > 0) {
            /* Create alias chain */
            int *ptr = &a[i];
            int temp = *ptr;
            asm volatile ("" : "+r" (temp));
            acc_int += temp;
        }
    }
    
    /* Store results to prevent elimination */
    global_sink = acc_int;
    global_float_sink = acc_float;
    
    /* Final volatile operation */
    asm volatile ("" : : "r" (acc_int), "r" (acc_float));
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_loop(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int sum = 0;
    
    if (n <= 0) return;
    
    /* Computed goto creates irreducible flow */
    goto *labels[i % 4];
    
label0:
    sum += arr[i] * 2;
    i++;
    if (i >= n) goto end;
    goto *labels[(i * 3) % 4];
    
label1:
    sum -= arr[i];
    i++;
    if (i >= n) goto end;
    goto *labels[(i * 5) % 4];
    
label2:
    sum ^= arr[i];
    i++;
    if (i >= n) goto end;
    goto *labels[(i * 7) % 4];
    
label3:
    sum |= arr[i] << 4;
    i++;
    if (i >= n) goto end;
    goto *labels[(i * 11) % 4];
    
end:
    global_sink += sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    const int N = 1000;
    int *array_a, *array_b;
    float *array_fa, *array_fb;
    int i, seed;
    
    /* Use command line or time-based seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Allocate and initialize arrays */
    array_a = (int*)malloc(N * sizeof(int));
    array_b = (int*)malloc(N * sizeof(int));
    array_fa = (float*)malloc(N * sizeof(float));
    array_fb = (float*)malloc(N * sizeof(float));
    
    if (!array_a || !array_b || !array_fa || !array_fb) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with random data */
    for (i = 0; i < N; i++) {
        array_a[i] = rand() % 100;
        array_b[i] = rand() % 100;
        array_fa[i] = (float)(rand() % 100) / 10.0f;
        array_fb[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Run the stress test multiple times */
    for (i = 0; i < 3; i++) {
        modulo_sched_stress(array_a, array_b, array_fa, array_fb, N, seed + i);
    }
    
    /* Also test irreducible control flow */
    irreducible_loop(array_a, N / 10);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_fa);
    free(array_fb);
    
    printf("Test completed. Check dump files:\n");
    printf("  - *.sms (modulo scheduling)\n");
    printf("  - *.sched2 (scheduling details)\n");
    printf("Look for lines with format: 'start_time end_time scheduled_time UID --(T,latency,distance)--> UID'\n");
    
    return 0;
}
