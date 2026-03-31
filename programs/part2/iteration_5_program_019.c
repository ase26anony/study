/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o trigger haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex dependencies */
#define FORCE_SCHED_BARRIER() asm volatile("" ::: "memory")

/* Function with architecture-specific scheduling hints */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static int process_block(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    int sum = 0;
    
    /* Initialize with volatile reads to prevent optimization */
    volatile int seed = 42;
    v0 = seed;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 ^ 0x55AA55AA;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch creates unpredictable control flow */
        if (__builtin_expect((arr1[i] & arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v4 = arr1[i] * v0;
            v5 = arr2[i] + v1;
            v6 = v4 ^ v5;
            v7 = v6 << (arr1[i] & 7);
            v8 = v7 - v2;
            v9 = v8 | v3;
            v10 = v9 * 11467;
            v11 = v10 ^ (v10 >> 16);
            
            /* Mix in floating point to use different functional units */
            f0 = (float)v4 * 1.41421356f;
            f1 = (float)v5 * 2.71828182f;
            f2 = f0 + f1;
            f3 = f2 * 0.57721566f;
            
            /* Memory barrier forces serialization point */
            FORCE_SCHED_BARRIER();
            
            /* Complex dependency chain */
            v12 = (int)f3 + v11;
            v13 = v12 * 1103515245 + 12345;
            v14 = v13 ^ v8;
            v15 = v14 & 0x7FFFFFFF;
            
            sum += v15;
        } else {
            /* Path B: Different instruction mix */
            v4 = arr1[i] + v0;
            v5 = arr2[i] - v1;
            v6 = v4 & v5;
            v7 = v6 >> (arr2[i] & 7);
            v8 = v7 + v2;
            v9 = v8 ^ v3;
            v10 = v9 * 16807;
            v11 = v10 ^ (v10 << 16);
            
            /* Different floating point operations */
            f4 = (float)v4 * 3.14159265f;
            f5 = (float)v5 * 1.61803398f;
            f6 = f4 - f5;
            f7 = f6 * 1.20205690f;
            
            /* Another barrier at different position */
            FORCE_SCHED_BARRIER();
            
            /* Alternative dependency chain */
            v12 = (int)f7 ^ v11;
            v13 = v12 * 1664525 + 1013904223;
            v14 = v13 | v8;
            v15 = v14 & 0x7FFFFFFF;
            
            sum -= v15;
        }
        
        /* Cross-iteration dependencies */
        v0 = v15 ^ v0;
        v1 = v14 + v1;
        v2 = v13 - v2;
        v3 = v12 | v3;
    }
    
    /* Use all variables to prevent dead code elimination */
    return sum + v0 + v1 + v2 + v3 + (int)f0 + (int)f1 + (int)f2 + (int)f3;
}

/* Complex control flow with switch and goto */
static int process_with_switch(int *arr, int size) {
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Switch creates multiple basic blocks */
        switch (arr[i] & 7) {
            case 0:
                a = arr[i] * 3;
                b = a + i;
                goto common_label;
            case 1:
                a = arr[i] + 5;
                b = a ^ i;
                goto common_label;
            case 2:
                a = arr[i] - 7;
                b = a | i;
                /* fall through */
            case 3:
                c = arr[i] * 11;
                d = c & 0xFF;
                goto merge_point;
            case 4:
                a = arr[i] << 2;
                b = a + 1;
                goto common_label;
            default:
                a = arr[i] >> 1;
                b = a - 1;
                goto common_label;
        }
        
    common_label:
        c = b * 13;
        d = c ^ 0xAA;
        
    merge_point:
        e = d + a;
        f = e * 17;
        
        /* Force scheduler to consider all paths */
        if (__builtin_expect((f & 1) == 0, 0)) {
            result += f;
        } else {
            result -= f;
        }
        
        /* Barrier in loop body */
        FORCE_SCHED_BARRIER();
    }
    
    return result + a + b + c + d + e + f;
}

int main(void) {
    const int SIZE = 256;
    int arr1[SIZE], arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1664525 + 1013904223;
        arr2[i] = (int)(seed & 0x7FFF);
    }
    
    int threshold = 5000;
    int sum1 = 0, sum2 = 0;
    
    /* Outer loop to increase scheduling complexity */
    for (int iter = 0; iter < 1000; iter++) {
        /* Process with high register pressure */
        sum1 += process_block(arr1, arr2, SIZE, threshold);
        
        /* Alternate between different processing modes */
        if (iter & 1) {
            sum2 += process_with_switch(arr1, SIZE);
        } else {
            sum2 += process_with_switch(arr2, SIZE);
        }
        
        /* Modify threshold to change branch behavior */
        threshold = (threshold + 17) & 0x3FFF;
        
        /* Prevent loop invariant code motion */
        FORCE_SCHED_BARRIER();
    }
    
    /* Final computation using all results */
    int final_result = sum1 ^ sum2;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int output = final_result;
    printf("Result: %d\n", output);
    
    return 0;
}
