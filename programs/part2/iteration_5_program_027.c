/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of complex scheduling model on x86 */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = arr1[0] ^ 0x12345678;
    v1 = arr2[0] | 0x87654321;
    v2 = v0 * v1;
    v3 = v0 + v1;
    v4 = v0 - v1;
    v5 = v0 ^ v1;
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    
    for (int i = 1; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = arr1[i] & arr2[i];
        
        if (__builtin_expect((branch_cond & 0x7F) > 64, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v6 = arr1[i] * v0;
            v7 = arr2[i] + v1;
            v8 = v6 ^ v7;
            v9 = v6 - v7;
            v10 = v8 * v9;
            v11 = v10 >> 3;
            v12 = v11 + v2;
            v13 = v12 - v3;
            v14 = v13 | v4;
            v15 = v14 & v5;
            
            /* Floating point ops mixed in */
            f2 = f0 * f1;
            f3 = f2 + (float)v6;
            f4 = f3 - (float)v7;
            f5 = f4 * 2.0f;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v15;
            
            /* Update state variables */
            v0 = v15 ^ v0;
            v1 = v15 + v1;
            v2 = v2 * v15;
            f0 = f5 * 0.9f;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
        } else {
            /* Path B: Different operation mix with same variables */
            v6 = arr1[i] + v0;
            v7 = arr2[i] - v1;
            v8 = v6 | v7;
            v9 = v6 & v7;
            v10 = v8 ^ v9;
            v11 = v10 << 2;
            v12 = v11 - v2;
            v13 = v12 + v3;
            v14 = v13 ^ v4;
            v15 = v14 | v5;
            
            /* Different floating point sequence */
            f2 = f1 / f0;
            f3 = f2 - (float)v6;
            f4 = f3 + (float)v7;
            f5 = f4 / 1.5f;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            mem_barrier = v15;
            
            /* Update state differently */
            v0 = v15 | v0;
            v1 = v15 - v1;
            v2 = v2 + v15;
            f0 = f5 + 1.1f;
            
            /* Barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Common merge point with more computation */
        f6 = f0 * f1;
        f7 = f6 + f2 - f3;
        
        /* Complex switch to create control flow merging */
        switch (i & 0x3) {
            case 0:
                v3 = v3 + v15;
                v4 = v4 ^ v15;
                goto common_label;
            case 1:
                v5 = v5 | v15;
                v6 = v6 & v15;
                goto common_label;
            case 2:
                v7 = v7 - v15;
                v8 = v8 + v15;
                /* fall through */
            default:
                v9 = v9 * v15;
                v10 = v10 ^ v15;
        }
        
    common_label:
        /* More operations at merge point */
        v11 = v3 + v4 - v5;
        v12 = v6 | v7 & v8;
        v13 = v9 ^ v10;
        
        /* Final barrier in loop */
        asm volatile("" ::: "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Volatile store to force computation */
    volatile int *volatile_ptr = &mem_barrier;
    *volatile_ptr = checksum;
}

/* MIPS-specific version with delay slot considerations */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
    volatile int barrier;
    
    r0 = arr1[0];
    r1 = arr2[0];
    
    for (int i = 1; i < size; i++) {
        /* MIPS often has delay slots - create complex branching */
        if (arr1[i] > arr2[i]) {
            r2 = arr1[i] * r0;
            r3 = arr2[i] + r1;
            asm volatile("" ::: "memory");
            barrier = r2;
            r0 = r2 ^ r3;
            r1 = r2 - r3;
        } else {
            r2 = arr1[i] + r0;
            r3 = arr2[i] - r1;
            asm volatile("" ::: "memory");
            barrier = r2;
            r0 = r2 | r3;
            r1 = r2 & r3;
        }
        
        /* More variables for pressure */
        r4 = r0 << 2;
        r5 = r1 >> 1;
        r6 = r4 + r5;
        r7 = r4 - r5;
        r8 = r6 * r7;
        r9 = r8 ^ r0;
        r10 = r9 & r1;
        r11 = r10 | r4;
        r12 = r11 - r5;
        
        asm volatile("" ::: "memory");
    }
    
    volatile int *p = &barrier;
    *p = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}
#endif

int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
#endif
    
    /* Compute final checksum from arrays to ensure work is done */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] ^ array2[i];
    }
    
    printf("Result: %d\n", final_sum);
    return 0;
}
