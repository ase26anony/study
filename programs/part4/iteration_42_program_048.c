/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int iterations) 
{
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
        int idx = j & 31;  /* Keep within array bounds */
        
        /* Complex inner computation with high ILP potential */
        for (i = 0; i < 32; i++) {
            /* Chain of dependent integer operations */
            int t1 = arr1[i] + seed;
            int t2 = t1 * arr2[(i + 1) & 31];
            int t3 = t2 - (arr1[(i + 2) & 31] >> 3);
            int t4 = t3 ^ (t1 & 0xFF);
            
            /* Mixed floating-point operations */
            float ft1 = farr1[i] * 1.1f;
            float ft2 = ft1 + farr2[(i + 3) & 31];
            float ft3 = ft2 / (farr1[i] + 0.5f);
            
            /* Conditional execution with side effects */
            if ((t4 & 7) > 3) {
                /* Branch 1: More integer ops */
                int t5 = t4 * 17;
                arr1[i] = t5 + (seed & 0xF);
                farr1[i] = ft3 * 2.0f;
                
                /* Additional computation in this branch */
                arr2[(i + 5) & 31] += t5 >> 4;
            } else {
                /* Branch 2: Different operations */
                int t6 = t4 / 13;
                arr1[i] = t6 - (seed & 0x7);
                farr1[i] = ft3 * 0.5f;
                
                /* Different array access pattern */
                arr2[(i + 7) & 31] ^= t6;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of values computed earlier - extends live ranges */
            if (i > 0) {
                /* Use t1, t2, ft1 computed much earlier */
                arr1[(i - 1) & 31] += (t1 + t2) & 0xFF;
                farr2[(i - 1) & 31] += ft1 * 0.25f;
            }
            
            /* More computation after the barrier */
            int t7 = arr1[i] * 3;
            float ft4 = farr1[i] + farr2[i];
            
            /* Final store with complex addressing */
            arr2[(idx + i) & 31] = t7 + (int)(ft4 * 100.0f);
            
            /* Update volatile to create scheduling barrier */
            seed = arr1[i] & 0xFF;
        }
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Ensure results are used */
    volatile int sink = arr1[0] + arr2[0];
    (void)sink;
}

/* Helper with loop-carried dependencies */
static int __attribute__((noinline))
complex_loop(int n) 
{
    int a = 1, b = 2, c = 3, d = 4;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Long dependency chain */
        int t1 = a * b + i;
        int t2 = t1 ^ c;
        int t3 = t2 - d;
        int t4 = t3 * 7;
        
        /* Conditional with both branches having side effects */
        if (t4 & 1) {
            a = t4 + b;
            c = t2 >> 3;
        } else {
            b = t4 - a;
            d = t3 << 2;
        }
        
        /* Cross-iteration dependencies */
        a = (a + b) & 0xFFF;
        b = (b ^ c) & 0xFFF;
        c = (c * d) & 0xFFF;
        d = (d - a) & 0xFFF;
    }
    
    return a + b + c + d;
}

int main(void) 
{
    int result = 0;
    int i;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Call target function multiple times */
    for (i = 0; i < 100; i++) {
        stress_sched(50);
        
        /* Also call the helper to add more scheduling complexity */
        result += complex_loop(100);
    }
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
