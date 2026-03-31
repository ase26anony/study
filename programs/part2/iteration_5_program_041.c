/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS: add -mips32r2 -msched-weight -mno-split-addresses */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to work with complex dependencies */
#define BARRIER() asm volatile("" ::: "memory")

/* Function attribute to target specific microarchitecture */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_kernel(int* restrict arr1, int* restrict arr2, int size, int threshold) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int* volatile_ptr = arr1; /* Prevent optimization */
    
    /* Initialize variables to create dependencies */
    v0 = *volatile_ptr;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 - v0;
    v4 = v3 ^ v1;
    v5 = v4 | v2;
    f0 = (float)v0 * 0.5f;
    f1 = f0 + 1.0f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int cond = arr1[i] & arr2[i];
        if (__builtin_expect((cond > threshold) && (i % 7 != 0), 0)) {
            /* Path A: Integer-heavy computation */
            v6 = arr1[i] + v0;
            v7 = arr2[i] - v1;
            v8 = v6 * v7;
            v9 = v8 >> 3;
            v10 = v9 & 0xFF;
            v11 = v10 | v2;
            v12 = v11 ^ v3;
            v13 = v12 + v4;
            v14 = v13 - v5;
            v15 = v14 * v6;
            
            f2 = (float)v6 * f0;
            f3 = f2 + f1;
            f4 = f3 * 2.0f;
            f5 = f4 - f0;
            
            /* Force serialization point */
            BARRIER();
            
            /* Cross-path dependencies */
            v0 = v15 + arr1[(i + 1) % size];
            v1 = v0 ^ v12;
            f0 = f5 * 0.75f;
        } else {
            /* Path B: Mixed integer/float with memory ops */
            v6 = arr2[i] * v0;
            v7 = arr1[i] / (v1 + 1);
            v8 = v6 & v7;
            v9 = v8 | v2;
            v10 = v9 ^ v3;
            v11 = v10 + v4;
            v12 = v11 - v5;
            
            f2 = (float)arr1[i] * 1.5f;
            f3 = (float)arr2[i] * 0.25f;
            f4 = f2 + f3;
            f5 = f4 * f0;
            f6 = f5 - f1;
            f7 = f6 / 2.0f;
            
            /* Different serialization point */
            BARRIER();
            
            /* Complex dependency chain */
            v13 = (int)f2 + v6;
            v14 = v13 * v7;
            v15 = v14 ^ v8;
            
            v0 = v15 - arr2[(i + 2) % size];
            v1 = v0 & v9;
            f0 = f7 + 1.0f;
        }
        
        /* Converge point with more operations */
        v2 = v0 + v1;
        v3 = v2 * v6;
        v4 = v3 ^ v7;
        v5 = v4 | v8;
        
        f1 = f0 * 0.9f;
        
        /* Occasionally force another barrier */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            BARRIER();
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    
    /* Volatile store to force computation */
    *volatile_ptr = result;
}

/* Alternative implementation for MIPS with delay slots */
#ifdef __mips__
__attribute__((optimize("O2")))
static void mips_complex_kernel(int* arr1, int* arr2, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int v = 0;
    
    for (int idx = 0; idx < size; idx++) {
        /* Create branch pattern that uses delay slots */
        if (arr1[idx] > arr2[idx]) {
            a = arr1[idx] + 1;
            b = a << 2;
            c = b - arr2[idx];
            asm volatile("nop" ::: "memory");
        } else {
            a = arr2[idx] * 3;
            b = a >> 1;
            c = b ^ arr1[idx];
            asm volatile("nop" ::: "memory");
        }
        
        /* More operations to fill delay slots */
        d = c + idx;
        e = d & 0xFF;
        f = e | a;
        g = f ^ b;
        
        /* Force state save with many live values */
        h = g + c;
        i = h - d;
        j = i * e;
        k = j >> 3;
        l = k & f;
        m = l | g;
        n = m ^ h;
        o = n + i;
        p = o - j;
        
        v = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    }
    
    arr1[0] = v;
}
#endif

int main() {
    const int SIZE = 256;
    int arr1[SIZE];
    int arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed >> 16) & 0xFFF;
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed >> 16) & 0xFFF;
    }
    
    /* Run the complex kernel multiple times */
    for (int iter = 0; iter < 100; iter++) {
        int threshold = iter & 0x7F;
        
#ifdef __mips__
        mips_complex_kernel(arr1, arr2, SIZE);
#else
        complex_kernel(arr1, arr2, SIZE, threshold);
#endif
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] + arr2[i]) & 0xFFF;
            arr2[i] = (arr2[i] + iter) & 0xFFF;
        }
    }
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
