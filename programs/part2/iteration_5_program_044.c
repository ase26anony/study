/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -funroll-loops=2 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS-specific patterns to engage delay slot scheduling */
__attribute__((noinline))
#endif
static int process_block(int *arr1, int *arr2, int idx) {
    /* High register pressure with many distinct variables */
    volatile int v0 = arr1[idx];  /* Prevent optimization */
    int v1 = arr2[idx];
    int v2 = v0 + v1;
    int v3 = v0 - v1;
    int v4 = v0 * v1;
    int v5 = v0 ^ v1;
    int v6 = v0 | v1;
    int v7 = v0 & v1;
    int v8 = v2 * v3;
    int v9 = v4 ^ v5;
    int v10 = v6 | v7;
    int v11 = v8 + v9;
    int v12 = v10 - v11;
    int v13 = v12 * v2;
    int v14 = v13 ^ v3;
    int v15 = v14 | v4;
    
    /* Floating point variables to increase pressure */
    float f0 = (float)v0;
    float f1 = (float)v1;
    float f2 = f0 * f1;
    float f3 = f0 + f1;
    float f4 = f2 - f3;
    float f5 = f4 * 1.414f;
    float f6 = f5 + 2.718f;
    float f7 = f6 * 0.577f;
    
    /* Data-dependent branch creating scheduling complexity */
    if (__builtin_expect((v0 & 0x7F) > (v1 & 0x3F), 0)) {
        /* Path A: More arithmetic operations */
        v2 = v2 * 3;
        v3 = v3 + v4;
        v4 = v4 ^ v5;
        v5 = v5 | v6;
        v6 = v6 & v7;
        v7 = v7 * 2;
        v8 = v8 - v9;
        v9 = v9 + v10;
        
        f0 = f0 * 2.0f;
        f1 = f1 + 1.0f;
        f2 = f2 - f3;
        f3 = f3 * f4;
        
        /* Memory barrier to force serialization point */
        asm volatile("" ::: "memory");
        
        /* Additional operations after barrier */
        v10 = v10 ^ v11;
        v11 = v11 | v12;
        v12 = v12 & v13;
        v13 = v13 * 3;
    } else {
        /* Path B: Different operation mix */
        v2 = v2 + 5;
        v3 = v3 - v4;
        v4 = v4 | v5;
        v5 = v5 ^ v6;
        v6 = v6 * v7;
        v7 = v7 & 0xFF;
        v8 = v8 + v9;
        v9 = v9 - v10;
        
        f0 = f0 / 2.0f;
        f1 = f1 - 1.0f;
        f2 = f2 + f3;
        f3 = f3 / f4;
        
        /* Memory barrier at different position */
        asm volatile("" ::: "memory");
        
        /* Different operations after barrier */
        v10 = v10 | v11;
        v11 = v11 ^ v12;
        v12 = v12 * v13;
        v13 = v13 & 0x7F;
    }
    
    /* Merge point with more operations */
    int v16 = v2 + v3 + v4 + v5;
    int v17 = v6 * v7 * v8;
    int v18 = v9 ^ v10 ^ v11;
    int v19 = v12 | v13 | v14;
    
    float f8 = f0 + f1 + f2;
    float f9 = f3 * f4 * f5;
    
    /* Complex expression to prevent optimization */
    return (v15 + v16 + v17 + v18 + v19) ^ 
           ((int)f8 * 100) ^ 
           ((int)f9 * 37);
}

/* Function with switch statement creating complex CFG */
#ifdef __mips__
__attribute__((noinline, optimize("O0")))  /* Less optimization for MIPS */
#else
__attribute__((noinline))
#endif
static int process_with_switch(int *arr, int idx) {
    int result = 0;
    
    /* Complex switch with multiple cases operating on same variables */
    switch (arr[idx] & 0x3) {
        case 0: {
            int t0 = arr[idx] + 1;
            int t1 = arr[idx + 1] * 2;
            int t2 = t0 ^ t1;
            asm volatile("" ::: "memory");
            result = t2 * 3;
            goto common_label;  /* Jump to common code */
        }
        case 1: {
            int t0 = arr[idx] - 1;
            int t1 = arr[idx + 1] / 2;
            int t2 = t0 | t1;
            asm volatile("" ::: "memory");
            result = t2 + 5;
            goto common_label;
        }
        case 2: {
            int t0 = arr[idx] * 3;
            int t1 = arr[idx + 1] + 7;
            int t2 = t0 & t1;
            asm volatile("" ::: "memory");
            result = t2 - 9;
            goto common_label;
        }
        default: {
            int t0 = arr[idx] ^ 0xFF;
            int t1 = arr[idx + 1] | 0x7F;
            int t2 = t0 * t1;
            asm volatile("" ::: "memory");
            result = t2 / 2;
            /* Fall through to common_label */
        }
    }
    
common_label:
    /* Common code reached from multiple paths */
    result = result ^ (arr[idx] & 0xF);
    asm volatile("" ::: "memory");
    return result * 17;
}

/* Main computational kernel */
int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        array2[i] = (seed >> 8) & 0x3FFF;
    }
    
    int total = 0;
    
    /* Outer loop with data-dependent branching */
    for (int outer = 0; outer < 1000; outer++) {
        for (int i = 0; i < SIZE - 4; i++) {
            /* First processing path */
            int r1 = process_block(array1, array2, i);
            
            /* Second processing with switch */
            int r2 = process_with_switch(array1, i);
            
            /* Complex merging of results */
            if (__builtin_expect((array1[i] & 0x1F) > 16, 0)) {
                total += r1 * 3 + r2;
            } else {
                total += r1 + r2 * 2;
            }
            
            /* Occasionally modify arrays to prevent optimization */
            if ((i & 0x1F) == 0) {
                array1[i] ^= total & 0xFF;
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int checksum = total;
    printf("Result: %d\n", checksum);
    
    return 0;
}
