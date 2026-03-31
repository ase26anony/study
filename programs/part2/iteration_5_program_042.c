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
static int complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    mem_barrier = arr1[0];
    v0 = mem_barrier;
    v1 = arr2[0] ^ 0x55AA55AA;
    
    /* Complex control flow with data-dependent branches */
    for (int i = 0; i < size; i++) {
        int idx = i & 255;
        int branch_cond = arr1[idx] ^ arr2[idx];
        
        /* Unpredictable branch using runtime data */
        if (__builtin_expect((branch_cond & 0xF0F0F0F0) != 0, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v2 = v0 + arr1[idx];
            v3 = v1 * arr2[idx];
            v4 = v2 ^ v3;
            v5 = v4 << (arr1[idx] & 7);
            v6 = v5 - v3;
            v7 = v6 | v4;
            v8 = v7 & 0x7FFFFFFF;
            v9 = v8 + v5;
            v10 = v9 * 1103515245 + 12345;
            
            /* Floating-point ops mixed with integer */
            f0 = (float)v10 * 0.5f;
            f1 = f0 + (float)v9;
            f2 = f1 * 3.14159f;
            f3 = f2 - f0;
            
            v11 = (int)f3 ^ v10;
            v12 = v11 * v9;
            v13 = v12 >> 4;
            v14 = v13 + v8;
            v15 = v14 ^ v7;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex switch to create CFG merges */
            switch (v15 & 3) {
                case 0: v0 = v15 + 1; break;
                case 1: v1 = v15 - 1; break;
                case 2: v0 = v15 * 2; break;
                default: v1 = v15 / 2; break;
            }
            
            /* Another barrier before path merge */
            asm volatile("" ::: "memory");
            
            /* Jump to common label creating complex predecessor */
            goto common_computation;
        } else {
            /* Path B: Different operation mix with high pressure */
            v2 = v0 - arr1[idx];
            v3 = v1 / (arr2[idx] | 1); /* Avoid division by zero */
            v4 = v2 & v3;
            v5 = v4 >> (arr2[idx] & 7);
            v6 = v5 + v3;
            v7 = v6 ^ v4;
            v8 = v7 | 0x80000000;
            v9 = v8 - v5;
            v10 = v9 ^ 0xDEADBEEF;
            
            /* Different FP sequence */
            f4 = (float)v10 * 1.5f;
            f5 = f4 - (float)v9;
            f6 = f5 * 2.71828f;
            f7 = f6 + f4;
            
            v11 = (int)f7 & v10;
            v12 = v11 + v9;
            v13 = v12 << 3;
            v14 = v13 - v8;
            v15 = v14 | v7;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            /* Different switch pattern */
            switch (v15 & 7) {
                case 0: v0 = v15 | 0xFF; break;
                case 1: v1 = v15 & 0xFF00; break;
                case 2: v0 = v15 ^ 0xAAAA; break;
                case 3: v1 = v15 + 0x1000; break;
                case 4: v0 = v15 - 0x1000; break;
                case 5: v1 = v15 * 3; break;
                case 6: v0 = v15 / 3; break;
                default: v1 = v15 % 17; break;
            }
            
            /* Barrier before merge */
            asm volatile("" ::: "memory");
            
common_computation:
            /* Common computation after both paths - creates convergence */
            f0 = (float)v0 * 0.25f;
            f1 = (float)v1 * 0.75f;
            f2 = f0 + f1;
            f3 = f2 * f2;
            
            /* More operations to increase pressure */
            v2 = v0 + v1;
            v3 = v2 * (int)f3;
            v4 = v3 ^ (arr1[idx] + i);
            v5 = v4 & (arr2[idx] - i);
            
            /* Use volatile function pointer to inhibit optimization */
            static int (*volatile fp)(int) = NULL;
            if (!fp) fp = &complex_scheduling_kernel; /* Self-reference */
            v6 = fp(v5) & 0xFF;
            
            v7 = v6 + v5;
            v8 = v7 << 2;
            v9 = v8 | v4;
            v10 = v9 ^ v3;
            
            /* Final barrier in loop */
            asm volatile("" ::: "memory");
        }
        
        /* Rotate variables to create live range overlaps */
        int tmp = v0;
        v0 = v1;
        v1 = v2;
        v2 = v3;
        v3 = v4;
        v4 = v5;
        v5 = v6;
        v6 = v7;
        v7 = v8;
        v8 = v9;
        v9 = v10;
        v10 = tmp;
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    
    return checksum;
}

/* Secondary kernel with different patterns */
#ifdef __mips__
__attribute__((noinline, optimize("O3")))
#endif
static int alternate_kernel(int *arr, int size) {
    /* Different register pressure pattern */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    float b0, b1, b2, b3, b4;
    
    a0 = arr[0];
    a1 = 1;
    
    for (int i = 1; i < size; i++) {
        /* Nested conditionals */
        if (arr[i] > 0) {
            if (arr[i] & 1) {
                a2 = a0 * a1;
                a3 = a2 + arr[i];
                a4 = a3 ^ a0;
                a5 = a4 << 3;
                
                b0 = (float)a5 * 1.1f;
                b1 = b0 + (float)a4;
                
                asm volatile("" ::: "memory");
                
                /* Small loop inside to create inner block */
                for (int j = 0; j < 2; j++) {
                    a6 = a5 + j;
                    a7 = a6 * (int)b1;
                    a8 = a7 ^ a3;
                }
            } else {
                a2 = a0 / (a1 | 1);
                a3 = a2 - arr[i];
                a4 = a3 & a0;
                a5 = a4 >> 1;
                
                b0 = (float)a5 * 0.9f;
                b1 = b0 - (float)a4;
                
                asm volatile("" ::: "memory");
            }
            
            /* Merge point */
            a9 = a5 + (int)b1;
            a10 = a9 * a2;
        } else {
            a2 = a0 + a1;
            a3 = a2 ^ arr[i];
            a4 = a3 | 0x12345678;
            a5 = a4 - a0;
            
            b2 = (float)a5 * 2.0f;
            b3 = b2 / (float)(a1 + 1);
            
            asm volatile("" ::: "memory");
            
            a9 = a5 * (int)b3;
            a10 = a9 & a4;
        }
        
        /* More operations with many live variables */
        a11 = a10 + a9 + a8 + a7 + a6 + a5 + a4 + a3 + a2;
        a12 = a11 ^ a0;
        
        /* Rotate */
        a0 = a1;
        a1 = a2;
        a2 = a3;
        a3 = a4;
        a4 = a5;
        a5 = a6;
        a6 = a7;
        a7 = a8;
        a8 = a9;
        a9 = a10;
        a10 = a11;
        a11 = a12;
        
        asm volatile("" ::: "memory");
    }
    
    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 +
           (int)(b0 + b1 + b2 + b3 + b4);
}

int main() {
    const int SIZE = 1024;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFFFFFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Run multiple kernels to increase scheduling opportunities */
    int result1 = complex_scheduling_kernel(array1, array2, SIZE);
    int result2 = alternate_kernel(array1, SIZE);
    int result3 = complex_scheduling_kernel(array2, array1, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d\n", result1, result2, result3);
    printf("Checksum: %d\n", result1 ^ result2 ^ result3);
    
    free(array1);
    free(array2);
    
    return 0;
}
