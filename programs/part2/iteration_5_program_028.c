/* Compile with: gcc -O3 -fschedule-insns -fno-omit-frame-pointer -fdump-rtl-sched -fdump-rtl-sched2 -march=nehalem -mtune=nehalem -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force complex scheduling decisions with architecture-specific tuning */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables that must be live simultaneously */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier;  /* Prevent optimizations across barriers */
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = arr1[0] ^ 0x12345678;
    v1 = arr2[0] | 0x87654321;
    v2 = v0 * v1;
    v3 = v1 - v0;
    v4 = v2 ^ v3;
    v5 = v3 + v2;
    f0 = (float)v0 * 1.41421356f;
    f1 = (float)v1 * 2.71828182f;
    
    for (int i = 1; i < size; i++) {
        /* Data-dependent branch creates unpredictable control flow */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v6 = arr1[i] * v0;
            v7 = arr2[i] + v1;
            v8 = v6 ^ v7;
            v9 = v7 - v6;
            v10 = v8 * v9;
            v11 = v9 / (v8 ? v8 : 1);
            v12 = v10 ^ v11;
            v13 = v11 + v10;
            
            /* Floating-point ops mixed with integer ops */
            f2 = f0 * (float)v6;
            f3 = f1 + (float)v7;
            f4 = f2 - f3;
            f5 = f3 * f2;
            
            /* Memory barrier forces serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v6;
            
            /* More operations after barrier */
            v14 = v12 ^ v13;
            v15 = v13 - v12;
            f6 = f4 * f5;
            f7 = f5 - f4;
            
            /* Cross-dependencies between variables */
            v0 = v14 ^ v15;
            v1 = v15 + v14;
            f0 = f6 + f7;
            f1 = f7 * f6;
            
        } else {
            /* Path B: Different operation mix to create alternative schedule */
            v6 = arr1[i] + v0;
            v7 = arr2[i] - v1;
            v8 = v6 | v7;
            v9 = v7 & v6;
            v10 = v8 ^ v9;
            v11 = v9 << (v8 & 3);
            v12 = v10 >> (v9 & 3);
            v13 = v11 | v12;
            
            /* Different FP operation pattern */
            f2 = f0 + (float)v6;
            f3 = f1 - (float)v7;
            f4 = f2 / (f3 != 0.0f ? f3 : 1.0f);
            f5 = f3 * f2;
            
            /* Different barrier placement */
            v14 = v12 ^ v13;
            asm volatile("" ::: "memory");
            mem_barrier = v7;
            
            /* Continue with different dependency chain */
            v15 = v13 - v12;
            f6 = f4 + f5;
            f7 = f5 - f4;
            
            /* Different update pattern */
            v0 = v14 & v15;
            v1 = v15 | v14;
            f0 = f6 * f7;
            f1 = f7 / (f6 != 0.0f ? f6 : 1.0f);
        }
        
        /* Common merge point with complex data flow */
        switch (i & 3) {
            case 0:
                v2 = v0 * v1;
                v3 = v1 - v0;
                goto common_label;
            case 1:
                v2 = v0 ^ v1;
                v3 = v1 + v0;
                goto common_label;
            case 2:
                v2 = v0 | v1;
                v3 = v1 & v0;
                goto common_label;
            default:
                v2 = v0 << 2;
                v3 = v1 >> 1;
                /* fall through */
        }
        
    common_label:
        /* Shared computations that depend on both paths */
        v4 = v2 ^ v3;
        v5 = v3 + v2;
        f2 = f0 * 1.5f;
        f3 = f1 / 2.0f;
        
        /* Force register spilling with many live values */
        arr1[i-1] = v0 + v1 + v2 + v3 + v4 + v5 + (int)f0 + (int)f1;
        arr2[i-1] = v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ (int)f2 ^ (int)f3;
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + 
                           v10 + v11 + v12 + v13 + v14 + v15 +
                           (int)f0 + (int)f1 + (int)f2 + (int)f3 +
                           (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Use checksum to prevent optimization */
    if (checksum == 0xdeadbeef) {
        printf("Impossible condition\n");
    }
}

/* Alternative implementation for MIPS if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int barrier;
    
    for (int idx = 0; idx < size; idx++) {
        /* MIPS delay slot patterns can trigger special scheduling */
        a = arr1[idx];
        b = arr2[idx];
        asm volatile("" ::: "memory");
        
        c = a + b;
        d = a - b;
        e = c * d;
        f = c ^ d;
        g = e << 2;
        h = f >> 1;
        
        /* Many interdependent operations */
        i = g + h;
        j = g - h;
        k = i * j;
        l = i ^ j;
        m = k + l;
        n = k - l;
        o = m * n;
        p = m ^ n;
        
        /* Force state saving with complex dependency web */
        arr1[idx] = a + c + e + g + i + k + m + o;
        arr2[idx] = b + d + f + h + j + l + n + p;
        
        asm volatile("" ::: "memory");
        barrier = idx;
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 0x12345678;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFFFFFF);
        array2[i] = (int)((seed >> 16) & 0x7FFFFFFF);
    }
    
    /* Run the scheduling stress test */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE, 0x40000000);
#endif
    
    /* Compute and print checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
