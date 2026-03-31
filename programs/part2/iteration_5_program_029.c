/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For cross-compilation to MIPS: mips-linux-gnu-gcc -O3 -mips32 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

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
static void complex_scheduling_kernel(volatile int* arr1, volatile int* arr2, int size) {
    /* Create high register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    int* volatile ptr = (int*)arr1; /* volatile pointer to prevent optimizations */
    
    /* Initialize variables to create dependencies */
    v1 = *ptr;
    v2 = v1 * 2;
    v3 = v2 + 1;
    v4 = v3 ^ 0x55AA55AA;
    v5 = v4 >> 3;
    f1 = (float)v1 * 0.5f;
    f2 = f1 + 3.14f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch to create unpredictable control flow */
        if (__builtin_expect((arr1[i] & 0x7F) > 64, 0)) {
            /* Path A: Integer-heavy operations */
            v6 = arr1[i] * v1;
            v7 = v6 + arr2[i];
            v8 = v7 ^ v2;
            v9 = v8 * v3;
            v10 = v9 - v4;
            v11 = v10 >> (arr1[i] & 0x3);
            v12 = v11 | v5;
            
            /* Floating-point operations mixed in */
            f3 = (float)arr1[i] * f1;
            f4 = f3 / f2;
            f5 = f4 + (float)arr2[i];
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            v13 = v12 * (arr2[i] + 1);
            v14 = v13 ^ (v6 + v7);
            v15 = v14 & 0xFFFFFFFF;
            
            f6 = f5 * 2.0f;
            f7 = f6 - f1;
            f8 = f7 / 3.14159f;
            
            /* Complex control flow with goto to create basic block merging */
            if (v15 > 1000) {
                v1 = v15;
                goto merge_point;
            }
        } else {
            /* Path B: Different operation mix */
            v6 = arr1[i] + v1;
            v7 = v6 * arr2[i];
            v8 = v7 | v2;
            v9 = v8 ^ v3;
            v10 = v9 - v4;
            v11 = v10 << (arr2[i] & 0x3);
            v12 = v11 & v5;
            
            /* Different FP operations */
            f3 = (float)arr2[i] + f1;
            f4 = f3 * f2;
            f5 = f4 / (float)(arr1[i] + 1);
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            v13 = v12 + (arr1[i] * 2);
            v14 = v13 ^ (v8 * v9);
            v15 = v14 | 0xAAAAAAAA;
            
            f6 = f5 + 1.0f;
            f7 = f6 * f1;
            f8 = f7 - 2.71828f;
            
            if (v15 < 500) {
                v2 = v15;
                goto merge_point;
            }
        }
        
        /* Default path continuation */
        v1 = v1 + arr1[i];
        v2 = v2 - arr2[i];
        
    merge_point:
        /* Common merge point with operations on all variables */
        v3 = v3 ^ v15;
        v4 = v4 + v13;
        v5 = v5 | v14;
        
        f1 = f1 + f8;
        f2 = f2 * f7;
        
        /* Switch statement to create additional control flow complexity */
        switch (i & 0x3) {
            case 0:
                v6 = v6 * 3;
                f3 = f3 + 1.0f;
                break;
            case 1:
                v7 = v7 / 2;
                f4 = f4 * 2.0f;
                break;
            case 2:
                v8 = v8 ^ 0x12345678;
                f5 = f5 - 0.5f;
                break;
            case 3:
                v9 = v9 + 100;
                f6 = f6 / 1.5f;
                /* Force potential state save with volatile asm */
                asm volatile("" ::: "memory");
                break;
        }
        
        /* Rotate variables to maintain live ranges */
        int temp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = temp;
        
        float ftemp = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6;
        f6 = f7; f7 = f8; f8 = ftemp;
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                           v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 +
                           (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    
    /* Use checksum in a way that can't be optimized away */
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    }
}

/* Secondary function to create more scheduling context */
__attribute__((noinline))
static void create_scheduling_pressure(int iterations) {
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize with pseudo-random values using LCG */
    uint32_t seed = 42;
    for (int i = 0; i < 256; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        array2[i] = (seed >> 8) & 0x7FFF;
    }
    
    /* Multiple calls to increase chance of state saving */
    for (int j = 0; j < iterations; j++) {
        complex_scheduling_kernel((int*)array1, (int*)array2, 256);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < 256; i++) {
            array1[i] = array1[i] + j;
            array2[i] = array2[i] - j;
        }
    }
}

int main() {
    printf("Starting scheduler stress test...\n");
    
    /* Call multiple times with different parameters */
    create_scheduling_pressure(10);
    create_scheduling_pressure(5);
    create_scheduling_pressure(8);
    
    /* Additional complex loop to increase scheduling pressure */
    volatile int counter = 0;
    for (int i = 0; i < 1000; i++) {
        int* volatile dynamic_array = (int*)malloc(128 * sizeof(int));
        if (dynamic_array) {
            for (int j = 0; j < 128; j++) {
                dynamic_array[j] = i * j;
            }
            
            /* Force scheduling complexity with function pointer */
            void (* volatile func)(int*, int*, int) = complex_scheduling_kernel;
            func((int*)dynamic_array, (int*)dynamic_array, 64);
            
            free(dynamic_array);
        }
        counter += i;
    }
    
    printf("Test completed. Counter: %d\n", counter);
    return 0;
}
