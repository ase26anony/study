/* test_sel_sched.c
 * Test to trigger selective scheduling RTL dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    /* Local arrays to create register pressure */
    int arr1[32];
    int arr2[32];
    float farr1[16];
    float farr2[16];
    
    volatile int barrier = 0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        if (i < 16) {
            farr1[i] = i * 1.5f;
            farr2[i] = i * 2.5f;
        }
    }
    
    int sum = 0;
    
    /* Outer loop - provides enough iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent arithmetic operations */
            int t1 = arr1[i-1] * 3 + arr2[i+1];
            int t2 = arr1[i] * 7 - arr2[i-1];
            int t3 = t1 * t2 + i;
            int t4 = t3 / (arr1[i] + 1);
            
            /* Floating point operations mixed in */
            float ft1 = farr1[i % 16] * 2.0f;
            float ft2 = farr2[i % 16] * 3.0f;
            float ft3 = ft1 + ft2;
            
            /* Volatile read creates scheduling barrier */
            int vol_read = barrier;
            
            /* Conditional execution with side effects */
            if ((t3 + vol_read) % 5 == 0) {
                /* Branch 1: complex operations */
                arr1[i] = t4 * 2 + arr2[i];
                farr1[i % 16] = ft3 * 1.5f;
                
                /* More arithmetic chain */
                int t5 = arr1[i] * 11;
                int t6 = t5 - arr2[i] * 3;
                arr2[i] = t6 / 4;
            } else {
                /* Branch 2: different operations */
                arr1[i] = t4 / 2 - arr2[i];
                farr2[i % 16] = ft3 * 0.75f;
                
                /* Alternative arithmetic chain */
                int t5 = arr1[i] * 13;
                int t6 = t5 + arr2[i] * 2;
                arr2[i] = t6 % 17;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Values computed early used later - extends live ranges */
            int late_use1 = t1 * 2 + t2;
            int late_use2 = t3 * 3 - t4;
            
            /* More operations using values from both branches */
            float ft4 = farr1[i % 16] + farr2[i % 16];
            farr1[i % 16] = ft4 * 0.5f + (float)late_use1;
            farr2[i % 16] = ft4 * 1.5f - (float)late_use2;
            
            /* Final accumulation with complex expression */
            sum += arr1[i] + arr2[i] + (int)(farr1[i % 16] + farr2[i % 16]);
            
            /* Another volatile write */
            barrier = i;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = sum % 100;
        arr2[0] = sum / 100;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling complexity */
static void __attribute__((noinline,noipa))
helper_func(int *data, int size) {
    volatile int sync = 0;
    
    for (int i = 1; i < size - 1; i++) {
        /* Data-dependent chain */
        int a = data[i-1] * 3;
        int b = data[i] * 7;
        int c = data[i+1] * 11;
        
        /* Complex condition */
        if ((a + b + c) % 7 < 3) {
            data[i] = a * b - c;
            asm volatile("" ::: "memory");
        } else {
            data[i] = b * c + a;
        }
        
        /* More operations */
        data[i] = data[i] % 1023;
        sync = data[i];
    }
}

int main() {
    int result1, result2;
    int data[64];
    
    /* Initialize data array */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 13 % 31;
    }
    
    /* Call both functions multiple times */
    for (int run = 0; run < 3; run++) {
        stress_sched(100, &result1);
        helper_func(data, 64);
        
        /* Mix results */
        result2 = 0;
        for (int i = 0; i < 64; i++) {
            result2 += data[i] * (i + 1);
        }
        
        printf("Run %d: result1 = %d, result2 = %d\n", 
               run, result1, result2);
    }
    
    /* Final checksum */
    int final_sum = result1 + result2;
    printf("Final checksum: %d\n", final_sum);
    
    return final_sum != 0 ? 0 : 1;
}
