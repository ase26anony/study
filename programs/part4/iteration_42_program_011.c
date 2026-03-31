/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity remains */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3 + seed;
        arr2[i] = i * 5 - seed;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f + seed;
        farr2[i] = i * 2.5f - seed;
    }
    
    int sum = 0;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent integer operations creating register pressure */
            int t1 = arr1[i-1] * arr2[i+1];
            int t2 = t1 + arr1[i] * 3;
            int t3 = t2 - arr2[i] * 2;
            int t4 = t3 ^ (t1 >> 3);
            int t5 = t4 * 7 + seed;
            
            /* Mix in floating-point operations */
            float ft1 = farr1[i % 16] * 2.0f;
            float ft2 = ft1 + farr2[i % 16] * 1.5f;
            float ft3 = ft2 * 0.75f;
            
            /* Conditional execution with side effects */
            if ((t5 & 0xF) > 8) {
                /* Branch 1: different arithmetic pattern */
                int t6 = t5 * 11 + (i << 2);
                arr1[i] = t6 ^ arr2[i-1];
                farr1[i % 16] = ft3 * 1.25f + i;
            } else {
                /* Branch 2: alternative computation */
                int t6 = t5 * 13 - (i << 1);
                arr1[i] = t6 | arr2[i+1];
                farr2[i % 16] = ft3 * 0.875f - i;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Extended live range usage - values computed earlier used here */
            int t7 = arr1[i] + t4;  /* t4 from much earlier */
            float ft4 = farr1[i % 16] + ft1;  /* ft1 from earlier */
            
            /* More complex computation mixing int and float */
            int t8 = t7 * 3 + (int)(ft4 * 100.0f);
            
            /* Store to array with index calculation */
            int idx = (t8 & 0x1F);
            arr2[idx] = t8 ^ seed;
            
            /* Accumulate checksum */
            sum += t8 + (int)ft4;
            
            /* Update volatile to create scheduling barrier */
            seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling opportunities */
static void __attribute__((noinline))
secondary_sched(int n, int *output)
{
    int buffer[64];
    volatile int barrier = 1;
    
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * i - i;
    }
    
    int acc = 0;
    for (int i = 0; i < n; i++) {
        /* Complex addressing patterns */
        int idx1 = (i * 17) & 0x3F;
        int idx2 = (i * 23) & 0x3F;
        int idx3 = (i * 37) & 0x3F;
        
        int val1 = buffer[idx1] * 3;
        int val2 = buffer[idx2] * 5;
        int val3 = buffer[idx3] * 7;
        
        /* Nested conditionals */
        if (val1 > val2) {
            if (val3 > 0) {
                buffer[idx1] = val1 + val3;
            } else {
                buffer[idx1] = val1 - val3;
            }
        } else {
            buffer[idx2] = val2 ^ val3;
        }
        
        /* Another asm barrier */
        asm volatile("" ::: "memory");
        
        acc += buffer[i & 0x3F] + barrier;
        barrier = barrier * 3 + 1;
    }
    
    *output = acc;
}

int main(void) 
{
    int result1, result2;
    int total = 0;
    
    /* Call stress functions multiple times */
    for (int run = 0; run < 3; run++) {
        stress_sched(100 + run * 50, &result1);
        secondary_sched(200 + run * 30, &result2);
        total += result1 + result2;
    }
    
    printf("Result checksum: %d\n", total);
    
    /* Additional test with different parameters */
    int temp;
    stress_sched(50, &temp);
    printf("Additional run: %d\n", temp);
    
    return 0;
}
