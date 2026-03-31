/* Compile with: gcc -O3 -fschedule-insns -fno-omit-frame-pointer -fdump-rtl-sched -fdump-rtl-sched2 -march=nehalem -mtune=nehalem -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force use of specific scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile int *volatile_ptr = arr1; /* Prevent optimizations */
    
    /* Initialize variables with non-trivial values */
    v1 = *volatile_ptr;
    v2 = v1 * 2;
    v3 = v2 + 1;
    v4 = v3 ^ 0x55AA55AA;
    v5 = v4 >> 3;
    f1 = (float)v1 * 0.5f;
    f2 = f1 + 3.14159f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch creating unpredictable control flow */
        if (__builtin_expect((arr1[i] & 0x7F) > 64, 0)) {
            /* Path 1: Integer-heavy operations */
            v6 = arr1[i] * v1;
            v7 = v6 + arr2[i];
            v8 = v7 ^ v2;
            v9 = v8 * v3;
            v10 = v9 - v4;
            v11 = v10 >> (arr1[i] & 0x3);
            v12 = v11 | v5;
            
            /* Floating point operations mixed in */
            f3 = (float)v6 * f1;
            f4 = f3 / f2;
            f5 = f4 + (float)v7;
            
            /* Memory barrier forcing serialization */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v13 = v12 + (int)f5;
            v14 = v13 * 2;
            v15 = v14 ^ 0x12345678;
            
            /* Complex dependency chain */
            v1 = v15 + v1;
            v2 = v2 ^ v14;
            v3 = v3 * v13;
            
        } else {
            /* Path 2: Different operation mix */
            v6 = arr1[i] + v1;
            v7 = v6 - arr2[i];
            v8 = v7 & v2;
            v9 = v8 / (v3 | 1); /* Avoid division by zero */
            v10 = v9 ^ v4;
            v11 = v10 << (arr2[i] & 0x3);
            v12 = v11 & v5;
            
            /* Different floating point pattern */
            f3 = (float)v6 + f1;
            f4 = f3 * f2;
            f5 = f4 - (float)v7;
            f6 = f5 * 2.0f;
            f7 = f6 / 3.0f;
            f8 = f7 + f1;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* Different post-barrier operations */
            v13 = v12 - (int)f8;
            v14 = v13 / 2;
            v15 = v14 | 0x87654321;
            
            /* Alternative dependency chain */
            v1 = v1 ^ v15;
            v2 = v2 + v14;
            v4 = v4 * v13;
            f1 = f1 + f8;
        }
        
        /* Converge point with more operations */
        v5 = v5 + v15;
        f2 = f2 * 1.01f;
        
        /* Occasional extra barrier to increase scheduling complexity */
        if ((i & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                   (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    
    /* Volatile store to ensure computation isn't optimized away */
    volatile int result = checksum;
    (void)result; /* Suppress unused warning */
}

/* Additional test case with switch-based control flow */
#ifdef __mips__
/* MIPS-specific patterns if cross-compiling */
__attribute__((noinline))
static void mips_specific_pattern(int *arr, int size) {
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    for (int i = 0; i < size; i++) {
        switch (arr[i] & 0x3) {
            case 0:
                a = arr[i] * 3;
                b = a + i;
                asm volatile("" ::: "memory");
                c = b ^ 0xFF;
                break;
            case 1:
                d = arr[i] / 2;
                e = d - i;
                asm volatile("" ::: "memory");
                f = e | 0xAA;
                break;
            case 2:
                a = a + d;
                b = b ^ e;
                asm volatile("" ::: "memory");
                c = c * f;
                break;
            default:
                d = d - a;
                e = e + b;
                asm volatile("" ::: "memory");
                f = f ^ c;
                break;
        }
    }
    volatile int res = a + b + c + d + e + f;
    (void)res;
}
#endif

int main() {
    const int SIZE = 512;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Multiple calls to increase scheduling opportunities */
    for (int iter = 0; iter < 100; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            array1[i] = (array1[i] * 3 + 1) & 0x7FFF;
        }
    }
    
#ifdef __mips__
    mips_specific_pattern(array1, SIZE);
#endif
    
    free(array1);
    free(array2);
    
    printf("Test completed (check RTL dumps for scheduler activity)\n");
    return 0;
}
