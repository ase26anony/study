/* test_sel_sched.c - Test to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline,optimize("O3"))) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 12345; /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (k = 0; k < 32; k++) {
        arr1[k] = k * 3;
        arr2[k] = k * 7;
        farr1[k] = k * 1.5f;
        farr2[k] = k * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop for sufficient iterations */
    for (i = 0; i < iterations; i++) {
        int temp1 = seed; /* volatile read creates scheduling barrier */
        int temp2 = i * 17;
        
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 32; j++) {
            /* Chain of dependent arithmetic operations */
            int idx1 = (j + temp1) & 31;
            int idx2 = (j + temp2) & 31;
            
            /* Mixed integer and FP calculations */
            int int_calc = arr1[idx1] * 3 + arr2[idx2] * 7;
            float fp_calc = farr1[idx1] * 2.3f + farr2[idx2] * 4.7f;
            
            /* Conditional with side effects in both branches */
            if ((int_calc ^ (j * 11)) > 100) {
                /* Branch 1: complex calculations */
                arr1[j] = int_calc + (int)(fp_calc * 1.5f);
                farr1[j] = fp_calc * 0.75f + (float)int_calc;
                
                /* More dependent operations */
                int t1 = arr1[j] * 2 - arr2[(j+1)&31];
                float t2 = farr1[j] + farr2[(j+2)&31] * 1.1f;
                
                arr2[j] = t1 + (int)(t2 * 2.0f);
                farr2[j] = t2 - (float)t1 * 0.3f;
            } else {
                /* Branch 2: different calculations */
                arr1[j] = int_calc - (int)(fp_calc * 0.8f);
                farr1[j] = fp_calc * 1.25f - (float)int_calc;
                
                /* Alternative dependent operations */
                int t1 = arr1[j] / 2 + arr2[(j+3)&31];
                float t2 = farr1[j] - farr2[(j+4)&31] * 0.9f;
                
                arr2[j] = t1 - (int)(t2 * 1.5f);
                farr2[j] = t2 + (float)t1 * 0.4f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extended live ranges) */
            if (j > 4) {
                int delayed_use = arr1[j-3] + arr2[j-4] 
                                + (int)(farr1[j-2] + farr2[j-1]);
                sum += delayed_use;
                
                /* More complex dependency chain */
                float fdelayed = farr1[j-1] * 0.33f - farr2[j-3] * 0.67f;
                sum += (int)(fdelayed * 100.0f);
            }
            
            /* Additional arithmetic to increase pressure */
            int extra_calc = (arr1[j] ^ arr2[j]) * (j + 1);
            float fextra_calc = farr1[j] * farr2[j] / (j + 2.0f);
            
            sum += extra_calc + (int)(fextra_calc * 10.0f);
            
            /* Another volatile to create scheduling uncertainty */
            volatile int barrier = j;
            if (barrier & 1) {
                sum += 1;
            }
        }
        
        /* Cross-iteration dependencies */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        temp1 = seed & 255;
        
        /* Modify array elements based on seed */
        arr1[temp1 & 31] ^= sum & 0xff;
        arr2[(temp1 + 7) & 31] += sum >> 8;
    }
    
    *result = sum;
}

/* Second complex function to increase scheduling opportunities */
static void __attribute__((noinline,optimize("O3")))
another_sched_func(int *data, int size, int *out)
{
    int i, j;
    int accum = 0;
    volatile int v = 1;
    
    for (i = 0; i < size; i++) {
        int base = data[i];
        
        for (j = 0; j < 16; j++) {
            /* Complex addressing calculations */
            int idx = (i * 17 + j * 13) & 31;
            int val1 = data[idx] * 3;
            int val2 = data[(idx + 5) & 31] * 7;
            
            /* Conditional with arithmetic in both paths */
            if ((val1 + val2) > (base * 2)) {
                int t = val1 - val2;
                data[idx] = t * 2 + j;
                accum += t * 3;
                
                /* Inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Floating point intermixed */
                float ft = (float)t * 0.5f;
                accum += (int)(ft * 10.0f);
            } else {
                int t = val1 + val2;
                data[idx] = t / 2 - j;
                accum -= t * 2;
                
                float ft = (float)t * 1.5f;
                accum -= (int)(ft * 5.0f);
            }
            
            /* Extended live range usage */
            if (j > 2) {
                accum += data[idx-1] + data[idx-2];
            }
        }
        
        /* Volatile read creates scheduling boundary */
        int barrier = v;
        if (barrier) {
            accum ^= i;
        }
    }
    
    *out = accum;
}

int main(void)
{
    int result1, result2;
    int data[32];
    int i;
    
    /* Initialize data array */
    for (i = 0; i < 32; i++) {
        data[i] = i * i + i * 3 + 7;
    }
    
    /* Call both complex functions multiple times */
    for (i = 0; i < 3; i++) {
        stress_sched(100, &result1);
        another_sched_func(data, 32, &result2);
        
        /* Mix results to prevent dead code elimination */
        data[i & 31] ^= result1;
        data[(i + 16) & 31] += result2;
    }
    
    /* Final checksum */
    int checksum = 0;
    for (i = 0; i < 32; i++) {
        checksum = checksum * 31 + data[i];
    }
    checksum += result1 + result2;
    
    printf("Result1: %d, Result2: %d, Checksum: %d\n", 
           result1, result2, checksum);
    
    return 0;
}
