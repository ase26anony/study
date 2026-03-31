/* Test case for GCC selective scheduling RTL dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < 32; i++) {
        arr1[i] = (i * 3) ^ 0x55;
        arr2[i] = (i * 5) ^ 0xAA;
        if (i < 16) {
            farr1[i] = i * 1.5f;
            farr2[i] = i * 2.5f;
        }
    }
    
    int sum = 0;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int a = arr1[i-1] + seed;
            int b = arr2[i] * a;
            int c = b ^ (a << 3);
            int d = c - arr1[i+1];
            
            /* Mix floating-point operations */
            float fa = farr1[i % 16] * 1.1f;
            float fb = farr2[i % 16] + fa;
            
            /* Conditional with side effects in both branches */
            if ((d & 0xF) > 7) {
                /* Branch 1: complex operations */
                int e = d * 3 + (b >> 2);
                arr1[i] = e ^ 0x1234;
                farr1[i % 16] = fb * 0.9f;
                
                /* Additional dependent operations */
                int f = e + (arr2[i-1] & 0xFF);
                arr2[i] = f * 2;
            } else {
                /* Branch 2: different complex operations */
                int e = d / 2 - (c & 0xFF);
                arr1[i] = e | 0xABCD;
                farr2[i % 16] = fa / 1.3f;
                
                /* Different dependency chain */
                int f = e ^ arr2[i+1];
                arr2[i] = f + 1;
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            int late_use = a + d;
            float late_fuse = fa + fb;
            
            /* More operations after the barrier */
            int g = arr1[i] + late_use;
            arr2[(i + 1) % 32] = g ^ late_use;
            
            /* Floating-point result mixing */
            farr1[(i + 2) % 16] = late_fuse * 0.5f;
            
            /* Accumulate checksum */
            sum += g + (int)late_fuse;
        }
        
        /* Modify seed to vary loop behavior */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    *result = sum;
}

/* Secondary function to create cross-function scheduling complexity */
static int __attribute__((noinline))
helper_func(int x, int y) {
    volatile int v = x;
    int r = (v * y) ^ (x + y);
    
    /* Complex bit manipulation */
    r = (r << 3) | (r >> 29);
    r = r ^ (y * 0x5A827999);
    
    /* Another inline assembly barrier */
    asm volatile("" ::: "memory");
    
    return r * 2 - 1;
}

int main(void) {
    int result1, result2, result3;
    
    /* Call target function multiple times with different parameters */
    stress_sched(100, &result1);
    
    /* Additional computation to keep selective scheduler active */
    result2 = helper_func(result1, 42);
    
    /* Second call with different iteration count */
    stress_sched(50, &result3);
    
    /* Final result mixing */
    int final_result = result1 + result2 * 3 - result3;
    
    /* Use volatile to ensure all computations complete */
    volatile int output = final_result;
    
    printf("Result: %d\n", output);
    
    /* Return non-zero to indicate execution */
    return (output != 0) ? 0 : 1;
}
