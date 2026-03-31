/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline,noipa))
static int stress_sched(int iterations) {
    volatile int seed = 12345;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides iteration count */
    for (j = 0; j < iterations; j++) {
        int local_sum = 0;
        float fsum = 0.0f;
        
        /* Complex inner loop with high ILP potential */
        for (i = 0; i < 32; i++) {
            int idx1, idx2;
            float fidx1, fidx2;
            
            /* Chain of dependent integer operations */
            idx1 = (arr1[i] * seed + j) % 32;
            idx2 = (arr2[idx1] * i + seed) % 32;
            
            /* Mixed floating-point operations */
            fidx1 = farr1[i % 16] * (float)seed * 0.01f;
            fidx2 = farr2[i % 16] * (float)i * 0.02f;
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Conditional execution with side effects */
            if ((idx1 + idx2) % 3 == 0) {
                /* Branch 1: complex arithmetic chain */
                arr1[i] = arr1[idx1] * 7 - arr2[idx2] / 3;
                arr2[i] = (arr1[i] ^ arr2[i]) + seed * 2;
                farr1[i % 16] = fidx1 * 1.7f - fidx2 * 0.3f;
                
                /* More operations to extend live ranges */
                local_sum += arr1[i] * 2 - arr2[i];
                fsum += farr1[i % 16] * 2.0f;
            } else {
                /* Branch 2: different arithmetic pattern */
                arr1[i] = arr2[idx2] * 11 + arr1[idx1] / 5;
                arr2[i] = (arr1[i] | arr2[i]) - seed * 3;
                farr2[i % 16] = fidx2 * 2.3f + fidx1 * 0.7f;
                
                /* Different operations for same variables */
                local_sum += arr2[i] * 3 + arr1[i];
                fsum += farr2[i % 16] * 3.0f;
            }
            
            /* Use values computed much earlier - extends live ranges */
            if (i > 4) {
                arr1[i] += arr1[i-4] * 2;
                arr2[i] -= arr2[i-4] / 2;
            }
            
            /* Another inline assembly barrier */
            asm volatile("" ::: "memory");
            
            /* More mixed operations using extended live values */
            farr1[i % 16] += (float)arr1[i] * 0.01f;
            farr2[i % 16] -= (float)arr2[i] * 0.02f;
            
            /* Final accumulation with all computed values */
            local_sum += (int)(farr1[i % 16] + farr2[i % 16]);
        }
        
        /* Update seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        sum += local_sum + (int)fsum;
    }
    
    return sum;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline,noipa))
static int helper_func(int x, int y) {
    volatile int v = x;
    int result = 0;
    
    for (int i = 0; i < 8; i++) {
        result += (v * y) >> i;
        v = (v * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Inline assembly in helper too */
        asm volatile("" ::: "memory");
        
        if (result % 2 == 0) {
            result ^= y;
        } else {
            result |= y;
        }
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call target function multiple times */
    for (int k = 0; k < 3; k++) {
        total += stress_sched(100);
        total += helper_func(k, total & 0xFF);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
