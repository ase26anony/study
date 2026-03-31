/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force use of specific scheduling model hooks */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Initialize with non-trivial values */
    v1 = arr1[0] ^ 0x55AA55AA;
    v2 = arr2[0] | 0x33CC33CC;
    v3 = v1 * v2;
    v4 = v1 + v2;
    v5 = v1 - v2;
    v6 = v1 ^ v2;
    v7 = v3 & v4;
    v8 = v5 | v6;
    
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = f1 + f2;
    f4 = f1 - f2;
    f5 = f1 * f2;
    
    for (int i = 1; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = arr1[i] & arr2[i];
        int likely_unpredictable = __builtin_expect((branch_cond & 0xFF) > 128, 0);
        
        if (likely_unpredictable) {
            /* Path A: Integer-heavy operations with many dependencies */
            v9 = arr1[i] * v1 + v2;
            v10 = arr2[i] * v3 - v4;
            v11 = v9 ^ v10;
            v12 = v9 & v10;
            v13 = v11 | v12;
            v14 = v13 << 3;
            v15 = v14 >> 1;
            
            /* Floating-point operations mixed in */
            f6 = (float)v9 * 0.25f;
            f7 = (float)v10 * 0.75f;
            f8 = f6 + f7 - f3;
            
            /* Chain dependencies */
            v1 = v15 + v5;
            v2 = v13 - v6;
            v3 = v9 * v10 / (v11 + 1);
            v4 = (v12 << 2) | (v13 >> 2);
            v5 = v14 ^ v15;
            
            f1 = f8 * 2.0f;
            f2 = f6 - f7;
            f3 = f1 + f2;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Force register spilling with more operations */
            v6 = v1 + v2 + v3 + v4 + v5;
            v7 = v1 * v2 * v3;
            v8 = v4 | v5 | v6;
            
        } else {
            /* Path B: Different operation mix with same variables */
            v9 = arr1[i] + v1 * 2;
            v10 = arr2[i] - v2 / 2;
            v11 = v9 | v10;
            v12 = v9 & ~v10;
            v13 = v11 ^ v12;
            v14 = v13 * 3;
            v15 = v14 % 257;
            
            /* Different FP operations */
            f6 = (float)v9 * 1.5f;
            f7 = (float)v10 * 2.5f;
            f8 = f6 - f7 + f4;
            
            /* Different dependency chain */
            v1 = v15 ^ v5;
            v2 = v13 & v6;
            v3 = v9 + v10 - v11;
            v4 = (v12 << 1) & (v13 >> 1);
            v5 = v14 | v15;
            
            f1 = f8 / 2.0f;
            f2 = f6 + f7;
            f4 = f1 - f2;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* More operations for register pressure */
            v6 = v1 * v2 - v3;
            v7 = v4 ^ v5 ^ v6;
            v8 = v7 << 1;
        }
        
        /* Common merge point with complex switch */
        switch (i & 0x7) { /* 8 different cases */
            case 0:
                v1 = v1 + v8;
                f1 = f1 + 1.0f;
                goto common_label;
            case 1:
                v2 = v2 - v7;
                f2 = f2 - 1.0f;
                goto common_label;
            case 2:
                v3 = v3 * 2;
                f3 = f3 * 2.0f;
                goto common_label;
            case 3:
                v4 = v4 | 0xFF;
                f4 = f4 / 2.0f;
                goto common_label;
            case 4:
                v5 = v5 ^ 0xAA;
                f5 = f5 + f1;
                goto common_label;
            case 5:
                v6 = v6 & 0x55;
                f6 = f6 - f2;
                goto common_label;
            case 6:
                v7 = v7 << 1;
                f7 = f7 * 1.5f;
                goto common_label;
            case 7:
                v8 = v8 >> 1;
                f8 = f8 / 1.5f;
                /* fall through */
            common_label:
                /* Complex operation using all variables */
                barrier = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
                f1 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
                break;
        }
        
        /* Occasionally use volatile to prevent reordering */
        if ((i & 0xF) == 0) {
            volatile int* ptr = &barrier;
            *ptr = v1;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final computation to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                   (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    
    /* Use checksum to prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    }
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* MIPS delay slot patterns */
    for (int i = 0; i < size; i++) {
        if (arr1[i] > arr2[i]) {
            r1 = arr1[i] << 2;
            r2 = arr2[i] >> 2;
            /* Force scheduler to handle delay slots */
            asm volatile("nop" ::: "memory");
        } else {
            r1 = arr1[i] >> 2;
            r2 = arr2[i] << 2;
            asm volatile("nop" ::: "memory");
        }
        
        r3 = r1 + r2;
        r4 = r1 - r2;
        r5 = r3 * r4;
        
        /* Complex dependency web */
        r6 = r5 & 0xFF;
        r7 = r6 | 0x55;
        r8 = r7 ^ r6;
        r9 = r8 << 1;
        r10 = r9 >> 1;
        
        asm volatile("" ::: "memory");
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
    
    /* Call the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
    /* MIPS-specific version if needed */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#endif
    
    free(array1);
    free(array2);
    
    return 0;
}
