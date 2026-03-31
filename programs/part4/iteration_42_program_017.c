/* Test program to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex control flow and dependencies */
static void __attribute__((noinline, optimize("O2"))) 
stress_sched(int iterations, int *result) 
{
    volatile int barrier = 0;  /* Creates scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop - provides iteration count */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 100; j++) {
            /* Create register pressure with many live variables */
            int t1 = arr1[j % 32];
            int t2 = arr2[(j + 1) % 32];
            float ft1 = farr1[j % 16];
            float ft2 = farr2[(j + 3) % 16];
            
            /* Chain of dependent arithmetic operations */
            t1 = t1 * 3 + t2;
            t2 = t2 * 7 - t1;
            ft1 = ft1 * 2.3f + ft2;
            ft2 = ft2 * 1.7f - ft1;
            
            /* Volatile read creates scheduling barrier */
            int vol_read = barrier;
            
            /* Complex conditional with side effects in both branches */
            if ((t1 * t2 + (int)ft1) % 17 > 8) {
                /* Branch 1: Different arithmetic pattern */
                arr1[(j + vol_read) % 32] = t1 * 2 + t2;
                arr2[j % 32] = t2 * 3 - t1;
                farr1[(j / 2) % 16] = ft1 + 1.0f;
                farr2[(j / 3) % 16] = ft2 - 0.5f;
                
                /* More computations in this branch */
                int t3 = arr1[(j + 2) % 32] * 11;
                float ft3 = farr1[(j + 1) % 16] * 3.14f;
                t3 = t3 + (int)(ft3 * 100);
                arr1[(j + 5) % 32] = t3;
            } else {
                /* Branch 2: Alternative computation pattern */
                arr1[j % 32] = t1 - t2 * 4;
                arr2[(j + 2) % 32] = t2 + t1 / 3;
                farr1[(j + 1) % 16] = ft1 * 0.9f;
                farr2[(j + 2) % 16] = ft2 * 1.1f;
                
                /* Different computations in else branch */
                int t4 = arr2[(j + 3) % 32] * 13;
                float ft4 = farr2[(j + 4) % 16] * 2.71f;
                t4 = t4 - (int)(ft4 * 50);
                arr2[(j + 7) % 32] = t4;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extended live ranges) */
            int delayed_use = t1 * 2 + (int)ft1 * 3;
            delayed_use += arr1[(j + 10) % 32];  /* Use value from earlier iteration */
            
            /* More arithmetic to increase pressure */
            for (k = 0; k < 4; k++) {
                delayed_use = delayed_use * (k + 2) - arr2[(j + k) % 32];
            }
            
            sum += delayed_use;
            
            /* Volatile write creates another scheduling barrier */
            barrier = j % 7;
        }
        
        /* Cross-iteration dependencies */
        arr1[i % 32] = sum % 1000;
        arr2[(i + 3) % 32] = (sum * 3) % 1000;
    }
    
    *result = sum;
}

/* Helper function with different optimization characteristics */
static int __attribute__((noinline, optimize("O3")))
process_results(int *results, int count) 
{
    int final = 0;
    for (int i = 0; i < count; i++) {
        /* Mix of operations to encourage scheduling */
        final = (final * 31 + results[i]) % 1000000;
        if (i % 3 == 0) {
            final = final ^ (results[i] * 7);
        } else if (i % 3 == 1) {
            final = final + (results[i] / 3);
        } else {
            final = final - (results[i] % 13);
        }
    }
    return final;
}

int main(void) 
{
    int results[4];
    int i;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Call target function multiple times with different parameters */
    for (i = 0; i < 4; i++) {
        stress_sched(50 + i * 10, &results[i]);
        printf("Iteration %d: result = %d\n", i, results[i]);
    }
    
    /* Process all results to ensure all code is executed */
    int final_result = process_results(results, 4);
    printf("Final checksum: %d\n", final_result);
    
    /* Validate result to prevent dead code elimination */
    if (final_result != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
