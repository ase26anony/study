/* haifa_sched_trigger.c
 * Program designed to trigger GCC Haifa scheduler state save/restore
 * and execute the uncovered cleanup code in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization */
    
    /* Initialize with non-trivial values to prevent constant folding */
    v0 = arr1[0] ^ 0x12345678;
    v1 = arr2[0] | 0x87654321;
    v2 = v0 * v1;
    v3 = v0 + v1;
    v4 = v0 - v1;
    v5 = v0 ^ v1;
    v6 = v2 * v3;
    v7 = v4 * v5;
    v8 = v6 + v7;
    v9 = v6 - v7;
    v10 = v8 * v9;
    v11 = v8 + v9;
    v12 = v10 ^ v11;
    v13 = v10 | v11;
    v14 = v12 & v13;
    v15 = v12 ^ v13;
    
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    f2 = f0 + f1;
    f3 = f0 - f1;
    f4 = f2 * f3;
    f5 = f2 / (f3 + 1.0f);
    f6 = f4 + f5;
    f7 = f4 - f5;
    
    for (int i = 1; i < size; i++) {
        /* Data-dependent branch creates unpredictable control flow */
        int branch_cond = arr1[i] & 3;
        
        /* Force scheduler to consider speculative motion */
        if (__builtin_expect((branch_cond == 0), 0)) {
            /* Path A: Integer-heavy operations */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * (arr1[i] | 1);
            v3 = v3 ^ arr2[i];
            v4 = (v4 << 2) | (arr1[i] & 3);
            v5 = (v5 >> 1) ^ arr2[i];
            v6 = v6 + v0 * v1;
            v7 = v7 - v2 * v3;
            v8 = v8 ^ (v4 * v5);
            v9 = v9 | (v6 + v7);
            v10 = v10 & (v8 ^ v9);
            v11 = v11 + (v10 * 3);
            v12 = v12 - (v11 / 2);
            v13 = v13 ^ (v12 | v11);
            v14 = v14 * (v13 & 0xFF);
            v15 = v15 + (v14 ^ 0xAA);
            
            /* Memory barrier creates serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v0;
            
        } else if (__builtin_expect((branch_cond == 1), 0)) {
            /* Path B: Mixed integer/float operations */
            f0 = f0 + (float)arr1[i];
            f1 = f1 - (float)arr2[i];
            v0 = v0 + (int)f0;
            v1 = v1 - (int)f1;
            f2 = f2 * f0;
            f3 = f3 / (f1 + 0.1f);
            v2 = v2 * (int)(f2 * 100.0f);
            v3 = v3 ^ (int)(f3 * 100.0f);
            f4 = f4 + f2 * f3;
            f5 = f5 - f2 / f3;
            v4 = v4 + (int)f4;
            v5 = v5 - (int)f5;
            f6 = f6 * (f4 + f5);
            f7 = f7 / (f4 - f5 + 0.01f);
            v6 = v6 ^ (int)(f6 * 10.0f);
            v7 = v7 | (int)(f7 * 10.0f);
            
            /* Different barrier placement */
            asm volatile("" ::: "memory");
            mem_barrier = v1;
            
        } else {
            /* Path C: Complex control flow with goto to create CFG complexity */
            int temp = arr1[i] * arr2[i];
            
            if (temp > 1000) {
                v8 = v8 * 3;
                v9 = v9 / 2;
                goto common_label;
            } else if (temp > 500) {
                v10 = v10 ^ 0x55;
                v11 = v11 | 0xAA;
                goto common_label;
            } else {
                v12 = v12 + temp;
                v13 = v13 - temp;
            }
            
        common_label:
            /* Common convergence point with more operations */
            v14 = v14 * (v8 + v9);
            v15 = v15 ^ (v10 & v11);
            f0 = f0 * (float)(v12 + v13);
            f1 = f1 / (float)(v14 - v15 + 1);
            
            /* Third barrier variant */
            asm volatile("" ::: "memory");
            mem_barrier = v2;
        }
        
        /* Additional operations after merge point */
        v0 = v0 ^ v15;
        v1 = v1 | v14;
        v2 = v2 * (v13 & 0xFF);
        v3 = v3 + (v12 >> 2);
        f0 = f0 + f7;
        f1 = f1 - f6;
        
        /* Occasional extra barrier to force state saves */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    arr1[0] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
              v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
              (int)f0 + (int)f1 + (int)f2 + (int)f3 + 
              (int)f4 + (int)f5 + (int)f6 + (int)f7;
}

/* Alternate implementation for MIPS if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    volatile int barrier;
    
    r0 = arr1[0];
    r1 = arr2[0];
    
    for (int i = 1; i < size; i++) {
        /* MIPS delay slot simulation pressure */
        if (arr1[i] > arr2[i]) {
            r2 = r0 * r1;
            r3 = r0 + r1;
            asm volatile("nop" ::: "memory");
            r4 = r2 - r3;
            r5 = r2 ^ r3;
        } else {
            r6 = r0 | r1;
            r7 = r0 & r1;
            asm volatile("nop" ::: "memory");
            r8 = r6 << 2;
            r9 = r7 >> 1;
        }
        
        r10 = r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
        barrier = r10;
        
        /* Force multiple delay slot filling attempts */
        if ((i & 3) == 0) {
            asm volatile("nop; nop; nop" ::: "memory");
        }
    }
    
    arr1[0] = r0 + r1 + r10;
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
    
    /* Execute the scheduling-intensive kernel */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
#endif
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    return 0;
}
