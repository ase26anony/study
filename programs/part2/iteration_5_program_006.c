/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force complex scheduling decisions by creating unpredictable control flow */
#define BARRIER() asm volatile("" ::: "memory")

/* Function attribute to target specific microarchitecture for detailed scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables that must be kept alive */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    
    /* Initialize with volatile reads to prevent optimization */
    volatile int seed = 42;
    v0 = seed;
    v1 = v0 * 3;
    v2 = v1 - 7;
    v3 = v2 ^ 0x55AA55AA;
    v4 = v3 >> 3;
    v5 = v4 | 0x80000000;
    
    f0 = (float)v0 * 0.5f;
    f1 = f0 + 1.234f;
    f2 = f1 * 3.14159f;
    
    /* Main computation loop with data-dependent branching */
    for (int i = 0; i < size; i++) {
        /* Create hard-to-predict branch using array data */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v6 = arr1[i] * v0;
            v7 = arr2[i] + v1;
            v8 = v6 ^ v7;
            v9 = v8 << (arr1[i] & 0x7);
            v10 = v9 - v2;
            v11 = v10 | v3;
            v12 = v11 & v4;
            v13 = v12 * v5;
            
            /* Floating-point ops mixed in */
            f3 = (float)v6 * f0;
            f4 = (float)v7 + f1;
            f5 = f3 / f2;
            f6 = f4 * f5;
            
            /* Memory barrier to force serialization point */
            BARRIER();
            
            /* More computations after barrier */
            v14 = v13 ^ (int)f6;
            v15 = v14 + (int)(f6 * 100.0f);
            
            /* Write back results to create dependencies */
            arr1[i] = v15 ^ 0x12345678;
            arr2[i] = v14 & 0x87654321;
        } else {
            /* Path B: Different computation pattern */
            v6 = arr1[i] + v0;
            v7 = arr2[i] - v1;
            v8 = v6 & v7;
            v9 = v8 >> (arr2[i] & 0x7);
            v10 = v9 ^ v2;
            v11 = v10 + v3;
            v12 = v11 | v4;
            v13 = v12 ^ v5;
            
            /* Different floating-point sequence */
            f3 = (float)v6 / f0;
            f4 = (float)v7 - f1;
            f5 = f3 + f2;
            f6 = f4 * f5;
            f7 = f6 * 2.71828f;
            
            /* Memory barrier at different position */
            v14 = v13 | (int)f7;
            BARRIER();
            
            /* Complex computation after barrier */
            v15 = v14 * (int)(f7 / 3.14159f);
            
            /* Different write pattern */
            arr1[i] = v15 + 0x55AA55AA;
            arr2[i] = v14 | 0xAA55AA55;
        }
        
        /* Common code with more register pressure */
        v0 = v0 ^ v15;
        v1 = v1 + v14;
        v2 = v2 * (arr1[i] & 0xFF);
        v3 = v3 - (arr2[i] & 0xFF);
        
        f0 = f0 + f6;
        f1 = f1 * ((float)v15 * 0.01f);
        
        /* Another barrier to create more scheduling complexity */
        BARRIER();
        
        /* Switch-like control flow using goto to create complex CFG */
        if ((i & 3) == 0) goto merge_point;
        if ((i & 3) == 1) goto alt_path;
        
        continue;
        
    alt_path:
        v4 = v4 ^ v0;
        v5 = v5 | v1;
        continue;
        
    merge_point:
        v6 = v6 & v2;
        v7 = v7 ^ v3;
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                         v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
                         (int)f0 + (int)f1 + (int)f2 + (int)f3 + 
                         (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
}

/* MIPS-specific version with delay slot considerations */
#ifdef __mips__
__attribute__((noinline))
static void mips_complex_kernel(int *arr1, int *arr2, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int init = 1;
    a = init;
    
    for (int idx = 0; idx < size; idx++) {
        /* MIPS delay slot patterns */
        if (arr1[idx] > arr2[idx]) {
            asm volatile("nop" ::: "memory");
            b = arr1[idx] << 2;
            c = arr2[idx] >> 1;
            asm volatile("nop" ::: "memory");
            d = b + c;
            e = d ^ 0xFFFF;
        } else {
            asm volatile("nop" ::: "memory");
            b = arr1[idx] >> 2;
            c = arr2[idx] << 1;
            asm volatile("nop" ::: "memory");
            d = b - c;
            e = d & 0xAAAA;
        }
        
        /* Create many live values */
        f = a + b;
        g = c * d;
        h = e ^ f;
        i = g | h;
        j = i << 3;
        k = j - a;
        l = k & 0xFF;
        m = l + b;
        n = m ^ c;
        o = n | d;
        p = o + e;
        a = p;
        
        arr1[idx] = a;
        arr2[idx] = b;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Run the complex kernel multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 100; iter++) {
        int threshold = (iter & 0xFF) * 3;
        
#ifdef __mips__
        if (iter & 1) {
            mips_complex_kernel(array1, array2, SIZE);
        } else {
            complex_kernel(array1, array2, SIZE, threshold);
        }
#else
        complex_kernel(array1, array2, SIZE, threshold);
#endif
        
        /* Modify threshold to change branch behavior */
        threshold = (threshold + 17) & 0x3FF;
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
