/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile int *volatile_ptr = arr1; /* Prevent optimizations */
    
    /* Initialize variables to create dependencies */
    v1 = *volatile_ptr;
    v2 = v1 + 1;
    v3 = v2 * 2;
    v4 = v3 ^ 0x55AA55AA;
    v5 = v4 - v1;
    f1 = (float)v1 * 0.5f;
    f2 = f1 + 3.14f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = __builtin_expect((arr1[i] & arr2[i]) > (i * 7), 0);
        
        if (branch_cond) {
            /* Path A: Integer-heavy operations */
            v6 = arr1[i] + v1;
            v7 = arr2[i] - v2;
            v8 = v6 * v7;
            v9 = v8 ^ v3;
            v10 = v9 >> 3;
            v11 = v10 | v4;
            v12 = v11 & 0x0F0F0F0F;
            v13 = v12 + v5;
            v14 = v13 * 2;
            v15 = v14 - v6;
            
            /* Floating-point ops mixed in */
            f3 = (float)v6 * 1.5f;
            f4 = f3 / f1;
            f5 = f4 + f2;
            
            /* Memory barrier creates serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex dependency chain */
            v1 = v15 + (int)f5;
            v2 = v1 ^ arr1[i];
            v3 = v2 * 3;
        } else {
            /* Path B: Different operation mix */
            v6 = arr1[i] ^ v1;
            v7 = arr2[i] | v2;
            v8 = v7 - v3;
            v9 = v8 * 5;
            v10 = v9 & v4;
            v11 = v10 + v5;
            v12 = v11 >> 2;
            v13 = v12 ^ 0x33333333;
            v14 = v13 * 7;
            v15 = v14 - v6;
            
            /* More floating-point operations */
            f3 = (float)v7 * 2.7f;
            f4 = f3 - f1;
            f5 = f4 * f2;
            f6 = f5 / 2.0f;
            f7 = f6 + 1.0f;
            f8 = f7 * 0.5f;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Different dependency pattern */
            v1 = v15 ^ (int)f8;
            v2 = v1 + arr2[i];
            v3 = v2 & 0x00FF00FF;
        }
        
        /* Common merge point with switch for additional complexity */
        switch (i & 3) {
            case 0:
                v4 = v3 + v15;
                f1 = f3 * 2.0f;
                goto common_label;
            case 1:
                v4 = v3 - v15;
                f1 = f3 / 2.0f;
                goto common_label;
            case 2:
                v4 = v3 ^ v15;
                f1 = f3 + 2.0f;
                /* Fall through */
            default:
                v4 = v3 * 2;
                f1 = f3 - 2.0f;
                break;
        }
        
    common_label:
        /* More operations at merge point */
        v5 = v4 + i;
        f2 = f1 * (float)v5;
        
        /* Force register spilling with many live variables */
        arr1[i] = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                  v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                  (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^ 
                   v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ (int)f1 ^ (int)f2 ^ 
                   (int)f3 ^ (int)f4 ^ (int)f5 ^ (int)f6 ^ (int)f7 ^ (int)f8;
    
    /* Volatile store to force computation */
    *volatile_ptr = checksum;
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr, int size) {
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create delay slot pressure */
        if (arr[i] > 0) {
            v1 = arr[i] + v1;
            asm volatile("nop" ::: "memory");
            v2 = v1 * 2;
        } else {
            v3 = arr[i] - v3;
            asm volatile("nop" ::: "memory");
            v4 = v3 ^ 0xFFFF;
        }
        
        /* Force many live variables */
        v5 = v1 + v2 + v3 + v4 + i;
        arr[i] = v5;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        array2[i] = (seed >> 8) & 0x7FFF;
    }
    
    /* Execute the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    mips_specific_kernel(array1, SIZE);
#endif
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum ^= array1[i];
        final_sum += array2[i];
    }
    
    printf("Result checksum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    return 0;
}
