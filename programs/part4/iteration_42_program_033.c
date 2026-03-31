/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline,optimize("O3"))) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 12345;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
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
    
    /* Outer loop - provides enough iterations */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create register pressure with many live variables */
            int t1 = arr1[i-1] * 3;
            int t2 = arr1[i] * 5;
            int t3 = arr1[i+1] * 7;
            float ft1 = farr1[i%16] * 1.3f;
            float ft2 = farr1[(i+1)%16] * 2.3f;
            
            /* Chain of dependent operations */
            t1 = t1 + t2 * 2;
            t2 = t2 + t3 / 3;
            t3 = t3 ^ (t1 << 2);
            
            ft1 = ft1 + ft2 * 3.14159f;
            ft2 = ft2 - ft1 / 2.71828f;
            
            /* Conditional execution with side effects */
            if ((t1 + t2 + (int)ft1) % 7 < 4) {
                /* Branch 1: complex operations */
                arr2[i] = t1 * t2 + arr2[i-1];
                farr2[i%16] = ft1 * ft2 + farr2[(i-1)%16];
                
                /* More operations to extend live ranges */
                t3 = t3 ^ (arr2[i] >> 3);
                ft1 = ft1 + (float)(arr2[i] % 17);
            } else {
                /* Branch 2: different operations */
                arr2[i] = t2 * t3 - arr2[i+1];
                farr2[i%16] = ft2 * 3.0f - farr2[(i+1)%16];
                
                /* Different operations to challenge scheduler */
                t1 = t1 | (arr2[i] & 0xFF);
                ft2 = ft2 * (float)(t1 % 13);
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            int final_val = t1 + t3 + (int)(ft1 + ft2);
            final_val = final_val ^ (arr2[i] << 2);
            final_val = final_val + (int)(farr2[i%16] * 100.0f);
            
            /* More operations after the barrier */
            arr1[i] = final_val % 1000;
            farr1[i%16] = (float)(final_val % 100) / 3.0f;
            
            /* Accumulate checksum */
            sum += final_val;
            
            /* Volatile read creates scheduling barrier */
            int barrier = seed;
            arr1[i] ^= barrier;
        }
        
        /* Modify seed to change pattern */
        seed = seed * 1103515245 + 12345;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling opportunities */
static int __attribute__((noinline))
process_data(int *data, int size) 
{
    int i, j;
    int result = 0;
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex dependency chain */
        for (j = 0; j < 8; j++) {
            val = (val * 3 + 1) & 0x7FFF;
            val = val ^ (val >> 7);
            val = val * 5 - 3;
            
            /* Conditional with arithmetic */
            if (val % 11 == 0) {
                val = val + (i * j);
            } else {
                val = val - (i | j);
            }
            
            /* Another asm barrier */
            asm volatile("" ::: "memory");
        }
        
        result ^= val;
        data[i] = val;
    }
    
    return result;
}

int main(void) 
{
    int result1, result2;
    int data[64];
    
    /* Initialize data array */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Call stress function multiple times */
    stress_sched(100, &result1);
    
    /* Process data with second function */
    result2 = process_data(data, 64);
    
    /* Combine results to prevent optimization */
    int final_result = result1 ^ result2;
    
    /* Use results to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Additional check to use all data */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
