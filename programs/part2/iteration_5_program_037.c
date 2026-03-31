/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -mips32 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS-specific scheduling hooks are more likely to be used */
#else
/* Generic fallback */
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    v0 = *arr1;
    v1 = *arr2;
    mem_barrier = v0 + v1;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with unpredictable pattern */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v2 = arr1[i] * 3;
            v3 = arr2[i] / 7;
            v4 = v2 ^ v3;
            v5 = v4 << 3;
            v6 = v5 - v3;
            v7 = v6 * v2;
            v8 = v7 & 0xFFFF;
            v9 = v8 | 0x1F;
            v10 = v9 + arr1[(i + 1) % size];
            v11 = v10 - arr2[(i + 2) % size];
            v12 = v11 * 13;
            v13 = v12 ^ v11;
            v14 = v13 + v10;
            v15 = v14 * 7;
            
            /* Floating-point ops mixed in */
            f0 = (float)v2 * 1.5f;
            f1 = (float)v3 * 2.5f;
            f2 = f0 + f1;
            f3 = f2 * 0.75f;
            f4 = f3 - f1;
            f5 = f4 / 2.0f;
            f6 = f5 * f3;
            f7 = f6 + f0;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex dependency chain continues */
            v0 = v15 + (int)f7;
            v1 = v0 ^ (int)f3;
        } else {
            /* Path B: Different operation mix */
            v2 = arr1[i] + 17;
            v3 = arr2[i] - 23;
            v4 = v2 * v3;
            v5 = v4 >> 2;
            v6 = v5 | 0x3F;
            v7 = v6 ^ v4;
            v8 = v7 + arr2[(i + 3) % size];
            v9 = v8 - arr1[(i + 4) % size];
            v10 = v9 * 11;
            v11 = v10 & 0xFFF;
            v12 = v11 | v10;
            v13 = v12 ^ v9;
            v14 = v13 + v8;
            v15 = v14 * 5;
            
            /* Different FP sequence */
            f0 = (float)v2 * 0.25f;
            f1 = (float)v3 * 4.0f;
            f2 = f1 - f0;
            f3 = f2 * 3.14f;
            f4 = f3 / 1.618f;
            f5 = f4 + f0;
            f6 = f5 * f2;
            f7 = f6 - f3;
            
            /* Another barrier at different position */
            asm volatile("" ::: "memory");
            
            v0 = v15 - (int)f7;
            v1 = v0 | (int)f4;
        }
        
        /* Merge point with switch to create complex CFG */
        switch (arr1[i] & 0x3) {
            case 0:
                v2 = v0 + v1;
                goto common_label;
            case 1:
                v2 = v0 - v1;
                goto common_label;
            case 2:
                v2 = v0 * v1;
                /* Fall through */
            default:
                v2 = v0 ^ v1;
                common_label:
                v3 = v2 + i;
                break;
        }
        
        /* Store results back to create memory dependencies */
        arr1[i] = v0 + v3;
        arr2[i] = v1 - v3;
        
        /* Volatile access to prevent reordering */
        mem_barrier = arr1[i] + arr2[i];
    }
}

/* Another variant with different register pressure pattern */
__attribute__((noinline))
static int nested_loop_scheduler(int *arr, int size) {
    int sum = 0;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
    
    for (int i = 0; i < size; i++) {
        t0 = arr[i];
        for (int j = 0; j < 8; j++) {
            /* Unrolled inner loop creates dense basic block */
            t1 = t0 + j;
            t2 = t1 * 3;
            t3 = t2 ^ t1;
            t4 = t3 << (j & 3);
            t5 = t4 - t2;
            t6 = t5 * 7;
            t7 = t6 & 0xFF;
            t8 = t7 | 0x1;
            t9 = t8 + t0;
            t10 = t9 - t5;
            t11 = t10 * 11;
            t12 = t11 ^ t9;
            t13 = t12 + t8;
            t14 = t13 * 13;
            t15 = t14 ^ t13;
            
            /* Barrier in inner loop */
            if (j == 4) {
                asm volatile("" ::: "memory");
            }
            
            t0 = t15;
        }
        sum += t0;
        arr[i] = t0;
    }
    return sum;
}

int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0xFFF;
        array2[i] = (int)(seed >> 8) & 0xFFF;
    }
    
    /* Threshold based on array content to create branch variance */
    int threshold = 0;
    for (int i = 0; i < 16; i++) {
        threshold += array1[i];
    }
    threshold = (threshold / 16) & 0x1FF;
    
    /* Execute kernel multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 3; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE, threshold);
        
        /* Call another scheduling-intensive function */
        int sum = nested_loop_scheduler(array1, SIZE);
        
        /* Modify threshold to change branch behavior */
        threshold = (threshold + sum) & 0x3FF;
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] ^ array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
