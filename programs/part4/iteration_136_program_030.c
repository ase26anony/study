/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_result = 0;
volatile int global_counter = 0;

/* Function with complex loop for modulo scheduling */
__attribute__((optimize("no-unroll-loops")))
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n, int seed) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = seed;
    float acc_float = (float)seed;
    int i, j;
    
    /* Cross-iteration dependencies with mixed operations */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + b[i];
        
        /* Complex control flow with switch */
        switch (i % 5) {
            case 0:
                /* Integer operations with memory access */
                acc_int += (a[i] & 0xFF) | (b[i] << 8);
                /* Inline asm to create artificial use */
                asm volatile ("" : "+r" (acc_int));
                break;
            case 1:
                /* Floating point operation with different latency */
                acc_float = acc_float * c[i] + d[i];
                /* Force register use */
                asm volatile ("" : "+r" (acc_float));
                break;
            case 2:
                /* Mixed integer/float with conversion */
                acc_int += (int)(acc_float * 100.0f);
                acc_float = (float)acc_int * 0.01f;
                /* Memory barrier effect */
                asm volatile ("" : : : "memory");
                break;
            case 3:
                /* Pointer chasing-like dependency */
                if (i > 0) {
                    a[i] += a[i-1] & 0x7F;
                }
                /* Volatile access to prevent reordering */
                local_volatile = b[i];
                break;
            case 4:
                /* Nested loop for additional pressure */
                for (j = 0; j < 3; j++) {
                    acc_int ^= (b[i] << j);
                }
                /* Explicit latency hint */
                asm volatile ("nop" : : : "memory");
                break;
        }
        
        /* Additional cross-iteration float dependency */
        if (i > 0) {
            d[i] += d[i-1] * 0.5f;
        }
        
        /* Conditional store to create memory dependencies */
        if (acc_int % 7 == 0) {
            global_counter = acc_int;
        }
    }
    
    /* Final results to prevent dead code elimination */
    global_result = acc_int + (int)acc_float;
    
    /* Force memory synchronization */
    asm volatile ("" : : : "memory");
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_loop(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int sum = 0;
    
    /* Computed goto creates irreducible flow */
    goto *labels[n % 4];
    
label0:
    for (; i < n; i++) {
        sum += arr[i] * 2;
        if (sum % 3 == 0) goto label1;
        if (sum % 5 == 0) goto label2;
        arr[i] = sum;
    }
    goto end;
    
label1:
    for (; i < n; i++) {
        sum -= arr[i] / 2;
        if (sum % 7 == 0) goto label2;
        if (sum % 11 == 0) goto label3;
        arr[i] = sum;
    }
    goto end;
    
label2:
    for (; i < n; i++) {
        sum ^= arr[i];
        if (sum % 13 == 0) goto label3;
        if (sum % 17 == 0) goto label0;
        arr[i] = sum;
    }
    goto end;
    
label3:
    for (; i < n; i++) {
        sum |= arr[i];
        if (sum % 19 == 0) goto label0;
        if (sum % 23 == 0) goto label1;
        arr[i] = sum;
    }
    
end:
    global_result ^= sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    const int N = 1000;
    int *array1, *array2;
    float *array3, *array4;
    int i;
    
    /* Allocate and initialize arrays with pattern */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (float*)malloc(N * sizeof(float));
    array4 = (float*)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(42);
    for (i = 0; i < N; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = (float)(rand() % 100) / 10.0f;
        array4[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Run modulo scheduling test multiple times */
    for (i = 0; i < 3; i++) {
        modulo_sched_stress(array1, array2, array3, array4, 
                           N - i * 10, rand() % 100);
    }
    
    /* Test with irreducible control flow */
    irreducible_loop(array1, N / 2);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    printf("Test completed. Check dump files:\n");
    printf("  - *.sms (modulo scheduling)\n");
    printf("  - *.sched2 (scheduling details)\n");
    printf("Global result: %d\n", global_result);
    
    return 0;
}
