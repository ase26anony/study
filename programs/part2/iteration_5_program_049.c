/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* Alternative: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int* restrict arr1, int* restrict arr2, int size, int threshold) {
    /* High register pressure: many local variables in a small scope */
    volatile int seed = *arr1; /* volatile read creates hard dependency */
    int i, j;
    
    /* Many distinct variables to overwhelm registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    
    /* Initialize with non-trivial values */
    v0 = seed ^ 0x12345678;
    v1 = seed * 1103515245 + 12345;
    v2 = v0 ^ v1;
    v3 = v1 - v2;
    v4 = v2 * v3;
    v5 = v3 / (v4 ? v4 : 1);
    v6 = v4 | v5;
    v7 = v5 & v6;
    v8 = v6 ^ v7;
    v9 = v7 + v8;
    v10 = v8 - v9;
    v11 = v9 * v10;
    v12 = v10 ^ v11;
    v13 = v11 + v12;
    v14 = v12 - v13;
    v15 = v13 * v14;
    
    f0 = (float)v0 * 0.1f;
    f1 = (float)v1 * 0.2f;
    f2 = (float)v2 * 0.3f;
    f3 = (float)v3 * 0.4f;
    f4 = (float)v4 * 0.5f;
    f5 = (float)v5 * 0.6f;
    f6 = (float)v6 * 0.7f;
    f7 = (float)v7 * 0.8f;
    
    /* Complex loop with data-dependent branches */
    for (i = 0; i < size; i++) {
        /* Unpredictable branch using array data */
        if (__builtin_expect((arr1[i] & arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy operations */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * (arr1[i] | 1);
            v3 = v3 ^ arr2[i];
            v4 = v4 & arr1[i];
            v5 = v5 | arr2[i];
            
            /* Mix in floating point to use different functional units */
            f0 = f0 + (float)arr1[i];
            f1 = f1 - (float)arr2[i];
            f2 = f2 * (float)(arr1[i] + 1);
            
            /* Memory barrier creates serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v6 = v6 + (v0 >> 2);
            v7 = v7 - (v1 << 1);
            v8 = v8 * (v2 & 0xFF);
            v9 = v9 ^ v3;
            
            f3 = f3 + f0;
            f4 = f4 - f1;
            f5 = f5 * f2;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Switch-like control flow within the same basic block */
            switch (arr1[i] & 0x3) {
                case 0:
                    v10 = v10 + v4;
                    v11 = v11 - v5;
                    goto common_label;
                case 1:
                    v12 = v12 * v6;
                    v13 = v13 ^ v7;
                    goto common_label;
                case 2:
                    v14 = v14 & v8;
                    v15 = v15 | v9;
                    goto common_label;
                default:
                    v10 = v10 ^ v10; /* Self-XOR creates zero */
                    v11 = v11 + 1;
                    /* fall through */
            }
        } else {
            /* Path B: Different operation mix */
            v0 = v0 - arr2[i];
            v1 = v1 + arr1[i];
            v2 = v2 ^ arr1[i];
            v3 = v3 & arr2[i];
            v4 = v4 | arr1[i];
            v5 = v5 * (arr2[i] | 1);
            
            f0 = f0 - (float)arr2[i];
            f1 = f1 + (float)arr1[i];
            f2 = f2 / ((float)arr1[i] + 1.0f);
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            v6 = v6 - (v0 << 1);
            v7 = v7 + (v1 >> 2);
            v8 = v8 ^ (v2 & 0xFF);
            v9 = v9 & v3;
            
            f3 = f3 - f0;
            f4 = f4 + f1;
            f5 = f5 / (f2 + 0.1f);
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Different switch pattern */
            switch (arr2[i] & 0x3) {
                case 0:
                    v12 = v12 + v4;
                    v13 = v13 - v5;
                    break;
                case 1:
                    v14 = v14 * v6;
                    v15 = v15 ^ v7;
                    break;
                case 2:
                    v10 = v10 & v8;
                    v11 = v11 | v9;
                    break;
                default:
                    v12 = v12 ^ v12;
                    v13 = v13 + 1;
                    break;
            }
        }
        
common_label:
        /* Common convergence point with more operations */
        f6 = f6 + f3;
        f7 = f7 - f4;
        
        /* Force register pressure with many live variables */
        v0 = v0 ^ v15;
        v1 = v1 + v14;
        v2 = v2 - v13;
        v3 = v3 * v12;
        v4 = v4 & v11;
        v5 = v5 | v10;
        
        /* Final barrier before loop continues */
        asm volatile("" ::: "memory");
        
        /* Small inner loop to create more scheduling complexity */
        for (j = 0; j < 2; j++) {
            v0 = v0 + j;
            v1 = v1 - j;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + 
                   v10 + v11 + v12 + v13 + v14 + v15 +
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 + 
                   (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Use volatile to force output */
    volatile int result = checksum;
    (void)result;
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int* arr1, int* arr2, int size) {
    int i;
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    int r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    for (i = 0; i < size; i++) {
        /* Create delay slot scheduling pressure */
        if (arr1[i] > arr2[i]) {
            r0 = r0 + arr1[i];
            r1 = r1 - arr2[i];
            asm volatile("nop" ::: "memory");
            r2 = r2 * arr1[i];
            r3 = r3 ^ arr2[i];
        } else {
            r4 = r4 + arr2[i];
            r5 = r5 - arr1[i];
            asm volatile("nop" ::: "memory");
            r6 = r6 * arr2[i];
            r7 = r7 ^ arr1[i];
        }
        
        /* Force many live registers */
        r8 = r0 + r1;
        r9 = r2 - r3;
        r10 = r4 * r5;
        r11 = r6 ^ r7;
        r12 = r8 + r9;
        r13 = r10 - r11;
        r14 = r12 * r13;
        r15 = r14 ^ r15;
        
        asm volatile("" ::: "memory");
    }
    
    volatile int result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                         r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    (void)result;
}
#endif

int main() {
    const int SIZE = 256;
    int arr1[SIZE];
    int arr2[SIZE];
    int i;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        arr2[i] = (seed >> 16) & 0x7FFF;
    }
    
    /* Call the scheduling-intensive kernel */
#ifdef __mips__
    mips_specific_kernel(arr1, arr2, SIZE);
#else
    complex_scheduling_kernel(arr1, arr2, SIZE, 10000);
#endif
    
    /* Additional calls to increase compilation unit size */
    for (i = 0; i < 3; i++) {
#ifdef __mips__
        mips_specific_kernel(arr1, arr2, SIZE / 2);
#else
        complex_scheduling_kernel(arr1 + i * 64, arr2 + i * 64, 64, 5000);
#endif
    }
    
    return 0;
}
