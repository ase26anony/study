/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int trigger = 0;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop for sufficient iterations */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 100; j++) {
            /* Create register pressure with many variables */
            int t1 = arr1[j & 31];
            int t2 = arr2[j & 31];
            float ft1 = farr1[j & 31];
            float ft2 = farr2[j & 31];
            
            /* Chain of dependent arithmetic operations */
            t1 = t1 * 3 + t2;
            t2 = t2 * 5 - t1;
            ft1 = ft1 * 1.7f + ft2;
            ft2 = ft2 * 2.3f - ft1;
            
            /* Volatile read creates scheduling barrier */
            int barrier = trigger;
            
            /* More arithmetic mixing int and float */
            t1 = t1 + barrier;
            t2 = t2 - barrier;
            ft1 = ft1 + (float)barrier;
            ft2 = ft2 - (float)barrier;
            
            /* Conditional execution with side effects */
            if ((t1 + t2) > 1000) {
                /* Branch 1: complex operations */
                for (k = 0; k < 4; k++) {
                    t1 = t1 ^ (t2 << k);
                    t2 = t2 | (t1 >> (k + 1));
                    ft1 = ft1 * (1.0f + (float)k * 0.1f);
                    ft2 = ft2 / (1.0f + (float)k * 0.2f);
                }
                arr1[(j + 1) & 31] = t1;
                farr1[(j + 1) & 31] = ft1;
            } else {
                /* Branch 2: different complex operations */
                for (k = 0; k < 3; k++) {
                    t1 = t1 & (t2 + k);
                    t2 = t2 ^ (t1 - k);
                    ft1 = ft1 + (float)(k * 2);
                    ft2 = ft2 - (float)(k * 3);
                }
                arr2[(j + 2) & 31] = t2;
                farr2[(j + 2) & 31] = ft2;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (live range extension) */
            sum += t1 + t2 + (int)ft1 + (int)ft2;
            
            /* More operations to extend live ranges further */
            t1 = t1 * 2 + sum;
            t2 = t2 * 3 - sum;
            ft1 = ft1 * 1.1f + (float)sum;
            ft2 = ft2 * 0.9f - (float)sum;
            
            /* Final store using extended live values */
            arr1[j & 31] = t1 + t2;
            arr2[j & 31] = t1 - t2;
            farr1[j & 31] = ft1 + ft2;
            farr2[j & 31] = ft1 - ft2;
        }
        
        /* Modify trigger to affect scheduling decisions */
        trigger = i & 255;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling opportunities */
static void __attribute__((noinline))
secondary_sched(int *data, int size, int *out)
{
    int i, j;
    int temp[16];
    volatile int sync = 0;
    
    for (i = 0; i < size; i++) {
        /* Nested loops with data dependencies */
        for (j = 0; j < 16; j++) {
            temp[j] = data[(i + j) % size] * (j + 1);
            
            /* Complex conditional with arithmetic */
            if (temp[j] & 1) {
                temp[j] = (temp[j] << 3) | (temp[j] >> 29);
                asm volatile("" ::: "memory");
            } else {
                temp[j] = (temp[j] >> 2) ^ 0x5A5A5A5A;
            }
            
            /* Mix float operations */
            float ftemp = (float)temp[j];
            ftemp = ftemp * 0.5f + (float)sync;
            temp[j] = (int)ftemp;
        }
        
        /* Reduce to output */
        int sum = 0;
        for (j = 0; j < 16; j++) {
            sum += temp[j] * (i + j);
        }
        out[i] = sum;
        
        sync = i & 1;
    }
}

int main(void) 
{
    int result1, result2;
    int data[64];
    int out[64];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 11 + 7;
    }
    
    /* Call both complex functions */
    stress_sched(50, &result1);
    secondary_sched(data, 64, out);
    
    /* Compute checksum to prevent optimization */
    result2 = 0;
    for (int i = 0; i < 64; i++) {
        result2 += out[i];
    }
    
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to affect control flow */
    if ((result1 + result2) > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
