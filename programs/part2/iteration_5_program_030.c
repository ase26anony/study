/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    mem_barrier = *arr1;
    v0 = mem_barrier;
    v1 = *arr2;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        if (__builtin_expect((arr1[i] ^ arr2[i]) & 1, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v2 = arr1[i] + v0;
            v3 = arr2[i] - v1;
            v4 = v2 * v3;
            v5 = v4 ^ v0;
            v6 = v5 >> 3;
            v7 = v6 & 0xFF;
            v8 = v7 * v2;
            v9 = v8 + v3;
            v10 = v9 - v4;
            v11 = v10 ^ v5;
            v12 = v11 * 0x5A5A5A5A;
            
            /* Floating point ops mixed with integer */
            f0 = (float)v2 * 1.5f;
            f1 = (float)v3 * 2.5f;
            f2 = f0 + f1;
            f3 = f2 * 0.75f;
            
            /* Memory barrier to force scheduler serialization point */
            asm volatile("" ::: "memory");
            
            /* More computations after barrier */
            v13 = (int)f3 + v12;
            v14 = v13 * v11;
            v15 = v14 ^ v10;
            
            f4 = f3 * 2.0f;
            f5 = f4 - f2;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            f6 = f5 + f0;
            f7 = f6 * 0.5f;
            
            v0 = v15 + (int)f7;
            v1 = v0 ^ arr1[i];
        } else {
            /* Path B: Different computation pattern with same variables */
            v2 = arr1[i] * 3;
            v3 = arr2[i] / 2;
            v4 = v2 | v3;
            v5 = v4 & 0xAAAAAAAA;
            v6 = v5 << 2;
            v7 = v6 + v2;
            v8 = v7 - v3;
            v9 = v8 ^ v4;
            v10 = v9 * 0x33333333;
            v11 = v10 >> 1;
            v12 = v11 & 0x7F;
            
            /* Different floating point pattern */
            f0 = (float)arr1[i] / 3.0f;
            f1 = (float)arr2[i] / 4.0f;
            f2 = f0 - f1;
            f3 = f2 * 4.0f;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            v13 = (int)f3 | v12;
            v14 = v13 + v11;
            v15 = v14 ^ v10;
            
            f4 = f3 / 2.0f;
            f5 = f4 + f2;
            
            /* Complex control flow with goto to create CFG complexity */
            if (v15 & 1) {
                f6 = f5 * 3.0f;
                goto merge_point;
            } else {
                f6 = f5 * 1.5f;
            }
            
            /* Another barrier before merge */
            asm volatile("" ::: "memory");
            
        merge_point:
            f7 = f6 - f0;
            
            v0 = v15 * (int)f7;
            v1 = v0 | arr2[i];
        }
        
        /* Common post-processing with switch for additional complexity */
        switch (i & 3) {
            case 0:
                v0 = v0 + v1;
                v1 = v1 * 2;
                break;
            case 1:
                v0 = v0 ^ v1;
                v1 = v1 + 1;
                break;
            case 2:
                v0 = v0 | v1;
                v1 = v1 - 1;
                break;
            case 3:
                v0 = v0 & v1;
                v1 = v1 >> 1;
                break;
        }
        
        /* Final barrier in loop */
        asm volatile("" ::: "memory");
        
        /* Force register spilling by using all variables */
        arr1[i] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                  v11 + v12 + v13 + v14 + v15 + (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^ 
                   v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ (int)f0 ^ (int)f1 ^ (int)f2 ^ 
                   (int)f3 ^ (int)f4 ^ (int)f5 ^ (int)f6 ^ (int)f7;
    
    /* Volatile store to force computation */
    mem_barrier = checksum;
}

/* MIPS-specific version with delay slot patterns */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_pattern(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        /* Pattern that might engage MIPS delay slot scheduling */
        asm volatile(
            "lw $0, %0\n\t"
            "nop\n\t"
            "addu $0, $0, %1\n\t"
            : : "m"(arr[i]), "r"(i) : "memory"
        );
        result += arr[i];
    }
    arr[0] = result;
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
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        array2[i] = (int)(seed >> 8) & 0x7FFF;
    }
    
    /* Call the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    /* Include MIPS-specific code if compiling for MIPS */
    mips_specific_pattern(array1, SIZE);
#endif
    
    /* Compute and print checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum ^= array1[i];
        final_sum += array2[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}
