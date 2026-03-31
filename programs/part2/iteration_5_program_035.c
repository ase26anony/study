/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force use of specific scheduling model hooks */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with non-trivial values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f;
    f5 = 5.5f; f6 = 6.6f; f7 = 7.7f; f8 = 8.8f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = __builtin_expect((arr1[i] ^ arr2[i]) & 1, 0);
        
        if (branch_cond) {
            /* Path 1: Integer-heavy operations with memory barriers */
            v1 = v1 + arr1[i];
            v2 = v2 - arr2[i];
            asm volatile("" ::: "memory"); /* Serialization point */
            v3 = v3 * (arr1[i] | 1);
            v4 = v4 ^ arr2[i];
            v5 = (v5 << 3) | (v5 >> 29);
            v6 = v6 + (arr1[i] & 0xFF);
            
            /* Mix float operations to use different functional units */
            f1 = f1 * 1.01f;
            f2 = f2 / 1.02f;
            
            v7 = v7 + (int)f1;
            v8 = v8 - (int)f2;
            
            /* Complex dependency chain */
            v9 = v9 + v1 * v2;
            v10 = v10 - v3 * v4;
            v11 = v11 ^ (v5 * v6);
            v12 = v12 | (v7 ^ v8);
            
            /* Memory barrier to force scheduler state save */
            mem_barrier = v9;
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v13 = v13 + (v10 >> 2);
            v14 = v14 * (v11 & 0x7F);
            v15 = v15 ^ (v12 | 0x80);
            
            f3 = f3 + f1 * 0.5f;
            f4 = f4 - f2 * 0.25f;
            
            /* Jump to common label creating CFG complexity */
            goto merge_point;
        } else {
            /* Path 2: Different operation mix */
            v1 = v1 - arr2[i];
            v2 = v2 + arr1[i];
            asm volatile("" ::: "memory");
            v3 = v3 ^ (arr2[i] & 0xF);
            v4 = v4 | (arr1[i] & 0xF0);
            v5 = (v5 >> 2) | (v5 << 30);
            v6 = v6 - (arr2[i] % 31);
            
            /* Different float operations */
            f5 = f5 + 2.0f;
            f6 = f6 - 1.0f;
            
            v7 = v7 * (int)f5;
            v8 = v8 / ((int)f6 | 1);
            
            /* Alternative dependency chain */
            v9 = v9 ^ v1;
            v10 = v10 | v2;
            v11 = v11 + v3 * v4;
            v12 = v12 - v5 ^ v6;
            
            /* Different barrier point */
            mem_barrier = v10;
            asm volatile("" ::: "memory");
            
            v13 = v13 * (v7 & 0x3F);
            v14 = v14 + (v8 >> 1);
            v15 = v15 | (v9 ^ v10);
            
            f7 = f7 * f5;
            f8 = f8 / f6;
            
            /* Fall through to merge point */
        }
        
    merge_point:
        /* Common computation merging both paths */
        /* Switch statement to create basic block complexity */
        switch (i & 0x3) {
            case 0:
                v1 = v1 + v13;
                v2 = v2 - v14;
                f1 = f1 + f3;
                f2 = f2 - f4;
                break;
            case 1:
                v3 = v3 * v15;
                v4 = v4 ^ v1;
                f3 = f3 * f5;
                f4 = f4 / f6;
                break;
            case 2:
                v5 = v5 | v2;
                v6 = v6 & v3;
                f5 = f5 + f7;
                f6 = f6 - f8;
                break;
            default:
                v7 = v7 ^ v4;
                v8 = v8 | v5;
                f7 = f7 * f1;
                f8 = f8 / f2;
                /* Use goto to create loop back-edge complexity */
                if (__builtin_expect((v7 & 1), 0)) {
                    goto special_handling;
                }
                break;
        }
        
        continue;
        
    special_handling:
        /* Rare path with different operations */
        v9 = v9 * 3;
        v10 = v10 / 2;
        f1 = f1 * 2.0f;
        f2 = f2 / 2.0f;
        /* Jump back */
        if (v9 > 0) goto merge_point;
    }
    
    /* Use all variables to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + 
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                   (int)f5 + (int)f6 + (int)f7 + (int)f8;
    
    /* Volatile store to force all computations */
    volatile int *result = &mem_barrier;
    *result = checksum;
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    volatile int barrier;
    
    for (int i = 0; i < size; i++) {
        /* MIPS delay slot patterns */
        if (arr1[i] > arr2[i]) {
            v1 = v1 + arr1[i];
            asm volatile("" ::: "memory");
            v2 = v2 - arr2[i];
            /* Force delay slot scheduling */
            asm volatile("nop" ::: "memory");
        } else {
            v3 = v3 * arr1[i];
            asm volatile("" ::: "memory");
            v4 = v4 ^ arr2[i];
        }
        
        /* Complex enough to trigger state saving */
        for (int j = 0; j < 4; j++) {
            v1 = v1 + (v2 >> j);
            v2 = v2 - (v3 << j);
            v3 = v3 ^ (v4 & (1 << j));
            v4 = v4 | (v1 % (j + 2));
        }
        
        barrier = v1;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        /* Simple LCG */
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        array2[i] = (seed >> 8) & 0xFF;
    }
    
    /* Execute the scheduling-intensive kernel */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
#endif
    
    free(array1);
    free(array2);
    
    return 0;
}
