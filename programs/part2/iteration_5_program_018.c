/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex microarchitecture decisions */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS-specific patterns to engage delay slot scheduling */
__attribute__((noinline))
#endif
static void compute_kernel(int *arr1, int *arr2, int size, int *result) {
    /* High register pressure: many live variables across basic blocks */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7;
    int i, t;
    
    /* Initialize volatile variables to prevent optimization */
    v0 = v1 = v2 = v3 = v4 = v5 = v6 = v7 = 1;
    v8 = v9 = v10 = v11 = v12 = v13 = v14 = v15 = 2;
    f0 = f1 = f2 = f3 = 1.0f;
    f4 = f5 = f6 = f7 = 2.0f;
    
    for (i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict pattern */
        int branch_cond = arr1[i] & 0x7F;  /* Use lower bits for branch decision */
        
        /* Force scheduler to consider speculative motion */
        if (__builtin_expect((branch_cond > 64), 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v0 = arr1[i] + v15;
            v1 = v0 * v14;
            v2 = v1 - v13;
            v3 = v2 ^ v12;
            v4 = v3 | v11;
            v5 = v4 & v10;
            v6 = v5 << 2;
            v7 = v6 >> 1;
            v8 = v7 + arr2[i];
            v9 = v8 * 3;
            v10 = v9 - v0;
            v11 = v10 ^ v1;
            v12 = v11 | v2;
            v13 = v12 & v3;
            v14 = v13 << 1;
            v15 = v14 >> 2;
            
            /* Mix in floating point to use different functional units */
            f0 = (float)v0 * 1.5f;
            f1 = f0 + (float)v1;
            f2 = f1 * 0.75f;
            f3 = f2 - f0;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex control flow within the path */
            switch (arr1[i] & 0x3) {
                case 0:
                    v0 = v1 + v2;
                    goto merge_point;
                case 1:
                    v0 = v3 - v4;
                    /* fall through */
                case 2:
                    v0 = v5 * v6;
                    break;
                default:
                    v0 = v7 ^ v8;
                    break;
            }
            merge_point:
            v1 = v0 + arr2[i];
        } else {
            /* Path B: Different instruction mix with memory operations */
            v15 = arr2[i] - v0;
            v14 = v15 * v1;
            v13 = v14 / (v2 + 1);
            v12 = v13 ^ v3;
            v11 = v12 | v4;
            v10 = v11 & v5;
            v9 = v10 << 3;
            v8 = v9 >> 2;
            v7 = v8 + arr1[i];
            v6 = v7 * 5;
            v5 = v6 - v15;
            v4 = v5 ^ v14;
            v3 = v4 | v13;
            v2 = v3 & v12;
            v1 = v2 << 2;
            v0 = v1 >> 1;
            
            /* Different floating point sequence */
            f4 = (float)v15 * 2.25f;
            f5 = f4 + (float)v14;
            f6 = f5 * 1.125f;
            f7 = f6 - f4;
            
            /* Another memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* More complex control flow with goto */
            if (arr2[i] & 1) {
                v15 = v14 + v13;
                goto b_merge;
            } else {
                v15 = v12 - v11;
            }
            b_merge:
            v14 = v15 * arr1[i];
        }
        
        /* Common code with high register pressure */
        f0 = f0 + f4;
        f1 = f1 + f5;
        f2 = f2 + f6;
        f3 = f3 + f7;
        
        /* Force spilling with many simultaneous operations */
        t = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
            v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        
        /* Use result to create anti-dependencies */
        arr1[i] = (arr1[i] ^ t) & 0xFF;
        arr2[i] = (arr2[i] + t) & 0xFF;
        
        /* Periodic barrier to split scheduling regions */
        if ((i & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    *result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
              v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
              (int)f0 + (int)f1 + (int)f2 + (int)f3 +
              (int)f4 + (int)f5 + (int)f6 + (int)f7;
}

/* Alternative version for architectures without target attributes */
#ifdef __mips__
static void mips_compute_kernel(int *arr1, int *arr2, int size, int *result) {
    volatile int v0, v1, v2, v3, v4, v5, v6, v7;
    int i;
    
    v0 = v1 = v2 = v3 = v4 = v5 = v6 = v7 = 1;
    
    for (i = 0; i < size; i++) {
        /* Create patterns that might engage MIPS delay slot scheduling */
        if (arr1[i] > arr2[i]) {
            v0 = arr1[i] + v7;
            v1 = v0 - v6;
            asm volatile("" ::: "memory");  /* Force scheduling boundary */
            v2 = v1 * v5;
            v3 = v2 ^ v4;
            v4 = v3 | arr2[i];
            v5 = v4 & 0xFF;
            v6 = v5 << (arr1[i] & 0x3);
            v7 = v6 >> 1;
        } else {
            v7 = arr2[i] - v0;
            v6 = v7 * v1;
            asm volatile("" ::: "memory");
            v5 = v6 / (v2 + 1);
            v4 = v5 ^ v3;
            v3 = v4 | arr1[i];
            v2 = v3 & 0xFF;
            v1 = v2 << (arr2[i] & 0x3);
            v0 = v1 >> 1;
        }
        
        /* Many live variables to increase register pressure */
        arr1[i] = v0 + v1 + v2 + v3;
        arr2[i] = v4 + v5 + v6 + v7;
    }
    
    *result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
}
#endif

int main() {
    const int SIZE = 512;
    int arr1[SIZE], arr2[SIZE];
    int i, result1, result2;
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0xFFF;
        arr2[i] = (seed >> 8) & 0xFFF;
    }
    
    /* Run computation kernel */
#ifdef __mips__
    mips_compute_kernel(arr1, arr2, SIZE, &result1);
#else
    compute_kernel(arr1, arr2, SIZE, &result1);
#endif
    
    /* Second pass with modified data to explore different scheduling paths */
    for (i = 0; i < SIZE; i++) {
        arr1[i] ^= 0x555;
        arr2[i] += 0xAAA;
    }
    
#ifdef __mips__
    mips_compute_kernel(arr1, arr2, SIZE, &result2);
#else
    compute_kernel(arr1, arr2, SIZE, &result2);
#endif
    
    /* Use results to prevent optimization */
    printf("Results: %d %d\n", result1, result2);
    
    return 0;
}
