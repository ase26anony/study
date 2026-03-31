/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -funroll-loops=2 -fdump-rtl-sched -fdump-rtl-sched2 -o haifa_test haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex microarchitecture decisions */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(volatile int* restrict arr1, 
                                      volatile int* restrict arr2, 
                                      int size, int threshold) {
    /* High register pressure: many live variables across basic blocks */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    
    /* Initialize with volatile reads to create hard dependencies */
    v0 = *arr1;
    v1 = *arr2;
    v2 = v0 ^ v1;
    v3 = v0 & v1;
    v4 = v0 | v1;
    v5 = v0 + v1;
    v6 = v0 - v1;
    v7 = v0 * v1;
    
    f0 = (float)v0;
    f1 = (float)v1;
    f2 = f0 + f1;
    f3 = f0 - f1;
    f4 = f0 * f1;
    
    /* Main computation with data-dependent branching */
    for (int i = 0; i < size; i++) {
        /* Unpredictable branch to force speculative scheduling */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v8 = arr1[i] * 3;
            v9 = arr2[i] * 5;
            v10 = v8 + v9;
            v11 = v8 - v9;
            v12 = v8 ^ v9;
            v13 = v8 & v9;
            v14 = v8 | v9;
            v15 = v8 % (v9 + 1);
            
            /* Floating-point ops mixed with integer */
            f5 = (float)v8 * 1.5f;
            f6 = (float)v9 * 2.5f;
            f7 = f5 + f6;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Cross-dependent computations */
            v0 = v0 + v10;
            v1 = v1 + v11;
            v2 = v2 ^ v12;
            v3 = v3 & v13;
            v4 = v4 | v14;
            v5 = v5 * (v15 + 1);
            v6 = v6 - v10;
            v7 = v7 ^ v11;
            
            f0 = f0 + f5;
            f1 = f1 + f6;
            f2 = f2 * f7;
            f3 = f3 - f5;
            f4 = f4 / (f6 + 0.1f);
        } else {
            /* Path B: Different operation mix */
            v8 = arr1[i] << 2;
            v9 = arr2[i] >> 1;
            v10 = v8 ^ 0xAAAAAAAA;
            v11 = v9 ^ 0x55555555;
            v12 = v10 + v11;
            v13 = v10 - v11;
            v14 = v10 * v11;
            v15 = (v10 << 3) | (v11 >> 3);
            
            /* Different FP pattern */
            f5 = (float)(arr1[i] & 0xFF) * 0.25f;
            f6 = (float)(arr2[i] & 0xFF) * 0.75f;
            f7 = f5 - f6;
            
            /* Another memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* Different dependency pattern */
            v0 = v0 ^ v10;
            v1 = v1 | v11;
            v2 = v2 + v12;
            v3 = v3 - v13;
            v4 = v4 * (v14 & 0xFF);
            v5 = v5 ^ v15;
            v6 = v6 & v10;
            v7 = v7 | v11;
            
            f0 = f0 - f5;
            f1 = f1 * f6;
            f2 = f2 + f7;
            f3 = f3 / (f5 + 0.5f);
            f4 = f4 - f6;
        }
        
        /* Common merge point with more computations */
        v0 = (v0 << 1) | (v0 >> 31);
        v1 = (v1 << 2) | (v1 >> 30);
        v2 = (v2 << 3) | (v2 >> 29);
        
        /* Force potential state save with complex expression */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            /* Additional nested control flow */
            switch (v0 & 0x3) {
                case 0:
                    v3 = v3 + arr1[i % size];
                    v4 = v4 - arr2[i % size];
                    goto common_label;
                case 1:
                    v5 = v5 * (arr1[i % size] | 1);
                    v6 = v6 ^ arr2[i % size];
                    goto common_label;
                case 2:
                    v7 = v7 & arr1[i % size];
                    v8 = v8 | arr2[i % size];
                    /* fall through */
                default:
                    v9 = v9 + (arr1[i % size] ^ arr2[i % size]);
            }
            common_label:
            v10 = v10 + 1;
        }
        
        /* Final barrier in loop */
        asm volatile("" ::: "memory");
    }
    
    /* Use all variables to prevent optimization */
    volatile int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    volatile float fresult = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (result), "+r" (fresult));
}

/* Alternative implementation for MIPS if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(volatile int* arr1, volatile int* arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* MIPS delay slot patterns might trigger different scheduler behavior */
    for (int i = 0; i < size; i++) {
        r0 = arr1[i];
        r1 = arr2[i];
        
        /* Create data dependencies for delay slots */
        r2 = r0 + r1;
        r3 = r0 - r1;
        r4 = r0 & r1;
        r5 = r0 | r1;
        r6 = r0 ^ r1;
        
        /* Branch with likely hint for MIPS */
        if (__builtin_expect(r0 > r1, 1)) {
            r7 = r2 * r3;
            r8 = r4 + r5;
            r9 = r6 << 2;
        } else {
            r7 = r3 / (r2 + 1);
            r8 = r5 - r4;
            r9 = r6 >> 2;
        }
        
        /* More operations to increase pressure */
        r10 = r7 + r8;
        r11 = r9 ^ r10;
        r12 = r11 * 3;
        r13 = r12 & 0xFF;
        r14 = r13 | 0x80;
        r15 = r14 - 1;
        
        asm volatile("" ::: "memory");
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    const int THRESHOLD = 1000;
    
    /* Initialize with pseudo-random values */
    int arr1[SIZE], arr2[SIZE];
    unsigned int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        arr2[i] = (seed >> 16) & 0x7FFF;
    }
    
    /* Volatile pointers to prevent optimization */
    volatile int* varr1 = arr1;
    volatile int* varr2 = arr2;
    
    /* Execute multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < 100; iter++) {
        complex_scheduling_kernel(varr1, varr2, SIZE, THRESHOLD + iter);
        
#ifdef __mips__
        mips_specific_kernel(varr1, varr2, SIZE);
#endif
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
