/* test_sel_sched.c - Test case for GCC selective scheduling RTL dumps */

#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline, optimize("O3")))
stress_sched(int iterations, int *result) {
    volatile int seed = 12345;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        arr1[i] = seed + i * 3;
        arr2[i] = seed - i * 5;
        if (i < 16) {
            farr1[i] = (seed + i) * 0.5f;
            farr2[i] = (seed - i) * 0.3f;
        }
    }
    
    int sum = 0;
    
    /* Outer loop - provides enough iterations for scheduling */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create long dependency chains with mixed operations */
            int t1 = arr1[i-1] * 3 + arr2[i+1];
            int t2 = arr1[i] ^ (arr2[i] << 2);
            float ft1 = farr1[i % 16] * 2.0f + farr2[(i+1) % 16];
            
            /* Conditional execution with side effects */
            if ((t1 + t2) % 7 > 3) {
                /* Branch 1: Integer-heavy operations */
                arr1[i] = t1 * 2 - t2;
                arr2[i] = (t2 ^ t1) + k;
                farr1[i % 16] = ft1 * 1.5f;
                
                /* Additional computation extending live ranges */
                int t3 = arr1[i] * arr2[i] / (k + 1);
                sum += t3 % 256;
            } else {
                /* Branch 2: Different operations creating scheduling pressure */
                arr1[i] = t2 * 3 + t1;
                arr2[i] = (t1 | t2) - k;
                farr2[i % 16] = ft1 * 0.75f;
                
                /* More computations with extended live ranges */
                int t4 = (arr1[i] + arr2[i]) * (k % 8 + 1);
                sum -= t4 % 128;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier in the loop */
            if (i > 10) {
                /* Cross-iteration dependencies */
                arr1[i] += arr1[i-10] % 17;
                arr2[i] ^= arr2[i-5];
                
                /* Floating-point operation mixing */
                farr1[i % 16] += farr2[(i-3) % 16] * 0.1f;
            }
            
            /* More arithmetic creating register pressure */
            int t5 = arr1[i] + arr2[i] * 3;
            int t6 = t5 ^ (sum << (i & 3));
            float ft2 = farr1[i % 16] + farr2[i % 16];
            
            /* Final store with complex addressing */
            arr1[(i + 1) % 32] = t6 + (int)(ft2 * 10.0f);
        }
        
        /* Loop-carried dependency */
        arr1[0] = sum % 1000;
        arr2[0] = k * 7;
    }
    
    *result = sum;
}

/* Helper function to create additional scheduling complexity */
static int __attribute__((noinline))
helper_func(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    int r = 0;
    
    /* Small loop with dependencies */
    for (int i = 0; i < 8; i++) {
        a = (a * 3 + b) ^ (a >> 2);
        b = (b * 5 - a) | (b << 3);
        r += a + b;
    }
    
    return r;
}

int main(void) {
    int result1, result2, result3;
    int total = 0;
    
    /* Multiple calls with different parameters */
    stress_sched(100, &result1);
    total += result1;
    
    stress_sched(50, &result2);
    total += result2;
    
    /* Call with helper to create interprocedural scheduling considerations */
    result3 = helper_func(result1, result2);
    total += result3;
    
    stress_sched(75, &result1);
    total += result1;
    
    printf("Result checksum: %d\n", total);
    
    /* Verify the result isn't optimized away */
    if (total == 0) {
        printf("Warning: Result was optimized to zero\n");
    }
    
    return 0;
}
