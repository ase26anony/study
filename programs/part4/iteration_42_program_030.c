/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int seed = 12345;  /* volatile to prevent optimization */
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
    
    /* Outer loop for sufficient iterations */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create long dependency chain with mixed operations */
            int t1 = arr1[i-1] * 3 + seed;
            int t2 = arr2[i+1] * 7 - seed;
            float ft1 = farr1[i%16] * 2.0f;
            float ft2 = farr2[i%16] * 3.0f;
            
            /* Complex condition with side effects */
            if ((t1 ^ t2) > (i * 1000)) {
                /* Branch 1: Integer-heavy operations */
                int t3 = t1 * t2 + (i << 3);
                float ft3 = ft1 * ft2 + (i * 0.5f);
                arr1[i] = t3 >> 4;
                farr1[i%16] = ft3 * 0.9f;
                
                /* Inline assembly as scheduling barrier */
                asm volatile("" ::: "memory");
                
                /* More operations after barrier */
                arr2[i] = (arr1[i] * arr1[i-1]) / (i + 1);
            } else {
                /* Branch 2: Different operations */
                int t3 = (t1 + t2) | 0xFF;
                float ft3 = (ft1 + ft2) * 0.75f;
                arr2[i] = t3 & 0x7FFF;
                farr2[i%16] = ft3 * 1.1f;
                
                /* Another inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Cross-branch value usage */
                arr1[i] = (arr2[i] + arr2[i-1]) * 2;
            }
            
            /* Use values computed much earlier in the loop */
            int delayed_use = arr1[i-15] + arr2[i-10];
            float fdelayed = farr1[(i-8)%16] + farr2[(i-5)%16];
            
            /* Complex final calculation with mixed types */
            sum += delayed_use + (int)(fdelayed * 100.0f);
            
            /* Update seed for next iteration */
            seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Rotate arrays to create varying patterns */
        int tmp = arr1[0];
        for (j = 0; j < 31; j++) {
            arr1[j] = arr1[j+1] + j;
        }
        arr1[31] = tmp + k;
    }
    
    *result = sum;
}

/* Secondary function to create more scheduling context */
static int __attribute__((noinline))
process_results(int *data, int count) {
    int total = 0;
    volatile int barrier = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex addressing pattern */
        int idx = (i * 37) % count;
        int val = data[idx];
        
        /* Multiple dependent operations */
        val = (val * 3) + (val >> 2);
        val = val ^ (val << 4);
        val = val | 0x5555;
        
        /* Conditional with both branches having side effects */
        if (val & 1) {
            barrier = val;
            val = (val * 2) + barrier;
        } else {
            barrier = val >> 1;
            val = (val / 2) - barrier;
        }
        
        total += val;
        
        /* Memory barrier affecting scheduling */
        asm volatile("" ::: "memory");
    }
    
    return total;
}

int main(void) {
    int result1, result2;
    int intermediate[64];
    
    /* Initialize intermediate array */
    for (int i = 0; i < 64; i++) {
        intermediate[i] = i * 11;
    }
    
    /* Call scheduling-intensive function multiple times */
    stress_sched(100, &result1);
    
    /* Process results through another function */
    result2 = process_results(intermediate, 64);
    
    /* Final result to prevent optimization */
    int final_result = result1 + result2;
    
    /* Print something to ensure execution */
    printf("Result: %d (0x%08x)\n", final_result, final_result);
    
    return 0;
}
