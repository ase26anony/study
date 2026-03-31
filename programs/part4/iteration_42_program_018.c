/* Test program to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    /* Local arrays to create register pressure */
    volatile int arr1[32];
    int arr2[32];
    float farr1[16];
    double darr1[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        darr1[i] = i * 2.5;
    }
    
    /* Complex loop with high ILP potential */
    for (int iter = 0; iter < iterations; iter++) {
        /* Multiple dependent calculations to create long dependency chains */
        int base = iter & 31;
        float fbase = (iter & 15) * 0.7f;
        double dbase = (iter & 7) * 1.3;
        
        /* Chain of integer calculations with dependencies */
        int a = arr1[base] + 1;
        int b = a * arr2[(base + 1) & 31];
        int c = b - arr1[(base + 2) & 31];
        int d = c ^ arr2[(base + 3) & 31];
        int e = d * 7 + arr1[(base + 4) & 31];
        int f = e >> 2;
        int g = f * 11 - arr2[(base + 5) & 31];
        int h = g & 0xFF;
        
        /* Floating point calculations mixed in */
        float f1 = farr1[base & 15] * 2.0f;
        float f2 = f1 + fbase;
        float f3 = f2 * 0.5f;
        
        double d1 = darr1[base & 7] * 3.0;
        double d2 = d1 + dbase;
        double d3 = d2 / 1.7;
        
        /* Conditional execution with side effects */
        if ((h + (int)f3) > 100) {
            /* Branch 1 calculations */
            int t1 = h * 3 + (int)(f3 * 10.0f);
            int t2 = t1 ^ ((int)d3 & 0xFF);
            arr1[(base + 6) & 31] = t2;
            farr1[(base + 1) & 15] = f3 * 1.1f;
            
            /* More calculations in this branch */
            int t3 = t2 * 13;
            int t4 = t3 - (int)(d3 * 20.0);
            arr2[(base + 7) & 31] = t4;
        } else {
            /* Branch 2 calculations - different operations */
            int t1 = h / 2 + (int)(d3 * 5.0);
            int t2 = t1 | ((int)f3 & 0x7F);
            arr1[(base + 8) & 31] = t2;
            darr1[(base + 2) & 7] = d3 * 0.9;
            
            /* Additional branch-specific calculations */
            float ftmp = f3 * 0.3f;
            int t3 = t2 + (int)(ftmp * 100.0f);
            arr2[(base + 9) & 31] = t3;
        }
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Use values computed earlier after the barrier */
        int late_use1 = arr1[(base + 10) & 31] + h;
        int late_use2 = arr2[(base + 11) & 31] ^ g;
        
        /* More calculations extending live ranges */
        float flate = farr1[(base + 3) & 15] * f3;
        double dlate = darr1[(base + 4) & 7] + d3;
        
        /* Store results with complex addressing */
        arr1[(base + 12) & 31] = late_use1 * 17 + (int)(flate * 2.0f);
        arr2[(base + 13) & 31] = late_use2 / 3 ^ (int)(dlate * 3.0);
        
        /* Final mixed calculations */
        int final1 = arr1[(base + 14) & 31] + arr2[(base + 15) & 31];
        float final2 = farr1[(base + 5) & 15] * 0.77f;
        double final3 = darr1[(base + 6) & 7] / 1.33;
        
        /* Accumulate to result with volatile to prevent optimization */
        *result += final1 + (int)(final2 * 10.0f) + (int)(final3 * 5.0);
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
}

/* Wrapper function with different loop characteristics */
static void __attribute__((noinline))
complex_wrapper(int outer_iters, int inner_iters, int *total)
{
    for (int i = 0; i < outer_iters; i++) {
        int local_result = 0;
        stress_sched(inner_iters, &local_result);
        
        /* Post-process result with more calculations */
        int processed = local_result;
        processed = (processed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Conditional based on processed result */
        if ((processed & 1) == 0) {
            *total += processed >> 1;
        } else {
            *total += processed * 3 + 1;
        }
        
        /* Small delay-like calculation */
        volatile int delay = 0;
        for (int j = 0; j < 10; j++) {
            delay += j * i;
        }
    }
}

int main(void)
{
    int total_result = 0;
    
    /* Call with parameters that create scheduling pressure */
    complex_wrapper(100, 50, &total_result);
    
    /* Additional call with different parameters */
    int temp = 0;
    stress_sched(200, &temp);
    total_result += temp;
    
    printf("Result: %d\n", total_result);
    
    /* Verify result is non-zero to ensure execution */
    if (total_result == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
