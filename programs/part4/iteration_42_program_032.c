/* Test program to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline, optimize("O2")))
static int stress_sched(int iterations) {
    volatile int seed = 12345;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides iteration count */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent arithmetic operations creating register pressure */
            int t1 = arr1[i-1] * seed;
            int t2 = arr2[i+1] + seed;
            float ft1 = farr1[i] * 1.732f;
            float ft2 = farr2[i] / 2.718f;
            
            /* Volatile read creates scheduling barrier */
            volatile int barrier = seed;
            
            /* Complex conditional with side effects in both branches */
            if ((t1 ^ t2) & 0xFF) {
                /* Branch 1: Integer-heavy operations */
                arr1[i] = t1 * 3 + t2 / 7;
                arr2[i] = (t1 << 3) | (t2 & 0xF);
                farr1[i] = ft1 + ft2 * 3.14159f;
                
                /* Additional dependent calculation */
                int t3 = arr1[i] * arr2[i];
                arr1[i+1] = t3 ^ (barrier * i);
            } else {
                /* Branch 2: Different arithmetic pattern */
                arr1[i] = t2 * 5 - t1 / 11;
                arr2[i] = (t2 >> 2) & (t1 | 0x7F);
                farr2[i] = ft2 - ft1 / 1.4142f;
                
                /* Different dependent calculation */
                int t4 = arr1[i] + arr2[i] * 2;
                arr2[i-1] = t4 & (barrier + i);
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of early-computed values extending live ranges */
            float ft3 = farr1[i-1] * 2.0f + farr2[i+1];
            int t5 = arr1[i-2] + arr2[i+2] * 3;
            
            /* Mix integer and float operations */
            farr1[i] = ft3 + (float)t5 * 0.25f;
            arr1[i] = t5 + (int)(ft3 * 4.0f);
            
            /* Another volatile write for scheduling complexity */
            volatile int sink = arr1[i] + arr2[i];
            (void)sink;  /* Prevent unused variable warning */
        }
        
        /* Modify seed to change pattern each outer iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Compute checksum from results */
    for (i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    return sum;
}

int main() {
    int result1, result2;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* First call with moderate iterations */
    result1 = stress_sched(100);
    printf("Result 1: %d\n", result1);
    
    /* Second call with more iterations */
    result2 = stress_sched(200);
    printf("Result 2: %d\n", result2);
    
    /* Verify results are non-zero (prevents dead code elimination) */
    if (result1 == 0 && result2 == 0) {
        printf("Unexpected zero results\n");
        return 1;
    }
    
    printf("Test completed successfully\n");
    return 0;
}
