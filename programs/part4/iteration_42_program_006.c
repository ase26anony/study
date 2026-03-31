/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 12345;  /* volatile read creates scheduling barrier */
    int i, j;
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    int counter = seed;  /* Start with volatile value */
    
    /* Outer loop to provide sufficient iterations */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent arithmetic operations */
            int t1 = arr1[i-1] * 3 + counter;
            int t2 = arr2[i+1] * 7 - counter;
            float ft1 = farr1[i%16] * 2.0f + (float)counter;
            float ft2 = farr2[i%16] * 3.0f - (float)counter;
            
            /* Create register pressure with many live values */
            int t3 = t1 * t2 + i;
            int t4 = t1 - t2 * i;
            float ft3 = ft1 * ft2 + (float)i;
            float ft4 = ft1 - ft2 * (float)i;
            
            /* Conditional execution with side effects */
            if ((t1 * t2 + (int)ft1) % 7 > 3) {
                /* Branch 1: complex operations */
                arr1[i] = t3 * 2 - t4;
                arr2[i] = t4 * 3 + t3;
                farr1[i%16] = ft3 * 1.5f;
                farr2[i%16] = ft4 * 2.5f;
                
                /* More arithmetic to extend live ranges */
                t1 = t1 + arr1[i] * 2;
                t2 = t2 - arr2[i] / 3;
            } else {
                /* Branch 2: different operations */
                arr1[i] = t4 * 5 + t3;
                arr2[i] = t3 * 11 - t4;
                farr1[i%16] = ft4 * 3.5f;
                farr2[i%16] = ft3 * 4.5f;
                
                /* Different arithmetic pattern */
                t1 = t1 - arr1[i] * 3;
                t2 = t2 + arr2[i] / 5;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extend live ranges) */
            int final1 = t1 * 2 + t3 * 3;
            int final2 = t2 * 5 - t4 * 7;
            float finalf1 = ft1 * 1.1f + ft3 * 2.2f;
            float finalf2 = ft2 * 3.3f - ft4 * 4.4f;
            
            /* More operations using extended live values */
            arr1[(i+1)%32] = final1 + final2;
            arr2[(i+2)%32] = final1 - final2;
            farr1[(i+3)%16] = finalf1 + finalf2;
            farr2[(i+4)%16] = finalf1 - finalf2;
            
            /* Update counter with complex dependency chain */
            counter = counter * 1103515245 + 12345;
            sum += arr1[i] + arr2[i] + (int)farr1[i%16] + (int)farr2[i%16];
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = arr1[31] * 2;
        arr2[0] = arr2[30] * 3;
        counter = (counter ^ sum) & 0x7FFFFFFF;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling complexity */
static void __attribute__((noinline))
helper_func(int *data, int size, int *out)
{
    int i;
    volatile int barrier = 999;
    int acc = barrier;
    
    for (i = 0; i < size; i++) {
        int idx = (i * 37) % size;
        int val = data[idx];
        
        /* Complex conditional with arithmetic */
        if ((val ^ i) & 1) {
            val = val * 3 + 1;
            acc = acc * 5 + val;
            
            /* Nested condition */
            if (val % 11 == 0) {
                val = val / 2;
                acc = acc - val * 7;
            } else {
                val = val * 2;
                acc = acc + val * 3;
            }
        } else {
            val = val / 2;
            acc = acc * 7 - val;
            
            /* Another scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        data[idx] = val;
        acc = (acc ^ val) & 0xFFFF;
    }
    
    *out = acc;
}

int main(void)
{
    int result1, result2;
    int data[64];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 13 + 7;
    }
    
    /* Call complex functions multiple times */
    stress_sched(100, &result1);
    helper_func(data, 64, &result2);
    
    /* Use results to prevent optimization */
    int final_result = result1 ^ result2;
    for (int i = 0; i < 64; i++) {
        final_result += data[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Additional calls with different parameters */
    stress_sched(50, &result1);
    helper_func(data, 64, &result2);
    final_result += result1 * 3 - result2 * 2;
    
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
