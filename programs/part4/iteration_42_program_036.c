/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to ensure function complexity is preserved */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    /* Local arrays to create register pressure */
    volatile int arr1[32];
    volatile int arr2[32];
    volatile float farr1[16];
    volatile float farr2[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Complex loop with high ILP potential */
    int sum = 0;
    for (int outer = 0; outer < iterations; outer++) {
        /* Inner loop with computational intensity */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int t1 = arr1[i-1] * 3 + outer;
            int t2 = arr2[i+1] * 7 - outer;
            int t3 = t1 * t2 / (i + 1);
            
            /* Floating point operations mixed in */
            float ft1 = farr1[i % 16] * 2.0f;
            float ft2 = farr2[i % 16] * 3.0f;
            float ft3 = ft1 + ft2 * (float)outer;
            
            /* Conditional with side effects in both branches */
            if ((t3 + (int)ft3) % 5 == 0) {
                /* Branch 1: complex arithmetic chain */
                int b1 = t3 * 11;
                b1 = b1 ^ (b1 >> 3);
                b1 = b1 * 13 + i;
                arr1[i] = b1;
                
                float fb1 = ft3 * 1.7f;
                fb1 = fb1 + (float)i * 0.3f;
                farr1[i % 16] = fb1;
            } else {
                /* Branch 2: different arithmetic pattern */
                int b2 = t3 * 17;
                b2 = b2 ^ (b2 << 2);
                b2 = b2 * 19 - i;
                arr2[i] = b2;
                
                float fb2 = ft3 * 2.3f;
                fb2 = fb2 - (float)i * 0.7f;
                farr2[i % 16] = fb2;
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (live range extension) */
            int late_use = t1 + t2 + (int)ft3;
            late_use = late_use * late_use;
            
            /* More computations that depend on conditional results */
            int idx = i % 8;
            int mix = arr1[idx] + arr2[idx*2];
            mix = mix ^ late_use;
            
            /* Final accumulation with complex expression */
            sum += mix * (i % 3 + 1);
            sum = sum ^ (sum << 3);
        }
        
        /* Cross-iteration dependency to prevent loop unrolling from simplifying too much */
        arr1[0] = sum % 1000;
        arr2[0] = (sum * 3) % 1000;
    }
    
    *result = sum;
}

/* Secondary function to create more scheduling context */
static void __attribute__((noinline))
helper_sched(int *data, int size, int *out)
{
    volatile int temp[16];
    int acc = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing pattern */
        int idx = (i * 7) % size;
        int val = data[idx];
        
        /* Multiple dependent operations */
        val = val * 3 + i;
        val = val ^ (val >> 2);
        val = val * 5 - idx;
        
        /* Conditional with arithmetic */
        if (val % 4 == 0) {
            temp[i % 16] = val * 11;
        } else {
            temp[i % 16] = val * 13;
        }
        
        /* Use temp values with delay */
        if (i >= 8) {
            acc += temp[(i-8) % 16] * temp[i % 16];
        }
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    *out = acc;
}

int main(void) 
{
    int result1, result2;
    int data[64];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Call stress functions multiple times */
    stress_sched(100, &result1);
    helper_sched(data, 64, &result2);
    
    /* Use results to prevent optimization */
    int final = result1 + result2;
    printf("Result: %d\n", final);
    
    /* Additional loop to increase execution time */
    volatile int check = 0;
    for (int i = 0; i < 1000; i++) {
        int tmp = i * 7;
        tmp = tmp ^ (tmp >> 1);
        tmp = tmp * 3 + i;
        check += tmp % 17;
    }
    
    printf("Check: %d\n", check);
    
    return final != 0 ? 0 : 1;
}
