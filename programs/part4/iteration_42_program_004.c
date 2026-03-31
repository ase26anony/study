/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline))
stress_sched(int iterations) {
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int temp1 = arr1[i-1] * seed;
            int temp2 = arr2[i+1] + seed;
            int temp3 = temp1 - temp2;
            int temp4 = temp3 * (i + j);
            
            /* Floating point operations mixed in */
            float ftemp1 = farr1[i] * 1.618f;
            float ftemp2 = farr2[i] / 3.14159f;
            float ftemp3 = ftemp1 + ftemp2;
            
            /* Conditional with side effects in both branches */
            if ((temp4 & 0xF) > 8) {
                /* Branch 1: complex operations */
                arr1[i] = temp4 ^ 0xAAAA;
                farr1[i] = ftemp3 * 2.0f;
                
                /* Additional dependent operations */
                int temp5 = arr1[i] * (j % 16);
                arr2[i] = temp5 + (i << 3);
                
                /* Memory barrier via inline assembly */
                asm volatile("" ::: "memory");
                
                /* More operations after barrier */
                farr2[i] = farr1[i] * farr2[i-1];
            } else {
                /* Branch 2: different operations */
                arr1[i] = temp4 | 0x5555;
                farr1[i] = ftemp3 / 2.0f;
                
                /* Different dependency chain */
                int temp6 = arr2[i] - (j % 8);
                arr2[i] = temp6 * (i >> 1);
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
                
                /* Cross-type operations */
                farr2[i] = (float)arr1[i] + farr2[i+1];
            }
            
            /* Use values computed much earlier (extended live ranges) */
            int final_temp = temp1 + temp2;  /* Uses values from start of iteration */
            arr1[0] += final_temp % 256;     /* Write to fixed location */
            
            /* More floating point with dependencies across iterations */
            if (i > 2) {
                farr1[1] += farr2[i-2] * 0.1f;
            }
        }
        
        /* Modify seed to vary pattern */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final volatile write to ensure all operations complete */
    volatile int sink = arr1[0] + arr2[0];
    (void)sink;
}

int main() {
    int i;
    int total = 0;
    
    /* Call target function multiple times with different iteration counts */
    for (i = 0; i < 100; i++) {
        stress_sched(50 + (i % 10));
        total += i;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
