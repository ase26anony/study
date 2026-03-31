/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * and execute the uncovered cleanup code in haifa-sched.cc lines 4681-4691
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__aarch64__)
/* AArch64 has complex scheduling models */
#elif defined(__mips__)
/* MIPS has delay slots that engage scheduler */
#endif
static void compute_intensive_kernel(int *arr1, int *arr2, int size, int *result) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization */
    
    /* Initialize with hard-to-predict values */
    v0 = arr1[0] ^ arr2[0];
    v1 = arr1[1] | arr2[1];
    v2 = arr1[2] & arr2[2];
    v3 = arr1[3] + arr2[3];
    v4 = arr1[4] - arr2[4];
    v5 = arr1[5] * arr2[5];
    v6 = arr1[6] % (arr2[6] ? arr2[6] : 1);
    v7 = arr1[7] << (arr2[7] & 3);
    v8 = arr1[8] >> (arr2[8] & 3);
    v9 = arr1[9] ^ ~arr2[9];
    v10 = arr1[10] | ~arr2[10];
    v11 = arr1[11] + (arr2[11] << 1);
    v12 = arr1[12] - (arr2[12] >> 1);
    v13 = arr1[13] * (arr2[13] | 1);
    v14 = arr1[14] ^ (arr2[14] << 2);
    v15 = arr1[15] | (arr2[15] >> 2);
    
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = (float)v3 * 3.5f;
    f4 = (float)v4 * 4.5f;
    f5 = (float)v5 * 5.5f;
    f6 = (float)v6 * 6.5f;
    f7 = (float)v7 * 7.5f;
    
    /* Complex control flow with data-dependent branches */
    for (int i = 16; i < size; i++) {
        /* Hard-to-predict branch using __builtin_expect */
        int branch_cond = arr1[i] & 0x7;
        if (__builtin_expect((branch_cond > 3), 0)) {
            /* Path 1: Integer-heavy operations */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * (arr1[i] | 1);
            v3 = v3 ^ arr2[i];
            v4 = v4 & arr1[i];
            v5 = v5 | arr2[i];
            v6 = v6 + (arr1[i] << 1);
            v7 = v7 - (arr2[i] >> 1);
            
            /* Mix in floating point to use different functional units */
            f0 = f0 + (float)v0 * 0.1f;
            f1 = f1 - (float)v1 * 0.2f;
            f2 = f2 * (float)(arr1[i] & 0xF) * 0.01f;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v0;
            
            /* More operations after barrier */
            v8 = v8 ^ v0;
            v9 = v9 & v1;
            v10 = v10 | v2;
            v11 = v11 + v3;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
        } else {
            /* Path 2: Different operation mix */
            v12 = v12 + arr2[i];
            v13 = v13 - arr1[i];
            v14 = v14 * (arr2[i] | 1);
            v15 = v15 ^ arr1[i];
            v0 = v0 & arr2[i];
            v1 = v1 | arr1[i];
            
            /* Different FP operations */
            f3 = f3 + (float)v12 * 0.3f;
            f4 = f4 - (float)v13 * 0.4f;
            f5 = f5 * (float)(arr2[i] & 0xF) * 0.02f;
            f6 = f6 / ((float)(arr1[i] & 0x7) + 1.0f);
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            mem_barrier = v12;
            
            /* Continue with different operations */
            v2 = v2 ^ v12;
            v3 = v3 & v13;
            v4 = v4 | v14;
            v5 = v5 + v15;
            
            /* Switch-like control flow within the same block */
            switch (arr1[i] & 0x3) {
                case 0:
                    v6 = v6 << 1;
                    f7 = f7 * 2.0f;
                    /* fall through */
                case 1:
                    v7 = v7 >> 1;
                    f0 = f0 / 2.0f;
                    break;
                case 2:
                    v8 = v8 * 2;
                    f1 = f1 + f2;
                    /* Use goto to create complex CFG */
                    if (__builtin_expect((arr2[i] & 0x1), 0))
                        goto common_label;
                    break;
                default:
                    v9 = v9 / 2;
                    f2 = f2 - f3;
                    break;
            }
            
            /* Common label for goto target */
            common_label:
            v10 = v10 ^ v11;
            
            /* Final barrier before merge */
            asm volatile("" ::: "memory");
        }
        
        /* Merge point - operations using results from both paths */
        v11 = v11 + (v0 ^ v12);
        v12 = v12 - (v1 & v13);
        
        /* Force register pressure with many live variables */
        f7 = f0 + f1 + f2 + f3 + f4 + f5 + f6;
        
        /* Occasionally use all variables to keep them live */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            v13 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                  v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
            f0 = f1 = f2 = f3 = f4 = f5 = f6 = f7 = (float)v13;
        }
    }
    
    /* Compute final result using all variables to prevent elimination */
    *result = v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ 
              v8 ^ v9 ^ v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^
              (int)f0 ^ (int)f1 ^ (int)f2 ^ (int)f3 ^ 
              (int)f4 ^ (int)f5 ^ (int)f6 ^ (int)f7;
}

/* Another kernel with different pattern to increase scheduling complexity */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void secondary_kernel(int *arr, int size, int *sum) {
    int s0 = 0, s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0;
    int t0, t1, t2, t3, t4, t5, t6, t7;
    
    /* Unrolled loop with dependencies */
    for (int i = 0; i < size; i += 8) {
        t0 = arr[i] * 3;
        t1 = arr[i+1] * 5;
        t2 = arr[i+2] * 7;
        t3 = arr[i+3] * 11;
        
        /* Create dependency chain */
        s0 = s0 + t0;
        s1 = s1 + t1 + s0;
        s2 = s2 + t2 + s1;
        s3 = s3 + t3 + s2;
        
        /* Memory barrier in middle of dependency chain */
        asm volatile("" ::: "memory");
        
        t4 = arr[i+4] * 13;
        t5 = arr[i+5] * 17;
        t6 = arr[i+6] * 19;
        t7 = arr[i+7] * 23;
        
        s4 = s4 + t4 + s3;
        s5 = s5 + t5 + s4;
        s6 = s6 + t6 + s5;
        s7 = s7 + t7 + s6;
        
        /* Complex condition with unpredictable branch */
        if (__builtin_expect(((t0 ^ t1 ^ t2 ^ t3) & 0xFF) > 128, 0)) {
            /* Alternate computation path */
            s0 = s0 ^ t4;
            s1 = s1 ^ t5;
            s2 = s2 ^ t6;
            s3 = s3 ^ t7;
            
            /* Force potential state save with many operations */
            for (int j = 0; j < 4; j++) {
                s4 = s4 + (t0 >> j);
                s5 = s5 + (t1 >> j);
                s6 = s6 + (t2 >> j);
                s7 = s7 + (t3 >> j);
                asm volatile("" ::: "memory");
            }
        }
    }
    
    *sum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7;
}

int main() {
    const int SIZE = 512;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values using LCG */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    int result1 = 0, result2 = 0;
    
    /* Call kernels multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < 100; iter++) {
        compute_intensive_kernel(array1, array2, SIZE, &result1);
        secondary_kernel(array1, SIZE, &result2);
        
        /* Mix arrays to change data dependencies */
        for (int i = 0; i < SIZE; i++) {
            array1[i] = array1[i] ^ result1;
            array2[i] = array2[i] ^ result2;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    printf("Checksum: %d\n", result1 ^ result2);
    
    free(array1);
    free(array2);
    
    return 0;
}
