/* test_sel_sched.c - Test to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline,optimize("O3"))) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 42;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop - provides enough iterations */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 128; j++) {
            /* Create register pressure with many live variables */
            int idx1 = (j + seed) & 31;
            int idx2 = (j * 7 + seed) & 31;
            int idx3 = (j * 11 + i) & 31;
            int idx4 = (j * 13 + i) & 31;
            
            float fidx1 = (j & 15);
            float fidx2 = ((j + 3) & 15);
            
            /* Chain of dependent integer operations */
            int t1 = arr1[idx1] * 3;
            int t2 = arr2[idx2] + t1;
            int t3 = t2 - arr1[idx3];
            int t4 = t3 * 2 + seed;
            
            /* Mix with floating point operations */
            float ft1 = farr1[(int)fidx1] * 2.0f;
            float ft2 = ft1 + farr2[(int)fidx2];
            float ft3 = ft2 * 1.5f;
            
            /* Conditional with side effects in both branches */
            if ((t4 & 0xF) > 8) {
                /* Branch 1: complex operations */
                arr1[idx1] = t4 + (int)(ft3 * 100.0f);
                arr2[idx2] = t3 - (int)(ft1 * 50.0f);
                farr1[(int)fidx1] = ft3 * 0.75f;
                
                /* More operations to extend live ranges */
                t1 = t1 * 2 + i;
                t2 = t2 / 3 + j;
            } else {
                /* Branch 2: different operations */
                arr1[idx3] = t4 * 2 - (int)(ft2 * 200.0f);
                arr2[idx4] = t3 + (int)(ft3 * 300.0f);
                farr2[(int)fidx2] = ft2 * 1.25f;
                
                /* Different operations to challenge scheduler */
                t3 = t3 * 3 - i;
                t4 = t4 / 2 + j;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed earlier after the barrier */
            /* This extends live ranges across the asm barrier */
            int t5 = t1 + t2 + t3 + t4;
            float ft4 = ft1 + ft2 + ft3;
            
            /* More operations using extended live values */
            arr1[(j + i) & 31] += t5;
            arr2[(j * 3 + i) & 31] -= (int)(ft4 * 10.0f);
            
            /* Accumulate to prevent dead code elimination */
            sum += t5 + (int)ft4;
            
            /* Update volatile to force memory operations */
            seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Additional loop to create more scheduling complexity */
        for (k = 0; k < 16; k++) {
            int idx = (i + k) & 31;
            float fidx = (i + k) & 15;
            
            /* Cross-type operations */
            arr1[idx] += (int)(farr1[(int)fidx] * 2.0f);
            farr2[(int)fidx] += arr2[idx] * 0.01f;
            
            /* Another asm barrier */
            asm volatile("" ::: "memory");
            
            sum += arr1[idx] - (int)farr2[(int)fidx];
        }
    }
    
    /* Final computation using all arrays */
    for (i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
    }
    for (i = 0; i < 16; i++) {
        sum += (int)(farr1[i] + farr2[i]);
    }
    
    *result = sum;
}

/* Wrapper to ensure multiple calls */
static void __attribute__((noinline))
run_test(int count) 
{
    int i;
    int total = 0;
    
    for (i = 0; i < count; i++) {
        int result;
        stress_sched(10 + (i % 5), &result);
        total += result;
        
        /* Vary parameters slightly each call */
        if (i % 3 == 0) {
            stress_sched(5 + (i % 3), &result);
            total -= result;
        }
    }
    
    /* Print to prevent optimization */
    printf("Checksum: %d\n", total);
}

int main(void) 
{
    clock_t start = clock();
    
    /* Run test multiple times */
    run_test(5);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Time elapsed: %.3f seconds\n", elapsed);
    
    return 0;
}
