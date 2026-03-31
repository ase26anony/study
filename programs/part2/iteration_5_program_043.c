/* haifa_sched_trigger.c
 * Program designed to trigger GCC Haifa scheduler state save/restore
 * and execute the cleanup logic in haifa-sched.cc lines 4681-4691
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__aarch64__)
/* AArch64 has complex scheduling models */
#elif defined(__mips__)
/* MIPS often has delay slots requiring scheduler backtracking */
#endif
static void compute_intensive_kernel(int *arr1, int *arr2, int size, int *result) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int *volatile_ptr = arr1; /* Prevent optimization */
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = *volatile_ptr;
    v1 = v0 ^ 0x55555555;
    v2 = v1 + 1;
    v3 = v2 * 3;
    v4 = v3 - v1;
    v5 = v4 & 0xAAAAAAAA;
    v6 = v5 | v2;
    v7 = v6 ^ v3;
    v8 = v7 << 2;
    v9 = v8 >> 1;
    v10 = v9 + v4;
    v11 = v10 - v5;
    v12 = v11 * v6;
    v13 = v12 / (v7 ? v7 : 1);
    v14 = v13 & v8;
    v15 = v14 | v9;
    
    f0 = (float)v0 * 0.5f;
    f1 = f0 + 1.0f;
    f2 = f1 * 2.0f;
    f3 = f2 - f0;
    f4 = f3 / (f1 ? f1 : 1.0f);
    f5 = f4 * 3.14f;
    f6 = f5 + f2;
    f7 = f6 - f3;
    
    /* Complex control flow with data-dependent branches */
    for (int i = 0; i < size; i++) {
        /* Hard-to-predict branch using array data */
        if (__builtin_expect((arr1[i] & 0x7) > 4, 0)) {
            /* Path A: Integer-heavy operations */
            v0 = arr1[i] + v15;
            v1 = v0 ^ arr2[i];
            v2 = v1 * v14;
            v3 = v2 - v13;
            v4 = v3 & v12;
            v5 = v4 | v11;
            v6 = v5 ^ v10;
            v7 = v6 + v9;
            v8 = v7 * v8;
            v9 = v8 - v7;
            v10 = v9 & v6;
            v11 = v10 | v5;
            v12 = v11 ^ v4;
            v13 = v12 + v3;
            v14 = v13 * v2;
            v15 = v14 - v1;
            
            /* Floating-point operations mixed in */
            f0 = (float)v0 * 0.25f;
            f1 = f0 + f7;
            f2 = f1 * f6;
            f3 = f2 - f5;
            f4 = f3 / f4;
            f5 = f4 * f3;
            f6 = f5 + f2;
            f7 = f6 - f1;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Another hard-to-predict branch inside the path */
            if (__builtin_expect((v15 & 1) == 0, 1)) {
                v0 = v15 * 2;
                v1 = v0 + 1;
            } else {
                v0 = v15 / 2;
                v1 = v0 - 1;
            }
        } else {
            /* Path B: Different operation mix */
            v0 = arr1[i] - v15;
            v1 = v0 | arr2[i];
            v2 = v1 & v14;
            v3 = v2 + v13;
            v4 = v3 ^ v12;
            v5 = v4 - v11;
            v6 = v5 | v10;
            v7 = v6 & v9;
            v8 = v7 ^ v8;
            v9 = v8 + v7;
            v10 = v9 | v6;
            v11 = v10 & v5;
            v12 = v11 ^ v4;
            v13 = v12 - v3;
            v14 = v13 | v2;
            v15 = v14 & v1;
            
            /* Different FP operation sequence */
            f0 = (float)v0 * 0.75f;
            f1 = f0 - f7;
            f2 = f1 / (f6 ? f6 : 1.0f);
            f3 = f2 + f5;
            f4 = f3 * f4;
            f5 = f4 - f3;
            f6 = f5 / (f2 ? f2 : 1.0f);
            f7 = f6 + f1;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            /* Switch-like control flow */
            switch (v15 & 0x3) {
                case 0:
                    v0 = v15 << 1;
                    v1 = v0 ^ 0xFF;
                    break;
                case 1:
                    v0 = v15 >> 1;
                    v1 = v0 | 0xAA;
                    break;
                case 2:
                    v0 = v15 * 3;
                    v1 = v0 & 0x55;
                    break;
                default:
                    v0 = v15 + 7;
                    v1 = v0 - 3;
                    /* goto creates complex CFG */
                    if (v1 > 1000) goto common_label;
                    break;
            }
        }
        
common_label:
        /* Common convergence point with more operations */
        v2 = v0 + v1;
        v3 = v2 * arr1[i];
        v4 = v3 - arr2[i];
        
        /* Force register spilling with many live variables */
        f0 = f0 + (float)v2;
        f1 = f1 - (float)v3;
        f2 = f2 * (float)v4;
        f3 = f3 / (f0 ? f0 : 1.0f);
        f4 = f4 + f1;
        f5 = f5 - f2;
        f6 = f6 * f3;
        f7 = f7 / (f4 ? f4 : 1.0f);
        
        /* Another barrier before loop continues */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    sum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
    *result = sum;
}

/* Alternate version for MIPS with explicit delay slot patterns */
#ifdef __mips__
__attribute__((noinline, naked))
static void mips_delay_slot_pattern(int *arr, int size) {
    /* MIPS-specific code that might trigger scheduler backtracking */
    asm volatile (
        ".set noreorder\n"
        "1:\n"
        "lw $t0, 0(%0)\n"
        "addiu %0, %0, 4\n"
        "bnez $t0, 1b\n"
        "addiu %1, %1, -1  # Delay slot\n"
        ".set reorder\n"
        : "+r"(arr), "+r"(size)
        :
        : "t0", "memory"
    );
}
#endif

int main(void) {
    const int SIZE = 256;
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    int result = 0;
    
    /* Multiple iterations to increase scheduling pressure */
    for (int iter = 0; iter < 100; iter++) {
        compute_intensive_kernel(arr1, arr2, SIZE, &result);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] * 13 + 7) & 0xFFFF;
            arr2[i] = (arr2[i] * 17 + 11) & 0xFFFF;
        }
    }
    
#ifdef __mips__
    /* Include MIPS-specific pattern if compiling for MIPS */
    mips_delay_slot_pattern(arr1, SIZE);
#endif
    
    printf("Result checksum: %d\n", result);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
