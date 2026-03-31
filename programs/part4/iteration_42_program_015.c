/* Test case to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int iterations) 
{
    volatile int seed = 12345;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with volatile to prevent optimization */
    for (i = 0; i < 32; i++) {
        arr1[i] = seed + i;
        arr2[i] = seed - i;
        farr1[i] = (float)(seed * i) * 0.1f;
        farr2[i] = (float)(seed / (i + 1)) * 0.01f;
    }
    
    /* Outer loop to provide iteration count */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create register pressure with mixed calculations */
            int t1 = arr1[i-1] * 3 + arr2[i+1];
            int t2 = arr1[i] ^ arr2[i-1];
            float f1 = farr1[i] * 2.5f + farr2[i-1];
            float f2 = farr1[i+1] / 1.7f - farr2[i];
            
            /* Volatile read creates scheduling barrier */
            volatile int barrier = seed;
            
            /* Conditional execution with side effects */
            if ((t1 + t2) > (barrier * 2)) {
                /* Branch 1: complex calculations */
                int t3 = t1 * t2 - arr1[i] + barrier;
                float f3 = f1 * f2 + (float)t3 * 0.01f;
                arr1[i] = t3 + (int)f3;
                farr1[i] = f3 * 0.9f + farr2[i];
                
                /* Additional dependent operations */
                arr2[i] = arr1[i-1] + arr2[i+1] * 2;
                farr2[i] = farr1[i+1] + farr2[i-1] * 1.5f;
            } else {
                /* Branch 2: different calculations */
                int t3 = t1 / (t2 ? t2 : 1) + arr2[i];
                float f3 = f2 - f1 * 0.7f + (float)t3;
                arr1[i] = t3 ^ (int)f3;
                farr1[i] = f3 / 1.3f - farr1[i-1];
                
                /* Different memory access pattern */
                arr2[i] = arr1[i+1] - arr2[i-1];
                farr2[i] = farr2[i+1] * 2.0f - farr1[i];
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Extended live range usage */
            int late_use1 = arr1[i] * 3 + arr2[i-1];
            float late_use2 = farr1[i] + farr2[i+1] * 2.0f;
            
            /* More calculations using values from much earlier */
            arr1[i+1] = late_use1 + t1;
            farr1[i-1] = late_use2 - f1;
            
            /* Cross-array dependencies */
            arr2[i] = arr1[i] + arr2[i] + late_use1;
            farr2[i] = farr1[i] * farr2[i] + late_use2;
        }
        
        /* Loop-carried dependency */
        seed = arr1[15] + arr2[20];
    }
    
    /* Final volatile write to prevent dead code elimination */
    volatile int result = arr1[0] + arr2[0] + (int)farr1[0] + (int)farr2[0];
    (void)result;
}

/* Wrapper to ensure multiple calls */
static int __attribute__((noinline))
run_scheduler_test(int count)
{
    int checksum = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Call with different iteration counts to vary behavior */
        stress_sched(50 + (i % 10));
        checksum += i * 3;
    }
    
    return checksum;
}

int main(void)
{
    int result;
    
    printf("Starting selective scheduling test...\n");
    
    /* Multiple calls to ensure scheduler runs */
    result = run_scheduler_test(100);
    
    printf("Test completed. Checksum: %d\n", result);
    
    return 0;
}
