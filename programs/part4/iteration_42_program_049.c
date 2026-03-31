/* Test program to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline, optimize("O2")))
static int stress_sched(int iterations) {
    volatile int seed = 12345;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides iteration count */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            int idx1, idx2;
            float fval1, fval2;
            
            /* Chain of dependent integer operations */
            idx1 = arr1[i-1] + arr2[i+1];
            idx2 = arr1[i] * arr2[i] - seed;
            
            /* Mixed floating-point operations */
            fval1 = farr1[i % 16] * 2.0f;
            fval2 = farr2[i % 16] / 1.5f;
            
            /* Conditional with side effects in both branches */
            if ((idx1 + idx2) % 7 > 3) {
                /* Branch 1: complex operations */
                arr1[i] = (idx1 * 3 + idx2 * 5) % 1024;
                farr1[i % 16] = fval1 + fval2 * 3.14159f;
                
                /* More dependent operations */
                arr2[i] = arr1[i-1] ^ arr1[i+1];
                farr2[i % 16] = farr1[i % 16] * 0.5f - fval1;
            } else {
                /* Branch 2: different complex operations */
                arr1[i] = (idx1 * 7 - idx2 * 11) % 1023;
                farr1[i % 16] = fval2 - fval1 * 2.71828f;
                
                /* Alternative dependent operations */
                arr2[i] = arr1[i-2] | arr1[i+2];
                farr2[i % 16] = farr1[i % 16] * 1.5f + fval2;
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            int late_use1 = arr1[i-5] + arr2[i-3];  /* Values from earlier iterations */
            float late_use2 = farr1[(i-4) % 16] * farr2[(i-2) % 16];
            
            /* More operations using extended live ranges */
            arr1[i] += late_use1;
            farr1[i % 16] += late_use2;
            
            /* Another volatile read to create scheduling barrier */
            volatile int barrier = seed;
            arr2[i] ^= barrier;
            
            /* Final accumulation with complex expression */
            sum += arr1[i] + arr2[i] + (int)(farr1[i % 16] + farr2[i % 16]);
        }
        
        /* Modify seed to vary pattern */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Another complex function to increase scheduling complexity */
__attribute__((noinline))
static int helper_func(int x, int y) {
    int a = x * y;
    int b = x + y;
    int c = x - y;
    
    /* Complex conditional with multiple operations */
    if (a > b) {
        a = (a << 3) | (b >> 2);
        b = (b * 7 + c * 11) % 256;
    } else {
        a = (a >> 2) & (b << 1);
        b = (c * 13 - a * 5) % 256;
    }
    
    /* Mix with floating point */
    float fa = a * 1.5f;
    float fb = b * 0.75f;
    
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
    
    return (int)(fa + fb) + (a ^ b);
}

int main() {
    int total = 0;
    int i;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Call target function multiple times with different parameters */
    for (i = 0; i < 5; i++) {
        int result = stress_sched(100 + i * 50);
        total += result;
        
        /* Also call helper to increase scheduling opportunities */
        total += helper_func(result, i);
    }
    
    printf("Result checksum: %d\n", total);
    printf("Test completed.\n");
    
    return 0;
}
