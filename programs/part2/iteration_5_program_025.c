/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o trigger haifa_sched_trigger.c
 */

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
    v0 = *arr1;
    v1 = *arr2;
    mem_barrier = v0 + v1;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        if (__builtin_expect((arr1[i] ^ arr2[i]) & 1, 0)) {
            /* Path A: Integer-heavy computation */
            v2 = arr1[i] * 3;
            v3 = arr2[i] / 7;
            v4 = v2 ^ v3;
            v5 = v4 << 2;
            v6 = v5 | 0x7F;
            v7 = v6 - arr1[i];
            v8 = v7 * v3;
            v9 = v8 >> 4;
            v10 = v9 & 0xFF;
            v11 = v10 + arr2[i];
            v12 = v11 * 2;
            v13 = v12 - v5;
            v14 = v13 ^ v8;
            v15 = v14 + v9;
            
            /* Mix in floating point to use different functional units */
            f0 = (float)v2 * 1.5f;
            f1 = (float)v3 * 2.7f;
            f2 = f0 + f1;
            f3 = f2 * 0.9f;
            f4 = f3 - f0;
            f5 = f4 / f1;
            f6 = f5 * 3.14159f;
            f7 = f6 + f2;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* Use results to prevent dead code elimination */
            arr1[i] = v15 + (int)f7;
        } else {
            /* Path B: Different instruction mix */
            v2 = arr1[i] + 17;
            v3 = arr2[i] - 23;
            v4 = v2 & v3;
            v5 = v4 | 0x3F;
            v6 = v5 * 11;
            v7 = v6 ^ arr1[i];
            v8 = v7 / 3;
            v9 = v8 << 1;
            v10 = v9 + arr2[i];
            v11 = v10 & 0x7FFF;
            v12 = v11 * v8;
            v13 = v12 >> 3;
            v14 = v13 ^ v6;
            v15 = v14 - v9;
            
            /* Different FP sequence */
            f0 = (float)v2 / 4.0f;
            f1 = (float)v3 * 1.8f;
            f2 = f0 - f1;
            f3 = f2 * 2.1f;
            f4 = f3 + f0;
            f5 = f4 / 1.3f;
            f6 = f5 * 0.707f;
            f7 = f6 - f2;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            arr2[i] = v15 - (int)f7;
        }
        
        /* Complex control flow with goto to create CFG complexity */
        if (i & 0x3) {
            goto merge_point;
        }
        
        /* Additional computation on merge path */
        v0 = v0 + v15;
        f0 = f0 + f7;
        
    merge_point:
        /* Use volatile to prevent reordering */
        mem_barrier = arr1[i] + arr2[i];
        
        /* Switch statement to create multiple basic blocks */
        switch (i & 0x7) {
            case 0: v1 = v1 * 2; break;
            case 1: v1 = v1 + 1; break;
            case 2: v1 = v1 - 3; break;
            case 3: v1 = v1 ^ 0xAA; break;
            case 4: v1 = v1 | 0x55; break;
            case 5: v1 = v1 & 0xF0; break;
            case 6: v1 = v1 << 1; break;
            case 7: v1 = v1 >> 1; break;
        }
    }
    
    /* Final computation using all variables to prevent optimization */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + 
                   v10 + v11 + v12 + v13 + v14 + v15 + 
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 + 
                   (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Output to prevent elimination */
    printf("Checksum: %d\n", checksum);
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int v0, v1, v2, v3, v4, v5;
    for (int i = 0; i < size; i++) {
        /* MIPS delay slot patterns */
        if (arr1[i] > arr2[i]) {
            v0 = arr1[i] * 2;
            v1 = arr2[i] + 1;
            asm volatile("nop" ::: "memory");
            v2 = v0 - v1;
        } else {
            v3 = arr1[i] | arr2[i];
            v4 = arr1[i] & arr2[i];
            asm volatile("nop" ::: "memory");
            v5 = v3 ^ v4;
        }
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Call the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#endif
    
    /* Compute final result to ensure all code executes */
    int total = 0;
    for (int i = 0; i < SIZE; i++) {
        total += array1[i] + array2[i];
    }
    printf("Total: %d\n", total);
    
    return 0;
}
