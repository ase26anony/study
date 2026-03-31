/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable target function to stress the scheduler */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int barrier = 0;  /* Creates scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (k = 0; k < 32; k++) {
        arr1[k] = k * 3;
        arr2[k] = k * 7;
        if (k < 16) {
            farr1[k] = k * 1.5f;
            farr2[k] = k * 2.5f;
        }
    }
    
    int sum = 0;
    
    /* Outer loop - provides iteration count */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 100; j++) {
            /* Multiple dependent arithmetic operations creating register pressure */
            int idx1 = (j + i) & 31;
            int idx2 = (j * 3) & 31;
            int idx3 = (j * 5) & 31;
            
            /* Chain of integer calculations with dependencies */
            int t1 = arr1[idx1] * 3 + barrier;
            int t2 = arr2[idx2] * 7 - t1;
            int t3 = t1 * t2 + arr1[idx3];
            int t4 = t2 ^ t3 | (t1 << 2);
            int t5 = (t3 * 11) / (t4 + 1);
            
            /* Floating point calculations mixed in */
            float f1 = farr1[j & 15] * 2.0f;
            float f2 = farr2[j & 15] / 1.5f;
            float f3 = f1 + f2 * 3.14f;
            
            /* Conditional execution with side effects */
            if ((t3 + t4) & 1) {
                /* Branch 1: different arithmetic pattern */
                arr1[idx1] = t5 + (int)(f3 * 10.0f);
                arr2[idx2] = t4 - t5 * 2;
                farr1[j & 15] = f2 * 0.9f + f1;
            } else {
                /* Branch 2: alternative computation */
                arr1[idx3] = t3 ^ t5;
                arr2[idx1] = t2 + t4 * 3;
                farr2[j & 15] = f1 * 1.1f - f2;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            int t6 = t1 + t2 * 2;  /* Uses t1, t2 from start of loop */
            float f4 = f1 * f2;    /* Uses f1, f2 from middle of loop */
            
            /* More calculations using extended live values */
            arr1[(j + 5) & 31] = t6 + (int)f4;
            arr2[(j + 7) & 31] = t6 ^ (int)(f4 * 100.0f);
            
            /* Volatile read creates scheduling barrier */
            barrier = *(&barrier);
            
            /* Accumulate checksum */
            sum += t5 + (int)f3 + arr1[idx1] + arr2[idx2];
        }
    }
    
    *result = sum;
}

/* Wrapper to call multiple times */
static void __attribute__((noinline))
run_scheduler_test(void) 
{
    int results[4];
    int total = 0;
    
    /* Call with different iteration counts */
    stress_sched(50, &results[0]);
    stress_sched(100, &results[1]);
    stress_sched(25, &results[2]);
    stress_sched(75, &results[3]);
    
    /* Combine results */
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    /* Use result to prevent optimization */
    printf("Scheduler test checksum: %d\n", total);
}

int main(void) 
{
    /* Run the test multiple times */
    for (int run = 0; run < 3; run++) {
        run_scheduler_test();
    }
    
    return 0;
}
