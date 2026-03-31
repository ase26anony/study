/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int seed = 12345;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        arr1[i] = seed + i * 3;
        arr2[i] = seed - i * 7;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = (seed + i) * 0.5f;
        farr2[i] = (seed - i) * 0.3f;
    }
    
    int sum = 0;
    /* Outer loop to provide sufficient iterations */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent integer operations creating register pressure */
            int t1 = arr1[i-1] * 3 + seed;
            int t2 = arr2[i+1] * 7 - seed;
            int t3 = t1 ^ t2;
            int t4 = t3 << (i & 3);
            int t5 = t4 - arr1[i] * 5;
            
            /* Mixed floating-point operations */
            float ft1 = farr1[i & 15] * 2.0f;
            float ft2 = farr2[i & 15] / 1.5f;
            float ft3 = ft1 + ft2 * (i & 7);
            
            /* Conditional execution with side effects */
            if ((t5 & 0xFF) > 128) {
                /* Branch 1: complex operations */
                int t6 = t5 * 11 + (i << 2);
                float ft4 = ft3 * 3.14f - (float)t6;
                arr1[i] = t6 ^ (int)ft4;
                farr1[i & 15] = ft4 * 0.9f;
                
                /* Additional dependent operations */
                int t7 = arr1[i] * 13;
                arr2[i] = t7 - (t5 >> 1);
            } else {
                /* Branch 2: different complex operations */
                int t6 = t5 * 13 - (i << 3);
                float ft4 = ft3 * 2.71f + (float)t6;
                arr2[i] = t6 | (int)ft4;
                farr2[i & 15] = ft4 * 1.1f;
                
                /* Cross-branch value usage */
                arr1[i] = t5 + (arr2[i] & 0xFF);
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of early-computed values to extend live ranges */
            int t8 = t3 + t4;  /* Use values from before conditional */
            float ft5 = ft1 * ft2;  /* Use values from before conditional */
            
            /* Complex final computation with mixed types */
            sum += arr1[i] + arr2[i] + (int)(ft5 * 100.0f) + t8;
            
            /* Additional operations to increase scheduling complexity */
            for (j = 0; j < 2; j++) {
                int idx = (i + j) & 31;
                arr1[idx] ^= sum;
                arr2[idx] += (sum >> 3);
            }
        }
        
        /* Modify seed to vary loop behavior */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    *result = sum;
}

/* Helper function to create additional scheduling context */
static int __attribute__((noinline))
process_result(int val) {
    int x = val;
    /* Create data dependencies */
    x = (x * 3) ^ 0xABCD;
    x = x + (x >> 16);
    x = x * 0x12345679;
    return x & 0xFFFF;
}

int main(void) {
    int result1, result2, result3;
    int final_result = 0;
    
    /* Multiple calls with different parameters */
    stress_sched(100, &result1);
    final_result ^= process_result(result1);
    
    stress_sched(50, &result2);
    final_result ^= process_result(result2);
    
    stress_sched(75, &result3);
    final_result ^= process_result(result3);
    
    printf("Result: %d\n", final_result);
    
    /* Additional test with different access patterns */
    {
        int temp_result;
        stress_sched(25, &temp_result);
        printf("Additional: %d\n", temp_result);
    }
    
    return 0;
}
