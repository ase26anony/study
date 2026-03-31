/* haifa-sched-trigger.c
 * Program designed to trigger GCC's Haifa scheduler state save/restore mechanism
 * to exercise the uncovered free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of specific x86 microarchitecture with detailed scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void compute_kernel(int* restrict arr1, int* restrict arr2, int size, int threshold) {
    /* High register pressure: many local variables in a small scope */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Force memory dependencies */
    
    /* Initialize with non-trivial values to prevent optimization */
    v0 = arr1[0] ^ arr2[0];
    v1 = arr1[1] | arr2[1];
    v2 = arr1[2] & arr2[2];
    v3 = arr1[3] + arr2[3];
    v4 = arr1[4] - arr2[4];
    v5 = arr1[5] * arr2[5];
    v6 = arr1[6] ^ ~arr2[6];
    v7 = arr1[7] | (arr2[7] << 2);
    v8 = arr1[8] & (arr2[8] >> 1);
    v9 = arr1[9] + (arr2[9] * 3);
    v10 = arr1[10] - (arr2[10] / 2);
    v11 = arr1[11] * (arr2[11] | 1);
    v12 = arr1[12];
    v13 = arr2[13];
    v14 = v0 ^ v1;
    v15 = v2 | v3;
    
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = (float)v3 * 3.5f;
    f4 = (float)v4 * 4.5f;
    f5 = (float)v5 * 5.5f;
    f6 = (float)v6 * 6.5f;
    f7 = (float)v7 * 7.5f;
    
    /* Complex loop with data-dependent branches and mixed operations */
    for (int i = 16; i < size - 8; i++) {
        /* Data-dependent branch - hard to predict */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v0 = v0 + (arr1[i] & 0xFF);
            v1 = v1 - (arr2[i] | 0x7F);
            v2 = v2 ^ (arr1[i+1] * 2);
            v3 = v3 | (arr2[i+1] / 3);
            v4 = v4 & (v0 ^ v1);
            v5 = v5 + (v2 | v3);
            v6 = v6 - (v4 & v5);
            v7 = v7 * (v6 ^ 0xABCD);
            v8 = v8 | (v7 & 0x1234);
            v9 = v9 ^ (v8 >> 3);
            v10 = v10 + (v9 << 2);
            v11 = v11 - (v10 * 3);
            v12 = v12 & (v11 | 0xF0F0);
            v13 = v13 ^ (v12 + 1);
            v14 = v14 | (v13 - 2);
            v15 = v15 & (v14 * 5);
            
            /* Floating-point operations mixed in */
            f0 = f0 + (float)v0 * 0.125f;
            f1 = f1 - (float)v1 * 0.25f;
            f2 = f2 * (float)v2 * 0.5f;
            f3 = f3 / ((float)v3 + 1.0f);
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v0;
            
            /* Jump to common code - creates complex CFG */
            goto common_merge;
        } else {
            /* Path B: Different operation mix */
            v0 = v0 - (arr1[i] | 0x3F);
            v1 = v1 + (arr2[i] & 0xBF);
            v2 = v2 | (arr1[i+1] / 5);
            v3 = v3 ^ (arr2[i+1] * 7);
            v4 = v4 + (v0 | v1);
            v5 = v5 - (v2 ^ v3);
            v6 = v6 & (v4 + v5);
            v7 = v7 | (v6 - 0x1234);
            v8 = v8 ^ (v7 & 0xABCD);
            v9 = v9 + (v8 << 1);
            v10 = v10 - (v9 >> 2);
            v11 = v11 & (v10 * 7);
            v12 = v12 ^ (v11 | 0x0F0F);
            v13 = v13 + (v12 - 3);
            v14 = v14 - (v13 * 2);
            v15 = v15 | (v14 + 4);
            
            /* Different FP operations */
            f4 = f4 + (float)v4 * 1.125f;
            f5 = f5 - (float)v5 * 2.25f;
            f6 = f6 * (float)v6 * 3.5f;
            f7 = f7 / ((float)v7 + 2.0f);
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            mem_barrier = v1;
            
            /* Fall through to common code */
        }
        
    common_merge:
        /* Common code with more mixed operations */
        f0 = f0 + f4 * 0.33f;
        f1 = f1 - f5 * 0.66f;
        f2 = f2 * f6 * 1.5f;
        f3 = f3 / (f7 + 0.1f);
        
        v0 = v0 ^ v8;
        v1 = v1 | v9;
        v2 = v2 & v10;
        v3 = v3 + v11;
        v4 = v4 - v12;
        v5 = v5 * v13;
        v6 = v6 ^ v14;
        v7 = v7 | v15;
        
        /* Switch statement to create additional control flow complexity */
        switch (arr1[i] & 0x7) {
            case 0:
                v8 = v8 + (v0 >> 1);
                f0 = f0 * 2.0f;
                break;
            case 1:
                v9 = v9 - (v1 << 1);
                f1 = f1 / 2.0f;
                break;
            case 2:
                v10 = v10 ^ (v2 & 0xAA);
                f2 = f2 + 1.0f;
                break;
            case 3:
                v11 = v11 | (v3 | 0x55);
                f3 = f3 - 1.0f;
                break;
            case 4:
                v12 = v12 & (v4 ^ 0xF0);
                f4 = f4 * 3.0f;
                break;
            case 5:
                v13 = v13 + (v5 * 3);
                f5 = f5 / 3.0f;
                break;
            case 6:
                v14 = v14 - (v6 / 2);
                f6 = f6 + 2.0f;
                break;
            default:
                v15 = v15 * (v7 | 1);
                f7 = f7 - 2.0f;
                break;
        }
        
        /* Final memory barrier in loop */
        asm volatile("" ::: "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    arr1[0] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
              v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
              (int)f0 + (int)f1 + (int)f2 + (int)f3 +
              (int)f4 + (int)f5 + (int)f6 + (int)f7;
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_compute_kernel(int* arr1, int* arr2, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int barrier;
    
    a = arr1[0]; b = arr2[0]; c = a + b; d = a - b;
    e = a * b; f = a ^ b; g = a | b; h = a & b;
    
    for (int idx = 1; idx < size; idx++) {
        /* Create delay slot pressure */
        if (arr1[idx] > arr2[idx]) {
            i = c + d; j = e - f; k = g ^ h;
            asm volatile("nop" ::: "memory");
            barrier = i;
            goto mips_merge;
        } else {
            l = d - c; m = f * e; n = h | g;
            asm volatile("nop" ::: "memory");
            barrier = l;
        }
        
    mips_merge:
        o = i + j + k + l + m + n;
        p = o * 2;
        
        /* Force dependency chain */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d),
                         "+r"(e), "+r"(f), "+r"(g), "+r"(h) : : "memory");
    }
    
    arr1[0] = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}
#endif

int main(void) {
    const int SIZE = 512;
    const int THRESHOLD = 1000;
    int* array1 = malloc(SIZE * sizeof(int));
    int* array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values using LCG */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Call computation kernel multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 100; iter++) {
#ifdef __mips__
        mips_compute_kernel(array1, array2, SIZE);
#else
        compute_kernel(array1, array2, SIZE, THRESHOLD + iter);
#endif
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    return 0;
}
