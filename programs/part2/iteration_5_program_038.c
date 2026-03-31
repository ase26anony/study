/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex microarchitecture decisions */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* Create high register pressure with many variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    mem_barrier = *arr1;
    v0 = mem_barrier;
    v1 = *arr2;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict pattern */
        int branch_cond = arr1[i] & 0x7F; /* Use lower bits for branch */
        
        /* Force scheduler to consider speculative motion */
        if (__builtin_expect((branch_cond > 64), 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v2 = v0 + arr1[i];
            v3 = v1 * arr2[i];
            v4 = v2 ^ v3;
            v5 = v4 << (arr1[i] & 0x3);
            v6 = v5 - v3;
            v7 = v6 | arr2[i];
            v8 = v7 + v2;
            v9 = v8 * 0x9E3779B9; /* Multiplication with constant */
            v10 = v9 - v5;
            v11 = v10 ^ v6;
            v12 = v11 + v7;
            v13 = v12 * v8;
            v14 = v13 >> 4;
            v15 = v14 & 0xFF;
            
            /* Floating point ops to mix instruction types */
            f0 = (float)v2 * 1.5f;
            f1 = (float)v3 * 2.5f;
            f2 = f0 + f1;
            f3 = f2 * 0.75f;
            f4 = f3 - f0;
            f5 = f4 / 2.0f;
            f6 = f5 * f3;
            f7 = f6 + f2;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* Cross-path dependencies */
            v0 = v15 + (int)f7;
            v1 = v14 ^ (int)f6;
        } else {
            /* Path B: Different computation pattern with same variables */
            v2 = v0 - arr1[i];
            v3 = v1 + arr2[i];
            v4 = v2 & v3;
            v5 = v4 >> (arr2[i] & 0x3);
            v6 = v5 * v3;
            v7 = v6 ^ arr1[i];
            v8 = v7 - v2;
            v9 = v8 + 0x61C88647;
            v10 = v9 ^ v5;
            v11 = v10 | v6;
            v12 = v11 - v7;
            v13 = v12 & v8;
            v14 = v13 << 2;
            v15 = v14 | 0x1;
            
            /* Different floating point sequence */
            f0 = (float)v2 / 3.0f;
            f1 = (float)v3 * 1.25f;
            f2 = f1 - f0;
            f3 = f2 * 2.0f;
            f4 = f3 + f0;
            f5 = f4 / 1.5f;
            f6 = f5 * f2;
            f7 = f6 - f1;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            v0 = v15 - (int)f7;
            v1 = v14 & (int)f6;
        }
        
        /* Complex switch to create CFG merges/splits */
        switch (arr1[i] & 0x3) {
            case 0:
                v2 = v0 + v1;
                goto common_label;
            case 1:
                v2 = v0 - v1;
                /* fall through */
            case 2:
                v3 = v2 * 2;
                goto common_label;
            default:
                v3 = v2 / 2;
                /* continue to common_label */
        }
        
    common_label:
        /* Force register spilling with many live variables */
        arr1[i] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        arr2[i] = (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
        
        /* Another barrier to prevent instruction reordering */
        asm volatile("" : "+r"(v0), "+r"(v1) : : "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    
    /* Prevent optimization of entire function */
    asm volatile("" : : "r"(checksum) : "memory");
}

/* Alternative MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int v0, v1, v2, v3, v4, v5;
    volatile int barrier;
    
    for (int i = 0; i < size; i++) {
        /* Create delay slot pressure */
        if (arr1[i] > arr2[i]) {
            v0 = arr1[i] * 3;
            v1 = arr2[i] + 7;
            /* Force nop insertion opportunities */
            asm volatile(".set noreorder\n\t"
                         ".set nomacro\n\t"
                         "nop\n\t"
                         ".set macro\n\t"
                         ".set reorder" : : : "memory");
        } else {
            v0 = arr1[i] / 2;
            v1 = arr2[i] - 3;
        }
        
        v2 = v0 << 2;
        v3 = v1 >> 1;
        v4 = v2 | v3;
        v5 = v4 & 0xFF;
        
        arr1[i] = v5;
        barrier = v0 + v1 + v2 + v3 + v4 + v5;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Run the scheduling stress test */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
#endif
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] ^ array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
