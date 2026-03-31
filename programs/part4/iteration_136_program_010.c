/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-sched2 -dA -fno-tree-vectorize -std=c99 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent loop unrolling and keep dependencies */
#define NO_UNROLL __attribute__((optimize("no-unroll-loops")))

/* Volatile variables to prevent optimization */
volatile int global_result = 0;
volatile int *volatile global_ptr = NULL;

/* Function with complex loop for modulo scheduling */
NO_UNROLL
static int modulo_sched_stress(int *a, float *b, int *c, int n) {
    volatile int acc_int = 1;
    volatile float acc_float = 1.0f;
    volatile int temp;
    int i, j;
    
    /* Cross-iteration dependencies with mixed operations */
    for (i = 0; i < n; i++) {
        /* Distance-1 dependence: acc_int depends on previous iteration */
        acc_int = acc_int * a[i] + c[i];
        
        /* Complex control flow with switch */
        switch (i % 4) {
            case 0:
                /* Integer operations with memory access */
                temp = acc_int;
                asm volatile ("" : "+r"(temp) : : "memory");
                acc_int = temp * 3;
                b[i] = b[i] + (float)acc_int;
                break;
                
            case 1:
                /* Floating point with dependency chain */
                acc_float = acc_float * 2.5f + b[i];
                asm volatile ("" : "+f"(acc_float) : : "memory");
                a[i] = (int)acc_float;
                break;
                
            case 2:
                /* Mixed operations with pointer chasing */
                if (global_ptr) {
                    temp = *global_ptr;
                    asm volatile ("" : "+r"(temp) : : "memory");
                    acc_int += temp;
                }
                c[i] = acc_int ^ i;
                break;
                
            case 3:
                /* Nested loop to create scheduling pressure */
                for (j = 0; j < 3; j++) {
                    temp = acc_int + j;
                    asm volatile ("" : "+r"(temp) : : "memory");
                    acc_int = temp;
                }
                b[i] = acc_float - (float)acc_int;
                break;
        }
        
        /* Additional recurrence with distance-1 */
        if (i > 0) {
            c[i] = c[i] + c[i-1];  /* Explicit distance-1 dependence */
        }
        
        /* Volatile memory operation to force scheduling */
        *(volatile int *)&a[i] = acc_int;
    }
    
    /* Combine results to prevent elimination */
    return acc_int + (int)acc_float;
}

/* Another function with irreducible control flow */
NO_UNROLL
static int irreducible_flow(int *arr, int n) {
    volatile int sum = 0;
    volatile int prod = 1;
    int i = 0;
    
    /* Computed goto for irreducible flow */
    void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
    
    while (i < n) {
        int idx = i % 4;
        goto *labels[idx];
        
        L0:
            sum += arr[i];
            prod *= arr[i++];
            asm volatile ("" : "+r"(sum), "+r"(prod) : : "memory");
            continue;
            
        L1:
            sum -= arr[i];
            prod = (prod + arr[i++]) ^ 0x55;
            asm volatile ("" : "+r"(sum), "+r"(prod) : : "memory");
            continue;
            
        L2:
            sum ^= arr[i];
            prod = prod | arr[i++];
            asm volatile ("" : "+r"(sum), "+r"(prod) : : "memory");
            continue;
            
        L3:
            sum = sum * arr[i];
            prod = prod - arr[i++];
            asm volatile ("" : "+r"(sum), "+r"(prod) : : "memory");
            continue;
    }
    
    return sum + prod;
}

/* Main test driver */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Allocate arrays with runtime size */
    int *a = (int *)malloc(n * sizeof(int));
    float *b = (float *)malloc(n * sizeof(float));
    int *c = (int *)malloc(n * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern (not constants) */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (float)((i * 5) % 101) * 0.1f;
        c[i] = (i * 7) % 103;
    }
    
    /* Set global pointer to create memory dependencies */
    global_ptr = &a[0];
    
    /* Call the stress function */
    int result1 = modulo_sched_stress(a, b, c, n);
    
    /* Call function with irreducible flow */
    int result2 = irreducible_flow(a, n);
    
    /* Store to volatile global to prevent elimination */
    global_result = result1 + result2;
    
    /* Print result to ensure execution */
    printf("Result: %d (check dump files for modulo scheduling output)\n", global_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
