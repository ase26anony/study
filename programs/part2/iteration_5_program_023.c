/* haifa_sched_trigger.c
 * Designed to trigger GCC's Haifa scheduler state save/restore mechanism
 * and exercise the free_sched_context cleanup path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force specific microarchitecture for x86 to use detailed scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
#ifdef __i386__
__attribute__((target("arch=core2")))
#endif
static void computation_kernel(int* restrict arr1, int* restrict arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5;
    volatile int mem_barrier; /* Force memory dependencies */
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16;
    f0 = 1.1f; f1 = 2.2f; f2 = 3.3f; f3 = 4.4f; f4 = 5.5f; f5 = 6.6f;
    mem_barrier = 0;
    
    /* Complex loop with data-dependent branches */
    for (int i = 0; i < size; i++) {
        /* Read volatile to create hard dependency */
        int threshold = mem_barrier;
        
        /* Data-dependent branch - unpredictable at compile time */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * v0;
            v3 = v3 ^ v1;
            v4 = v4 & v2;
            v5 = v5 | v3;
            v6 = v6 + v4;
            v7 = v7 - v5;
            v8 = v8 * v6;
            v9 = v9 ^ v7;
            
            /* Mix in floating point to use different functional units */
            f0 = f0 + (float)v0;
            f1 = f1 - (float)v1;
            f2 = f2 * f0;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* More computations after barrier */
            v10 = v10 + (int)f0;
            v11 = v11 - (int)f1;
            v12 = v12 * v10;
            v13 = v13 ^ v11;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Jump to common label creating complex CFG */
            goto common_path;
        } else {
            /* Path B: Different computation pattern */
            v0 = v0 - arr2[i];
            v1 = v1 + arr1[i];
            v2 = v2 ^ v0;
            v3 = v3 & v1;
            v4 = v4 | v2;
            v5 = v5 * v3;
            v6 = v6 - v4;
            v7 = v7 + v5;
            v8 = v8 ^ v6;
            v9 = v9 & v7;
            
            /* Different FP sequence */
            f3 = f3 + (float)v0;
            f4 = f4 - (float)v1;
            f5 = f5 * f3;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            v14 = v14 + (int)f3;
            v15 = v15 - (int)f4;
            v12 = v12 ^ v14;
            v13 = v13 & v15;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Fall through to common path */
        }
        
    common_path:
        /* Converging path with more computations */
        v0 = v0 ^ v15;
        v1 = v1 & v14;
        v2 = v2 | v13;
        v3 = v3 ^ v12;
        
        /* Force spilling with many live variables */
        f0 = f0 + f5;
        f1 = f1 - f4;
        f2 = f2 * f3;
        
        /* Complex expression with many operands */
        v4 = (v0 * v1) + (v2 ^ v3) - (v4 & v5) | (v6 ^ v7);
        
        /* Switch statement inside loop for additional CFG complexity */
        switch (arr1[i] & 0x3) {
            case 0:
                v5 = v5 + v8;
                v6 = v6 - v9;
                break;
            case 1:
                v5 = v5 - v8;
                v6 = v6 + v9;
                break;
            case 2:
                v5 = v5 ^ v8;
                v6 = v6 & v9;
                /* goto creates additional edge */
                goto special_case;
            default:
                v5 = v5 & v8;
                v6 = v6 ^ v9;
                break;
        }
        
        /* Back edge from special case */
        if (0) {
        special_case:
            v7 = v7 * v10;
            v8 = v8 ^ v11;
        }
        
        /* Final mixing */
        v9 = v9 + v12;
        v10 = v10 - v13;
        v11 = v11 * v14;
        v12 = v12 ^ v15;
        
        /* Use all variables to prevent dead code elimination */
        mem_barrier = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12;
    }
    
    /* Use results to prevent optimization */
    arr1[0] = v0 + v1 + v2 + v3 + v4;
    arr2[0] = v5 + v6 + v7 + v8 + v9;
}

/* MIPS-specific variant with delay slot considerations */
#ifdef __mips__
__attribute__((optimize("O2")))
static void mips_computation(int* arr1, int* arr2, int size) {
    int i, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    volatile int barrier;
    
    for (i = 0; i < size; i++) {
        barrier = arr1[i];
        if (barrier & 1) {
            t0 = arr1[i] + arr2[i];
            t1 = t0 * 3;
            t2 = t1 ^ 0x5555;
            asm volatile("" ::: "memory");
            t3 = t2 - arr1[i];
            t4 = t3 & 0xFFFF;
            goto mips_merge;
        } else {
            t0 = arr1[i] - arr2[i];
            t1 = t0 / 2;
            t2 = t1 | 0xAAAA;
            asm volatile("" ::: "memory");
            t3 = t2 + arr2[i];
            t4 = t3 | 0x3333;
        }
        
    mips_merge:
        t5 = t4 * t3;
        t6 = t5 ^ t2;
        t7 = t6 + t1;
        t8 = t7 - t0;
        t9 = t8 & 0xFF;
        
        /* Force delay slot scheduling complexity */
        asm volatile("nop" ::: "memory");
        arr1[i] = t9;
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    int* arr1 = malloc(SIZE * sizeof(int));
    int* arr2 = malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed >> 16) & 0x7FFF;
        arr2[i] = (int)(seed >> 8) & 0x7FFF;
    }
    
    /* Run computation kernel */
    computation_kernel(arr1, arr2, SIZE);
    
#ifdef __mips__
    mips_computation(arr1, arr2, SIZE);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    volatile int* varr1 = arr1;
    volatile int* varr2 = arr2;
    
    for (int i = 0; i < SIZE; i++) {
        checksum ^= varr1[i];
        checksum += varr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
