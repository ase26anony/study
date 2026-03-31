/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -mips32 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of specific scheduling model hooks */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS-specific scheduling patterns */
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    mem_barrier = *arr1;
    v0 = mem_barrier;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 - v0;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict pattern */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v4 = arr1[i] * 3;
            v5 = arr2[i] / 2;
            v6 = v4 ^ v5;
            v7 = v6 << 2;
            v8 = v7 | 0x7F;
            v9 = v8 & 0xFF;
            v10 = v9 + v4;
            v11 = v10 - v5;
            v12 = v11 * v6;
            v13 = v12 ^ v7;
            v14 = v13 + v8;
            v15 = v14 | v9;
            
            /* Floating-point ops mixed in */
            f0 = (float)v4 * 1.5f;
            f1 = (float)v5 * 2.5f;
            f2 = f0 + f1;
            f3 = f2 * 0.75f;
            f4 = f3 - f0;
            f5 = f4 / f1;
            f6 = f5 * 3.14159f;
            f7 = f6 + f2;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex dependency chain */
            v0 = v15 + (int)f7;
            v1 = v0 ^ arr1[i];
            v2 = v1 * arr2[i];
        } else {
            /* Path B: Different operation mix */
            v4 = arr1[i] + arr2[i];
            v5 = v4 * 7;
            v6 = v5 >> 1;
            v7 = v6 & 0x3F;
            v8 = v7 | 0x80;
            v9 = v8 ^ 0xAA;
            v10 = v9 - v4;
            v11 = v10 * 3;
            v12 = v11 / 2;
            v13 = v12 ^ v5;
            v14 = v13 + v6;
            v15 = v14 & v7;
            
            /* Alternate floating-point sequence */
            f0 = (float)arr1[i] * 0.25f;
            f1 = (float)arr2[i] * 4.0f;
            f2 = f0 - f1;
            f3 = f2 * 2.0f;
            f4 = f3 / 0.5f;
            f5 = f4 + f0;
            f6 = f5 * 1.618f;
            f7 = f6 - f1;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Different dependency pattern */
            v0 = v15 - (int)f7;
            v1 = v0 & arr1[i];
            v2 = v1 + arr2[i];
        }
        
        /* Merge point with switch to create complex CFG */
        switch (arr1[i] & 0x3) {
            case 0:
                v3 = v2 * 2;
                goto common_label;
            case 1:
                v3 = v2 + 5;
                goto common_label;
            case 2:
                v3 = v2 ^ 0xFF;
                /* fall through */
            default:
                v3 = v2 - 3;
        common_label:
                v4 = v3 * v0;
                v5 = v4 ^ v1;
        }
        
        /* Force register spilling with many live variables */
        arr1[i] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        arr2[i] = (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    }
    
    /* Use all variables to prevent dead code elimination */
    mem_barrier = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    mem_barrier += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
}

/* Function pointer to inhibit optimization */
static void (*volatile fp)(int*, int*, int, int) = complex_scheduling_kernel;

int main() {
    const int SIZE = 256;
    int arr1[SIZE], arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Call through volatile function pointer */
    fp(arr1, arr2, SIZE, 10000);
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
