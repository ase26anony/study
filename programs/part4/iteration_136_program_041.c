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
void modulo_sched_stress(int *a, int *b, float *c, float *d, int n) {
    volatile int local_volatile __attribute__((unused));
    int acc_int = *a;  /* Start with first element */
    float acc_float = *c;
    int i, j;
    
    /* Complex loop with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        /* Cross-iteration recurrence: a[i] depends on a[i-1] */
        acc_int = acc_int * a[i] + b[i];
        
        /* Mixed latency operations */
        switch (i % 4) {
            case 0:
                /* Integer operations */
                acc_int += (a[i] & 0xFF) | (b[i] << 8);
                /* Inline asm to create artificial dependencies */
                asm volatile ("" : "+r"(acc_int));
                break;
            case 1:
                /* Floating point operations (higher latency) */
                acc_float = acc_float * c[i] + d[i];
                /* Force memory barrier */
                asm volatile ("" : : "m"(*c), "m"(*d));
                break;
            case 2:
                /* Mixed integer/float with memory access */
                acc_int = (acc_int >> 3) ^ b[i];
                acc_float = acc_float + (float)acc_int;
                /* Volatile access to prevent optimization */
                local_volatile = a[i] + b[i];
                break;
            case 3:
                /* Complex operation chain */
                acc_int = (acc_int * 3) / 2;
                acc_float = acc_float * 2.0f - 1.0f;
                /* Another asm barrier */
                asm volatile ("" : "+r"(acc_int), "+f"(acc_float));
                break;
        }
        
        /* Additional nested loop for control flow complexity */
        for (j = 0; j < 2; j++) {
            /* Conditional execution inside nested loop */
            if ((i + j) % 3 == 0) {
                acc_int ^= (1 << j);
            } else {
                acc_float += j * 0.5f;
            }
        }
        
        /* Pointer chasing to create memory dependencies */
        if (i % 5 == 0) {
            int *ptr = &a[i];
            acc_int += *ptr;
            ptr = &b[i];
            acc_int -= *ptr;
        }
    }
    
    /* Store results to volatile globals */
    global_result = acc_int;
    global_counter++;
}

/* Another function with irreducible control flow */
__attribute__((optimize("no-unroll-loops")))
void irreducible_loop(int *arr, int n) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int i = 0;
    int sum = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[i % 4];
    
label0:
    for (; i < n; ) {
        sum += arr[i++];
        if (i % 2 == 0) goto label1;
        else goto label2;
    }
    goto end;
    
label1:
    for (; i < n; ) {
        sum -= arr[i++];
        if (i % 3 == 0) goto label2;
        else goto label3;
    }
    goto end;
    
label2:
    for (; i < n; ) {
        sum ^= arr[i++];
        if (i % 5 == 0) goto label3;
        else goto label0;
    }
    goto end;
    
label3:
    for (; i < n; ) {
        sum *= arr[i++];
        if (i % 7 == 0) goto label0;
        else goto label1;
    }
    
end:
    global_result += sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    const int N = 1000;
    int *array_a, *array_b;
    float *array_c, *array_d;
    int i;
    
    /* Allocate and initialize arrays with pattern */
    array_a = (int*)malloc(N * sizeof(int));
    array_b = (int*)malloc(N * sizeof(int));
    array_c = (float*)malloc(N * sizeof(float));
    array_d = (float*)malloc(N * sizeof(float));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constant) */
    for (i = 0; i < N; i++) {
        array_a[i] = (i * 3) % 97;
        array_b[i] = (i * 5) % 101;
        array_c[i] = (float)((i * 7) % 103) / 10.0f;
        array_d[i] = (float)((i * 11) % 107) / 10.0f;
    }
    
    /* Call the stress function multiple times */
    for (i = 0; i < 3; i++) {
        modulo_sched_stress(array_a, array_b, array_c, array_d, 
                           N - (i * 10));  /* Varying size */
    }
    
    /* Test irreducible control flow */
    irreducible_loop(array_a, 50);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", global_result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
