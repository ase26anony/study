/* haifa_sched_trigger.c
 * Program designed to trigger GCC Haifa scheduler state save/restore
 * and execute the uncovered cleanup code in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force complex scheduling decisions with architecture-specific tuning */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int *volatile_ptr = arr1; /* Prevent optimizations */
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = *volatile_ptr;
    v1 = v0 ^ 0x55555555;
    v2 = v1 + 1;
    v3 = v2 * 3;
    v4 = v3 >> 1;
    v5 = v4 | 0xAAAAAAAA;
    v6 = v5 & 0x33333333;
    v7 = v6 - v0;
    v8 = v7 ^ v1;
    v9 = v8 * 7;
    v10 = v9 / 2;
    v11 = v10 + v2;
    v12 = v11 ^ v3;
    v13 = v12 | v4;
    v14 = v13 & v5;
    v15 = v14 - v6;
    
    f0 = (float)v0 * 0.5f;
    f1 = f0 + 1.0f;
    f2 = f1 * 2.0f;
    f3 = f2 - 0.5f;
    f4 = f3 / 3.0f;
    f5 = f4 + f0;
    f6 = f5 * f1;
    f7 = f6 - f2;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with __builtin_expect for scheduler pressure */
        int branch_cond = arr1[i] & 0x7F; /* Use lower 7 bits */
        
        if (__builtin_expect((branch_cond > 64), 0)) {
            /* Path A: Integer-heavy operations */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * (arr1[i] | 1); /* Ensure non-zero */
            v3 = v3 ^ arr2[i];
            v4 = v4 & arr1[i];
            v5 = v5 | arr2[i];
            v6 = v6 + (arr1[i] << 2);
            v7 = v7 - (arr2[i] >> 1);
            v8 = v8 * (v0 & 0xFF);
            v9 = v9 ^ (v1 & 0xFF);
            v10 = v10 + (v2 & 0xFF);
            v11 = v11 - (v3 & 0xFF);
            v12 = v12 * (v4 | 1);
            v13 = v13 ^ v5;
            v14 = v14 + v6;
            v15 = v15 - v7;
            
            /* Floating point operations mixed in */
            f0 = f0 + (float)arr1[i];
            f1 = f1 - (float)arr2[i];
            f2 = f2 * (float)(arr1[i] | 1);
            f3 = f3 / (float)((arr2[i] & 0x7F) + 1);
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v0 = v0 ^ v8;
            v1 = v1 + v9;
            f4 = f4 * f0;
            f5 = f5 + f1;
            
        } else {
            /* Path B: Different operation mix */
            v0 = v0 - arr2[i];
            v1 = v1 + arr1[i];
            v2 = v2 ^ arr1[i];
            v3 = v3 & arr2[i];
            v4 = v4 | arr1[i];
            v5 = v5 * (arr2[i] | 1);
            v6 = v6 - (arr1[i] >> 1);
            v7 = v7 + (arr2[i] << 2);
            v8 = v8 ^ (v0 & 0xFF);
            v9 = v9 + (v1 & 0xFF);
            v10 = v10 - (v2 & 0xFF);
            v11 = v11 * (v3 | 1);
            v12 = v12 ^ v4;
            v13 = v13 + v5;
            v14 = v14 - v6;
            v15 = v15 * (v7 | 1);
            
            /* Different floating point pattern */
            f0 = f0 - (float)arr2[i];
            f1 = f1 + (float)arr1[i];
            f2 = f2 / (float)((arr1[i] & 0x7F) + 1);
            f3 = f3 * (float)(arr2[i] | 1);
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* Different post-barrier operations */
            v0 = v0 + v8;
            v1 = v1 ^ v9;
            f4 = f4 + f0;
            f5 = f5 * f1;
        }
        
        /* Common merging point with complex operations */
        switch (i & 0x3) {
            case 0:
                v0 = v0 + v10;
                v1 = v1 - v11;
                f6 = f6 * f2;
                goto common_label;
            case 1:
                v2 = v2 ^ v12;
                v3 = v3 + v13;
                f7 = f7 - f3;
                goto common_label;
            case 2:
                v4 = v4 & v14;
                v5 = v5 | v15;
                f6 = f6 + f4;
                /* fall through */
            default:
                v6 = v6 * v0;
                v7 = v7 ^ v1;
                f7 = f7 * f5;
        }
        
    common_label:
        /* Complex operation mixing all variables */
        v8 = (v0 + v2) * (v4 | 1);
        v9 = (v1 ^ v3) & (v5 & 0xFF);
        v10 = v6 - v7 + v8 - v9;
        f0 = f0 + f2 - f4 + f6;
        
        /* Another barrier to increase scheduling complexity */
        asm volatile("" ::: "memory");
        
        /* Final mixing */
        v11 = v10 ^ arr1[i];
        v12 = v11 + arr2[i];
        f1 = f1 * (float)(arr1[i] & 0xFF);
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Volatile store to force computation */
    volatile int result = checksum;
    (void)result;
}

/* Alternate implementation for MIPS if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int v0, v1, v2, v3, v4, v5, v6, v7;
    volatile int *vp = arr1;
    
    v0 = *vp;
    for (int i = 0; i < size; i++) {
        /* MIPS delay slot simulation pressure */
        if (arr1[i] > arr2[i]) {
            v1 = v0 + arr1[i];
            v2 = v1 - arr2[i];
            asm volatile("" ::: "memory");
            v3 = v2 * arr1[i];
            v4 = v3 ^ arr2[i];
        } else {
            v1 = v0 - arr2[i];
            v2 = v1 ^ arr1[i];
            asm volatile("" ::: "memory");
            v3 = v2 & arr1[i];
            v4 = v3 | arr2[i];
        }
        
        v5 = v4 + v1;
        v6 = v5 - v2;
        v7 = v6 * v3;
        v0 = v7 ^ v4;
    }
    
    volatile int r = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    (void)r;
}
#endif

int main(void) {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Execute the scheduling pressure kernel */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
#endif
    
    /* Additional loop to increase optimization scope */
    for (int iter = 0; iter < 10; iter++) {
        int sum = 0;
        for (int i = 0; i < SIZE; i++) {
            /* Complex data-dependent operation */
            sum += (array1[i] * iter) ^ array2[i];
            sum = (sum << 1) | (sum >> 31); /* Rotate */
            
            /* Occasional barrier */
            if ((i & 0xF) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        volatile int dummy = sum;
        (void)dummy;
    }
    
    free(array1);
    free(array2);
    
    printf("Scheduler pressure test completed\n");
    return 0;
}
