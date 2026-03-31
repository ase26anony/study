/* Test case to trigger selective scheduling RTL dumps in sel-sched-dump.cc */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) {
    volatile int barrier = 0;
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        if (i < 16) {
            farr1[i] = i * 1.5f;
            farr2[i] = i * 2.5f;
        }
    }
    
    int sum = 0;
    /* Outer loop to increase iteration count */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int t1 = arr1[i-1] * 3 + barrier;
            int t2 = arr2[i+1] * 7 - barrier;
            int t3 = t1 * t2 + i;
            int t4 = t3 ^ (t1 << 2);
            
            /* Floating-point operations mixed in */
            float ft1 = farr1[i % 16] * 2.0f;
            float ft2 = farr2[i % 16] / 1.5f;
            float ft3 = ft1 + ft2;
            
            /* Conditional with side effects in both branches */
            if ((t4 + (int)ft3) % 7 > 3) {
                /* Branch 1: complex operations */
                arr1[i] = t4 * 11 + (int)(ft3 * 100.0f);
                farr1[i % 16] = ft3 * 1.1f - ft1;
                
                /* More dependent operations */
                int t5 = arr1[i] ^ arr2[i];
                arr2[i] = t5 * 3 + outer;
            } else {
                /* Branch 2: different complex operations */
                arr2[i] = t4 * 13 - (int)(ft3 * 50.0f);
                farr2[i % 16] = ft3 * 0.9f + ft2;
                
                /* Cross-dependent operations */
                int t6 = arr1[i] | arr2[i];
                arr1[i] = t6 * 5 - outer;
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier with extended live ranges */
            int final1 = t3 + arr1[i] + (int)farr1[i % 16];
            int final2 = t4 + arr2[i] + (int)farr2[i % 16];
            
            /* Final computation using all values */
            sum += final1 * 3 - final2 * 2 + i + outer;
            
            /* Volatile read to create scheduling barrier */
            barrier = *(&barrier);
        }
        
        /* Modify array elements to create loop-carried dependencies */
        for (int j = 0; j < 32; j++) {
            arr1[j] = (arr1[j] + sum) & 0xFFF;
            arr2[j] = (arr2[j] ^ sum) & 0xFFF;
            if (j < 16) {
                farr1[j] = farr1[j] + (sum % 100) * 0.01f;
                farr2[j] = farr2[j] - (sum % 50) * 0.02f;
            }
        }
    }
    
    *result = sum;
}

/* Helper to prevent optimization */
static int __attribute__((noinline)) 
compute_checksum(int seed) {
    int result1 = 0, result2 = 0;
    
    /* Call with different iteration counts to create varying pressure */
    stress_sched(seed % 100 + 50, &result1);
    stress_sched(seed % 50 + 25, &result2);
    
    return result1 + result2 * 3;
}

int main(void) {
    int total = 0;
    
    /* Multiple calls with different parameters */
    for (int i = 0; i < 10; i++) {
        total += compute_checksum(i * 12345);
    }
    
    printf("Result: %d\n", total);
    
    /* Additional complex call */
    int final_result;
    stress_sched(100, &final_result);
    printf("Final: %d\n", final_result + total);
    
    return 0;
}
