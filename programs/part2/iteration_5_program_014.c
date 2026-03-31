/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many distinct variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with non-trivial values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f; f6 = 6.6f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v1 = v2 + v3;
            v2 = v4 * v5;
            v3 = v6 ^ v7;
            v4 = v8 | v9;
            v5 = v10 & v11;
            v6 = v12 - v13;
            v7 = v14 << 2;
            v8 = v15 >> 1;
            
            /* Mix integer and float operations */
            f1 = f2 + f3;
            f2 = f4 * f5;
            f3 = (float)v1 + f6;
            
            /* Memory operations to create additional pressure */
            mem_barrier = arr1[i];
            v9 = mem_barrier * v2;
            v10 = arr2[i] + v3;
            
            /* Complex dependency chain */
            v11 = (v4 * v5) + (v6 ^ v7);
            v12 = (v8 << 1) | (v9 & 0xFF);
            v13 = v10 - v11;
            v14 = v12 * v13;
            v15 = v14 ^ v15;
            
            /* Artificial serialization point */
            asm volatile("" ::: "memory");
            
            /* Branch to common label creating CFG complexity */
            goto merge_point;
        } else {
            /* Path B: Different operation mix with same variables */
            v1 = v3 - v2;
            v2 = v5 / (v4 ? v4 : 1);
            v3 = v7 & v6;
            v4 = v9 ^ v8;
            v5 = v11 | v10;
            v6 = v13 + v12;
            v7 = v15 << 1;
            v8 = v14 >> 2;
            
            /* Different float operation sequence */
            f4 = f5 - f6;
            f5 = f1 * f2;
            f6 = (float)v2 - f3;
            
            /* More memory operations with volatile read */
            volatile int *volatile_ptr = &arr1[i];
            v9 = *volatile_ptr + v4;
            v10 = arr2[i] * v5;
            
            /* Alternative dependency chain */
            v11 = (v6 ^ v7) | (v8 & v9);
            v12 = (v10 << 2) + (v11 >> 1);
            v13 = v12 * v13;
            v14 = v13 ^ v14;
            v15 = v15 + v14 * 3;
            
            /* Another serialization point */
            asm volatile("" ::: "memory");
            
            /* Fall through to merge point */
        }
        
merge_point:
        /* Common computation using all variables - creates convergence point */
        v1 = v1 + v2 + v3;
        v4 = v4 ^ v5 ^ v6;
        v7 = v7 * v8 * (v9 ? v9 : 1);
        v10 = (v10 << 1) | (v11 >> 1);
        v12 = v12 + v13 - v14;
        v15 = v15 ^ (v1 & v4);
        
        f1 = f1 + f2;
        f3 = f3 * f4;
        f5 = f5 - f6;
        f6 = (f1 + f3) * f5;
        
        /* Switch statement for additional control flow complexity */
        switch (i & 0x3) {
            case 0:
                v1 = v2 + v3;
                break;
            case 1:
                v4 = v5 * v6;
                /* Intentional fall-through */
            case 2:
                v7 = v8 ^ v9;
                v10 = v11 | v12;
                break;
            case 3:
                v13 = v14 - v15;
                /* goto creates loop within basic block */
                if (v13 > 1000) goto reset_vars;
                break;
        }
        
        continue;
        
reset_vars:
        /* Reset some variables occasionally */
        v1 = 1; v2 = 2; v3 = 3;
        f1 = 1.1f; f2 = 2.2f;
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                         v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                         (int)f3 + (int)f4 + (int)f5 + (int)f6;
    (void)result;
}

/* MIPS-specific version with delay slot considerations */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr, int size) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int barrier;
    
    for (int i = 0; i < size; i++) {
        /* Create branch-likely patterns for MIPS delay slots */
        if (__builtin_expect(arr[i] & 1, 1)) {
            a = b + c;
            asm volatile("" ::: "memory");
            b = d * e;
            c = f ^ g;
            d = h << a;
        } else {
            a = c - b;
            asm volatile("" ::: "memory");
            b = e / (d ? d : 1);
            c = g & f;
            d = a >> h;
        }
        
        /* Force multiple instruction types in delay slots */
        e = a + b;
        f = c * d;
        g = e ^ f;
        h = g + arr[i];
        
        barrier = arr[size - i - 1];
        a = barrier + h;
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    const int THRESHOLD = 1000000;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        array2[i] = (seed >> 8) & 0x7FFF;
    }
    
    /* Execute the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE, THRESHOLD);
    
#ifdef __mips__
    mips_specific_kernel(array1, SIZE);
#endif
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i] + array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
