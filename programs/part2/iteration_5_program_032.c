/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fschedule-insns -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force scheduler to work with complex dependencies */
#define BARRIER() asm volatile("" ::: "memory")

/* Function attribute to target specific microarchitecture */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int *volatile_ptr = arr1; /* Prevent optimizations */
    
    /* Initialize with volatile reads to create hard dependencies */
    v0 = *volatile_ptr;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 - v0;
    v4 = v3 ^ v1;
    v5 = v4 | v2;
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with unpredictable pattern */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v6 = arr1[i] + v0;
            v7 = arr2[i] - v1;
            v8 = v6 * v7;
            v9 = v8 >> 3;
            v10 = v9 & 0xFF;
            v11 = v10 | v5;
            v12 = v11 ^ v4;
            v13 = v12 + v3;
            v14 = v13 - v2;
            v15 = v14 * v1;
            
            /* Floating-point ops mixed in */
            f2 = f0 + (float)v6;
            f3 = f1 * (float)v7;
            f4 = f2 - f3;
            f5 = f4 * 2.0f;
            
            /* Memory barrier to force serialization point */
            BARRIER();
            
            /* Complex dependency chain */
            v0 = v15 + (int)f5;
            v1 = v0 ^ arr1[i];
            v2 = v1 * 3;
            f0 = f5 + (float)v2;
            
            /* Another barrier */
            BARRIER();
            
            /* Jump to common code */
            goto common_label;
        } else {
            /* Path B: Different operation mix */
            v6 = arr1[i] * v0;
            v7 = arr2[i] + v1;
            v8 = v6 - v7;
            v9 = v8 << 2;
            v10 = v9 | 0x7F;
            v11 = v10 & v5;
            v12 = v11 ^ v4;
            v13 = v12 * v3;
            v14 = v13 + v2;
            v15 = v14 / (v1 + 1);
            
            /* Different FP sequence */
            f2 = f0 * (float)v6;
            f3 = f1 - (float)v7;
            f4 = f2 + f3;
            f5 = f4 / 2.0f;
            
            BARRIER();
            
            v0 = v15 - (int)f5;
            v1 = v0 & arr2[i];
            v2 = v1 + 5;
            f0 = f5 - (float)v2;
            
            BARRIER();
            
            /* Fall through to common code */
        }
        
    common_label:
        /* Common merging point with more operations */
        switch (i & 3) {
            case 0:
                v3 = v0 + v1;
                f1 = f0 * 1.1f;
                break;
            case 1:
                v3 = v0 - v1;
                f1 = f0 / 1.1f;
                break;
            case 2:
                v3 = v0 * v1;
                f1 = f0 + 2.2f;
                break;
            default:
                v3 = v0 ^ v1;
                f1 = f0 - 2.2f;
                /* Force spill with many live variables */
                v4 = v3 + v2;
                v5 = v4 * v15;
                v6 = v5 & v14;
                v7 = v6 | v13;
                v8 = v7 ^ v12;
                v9 = v8 + v11;
                v10 = v9 - v10;
                f2 = f1 + (float)v4;
                f3 = f2 * (float)v5;
                f4 = f3 - (float)v6;
                f5 = f4 / (float)v7;
                f6 = f5 + (float)v8;
                f7 = f6 * (float)v9;
                break;
        }
        
        /* Store results back to prevent elimination */
        arr1[i] = v3 + (int)f1;
        arr2[i] = v10 + (int)f7;
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + 
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                   (int)f5 + (int)f6 + (int)f7;
    
    /* Volatile store to force computation */
    *volatile_ptr = checksum;
}

/* MIPS-specific version with delay slot considerations */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int *v = arr;
    
    /* MIPS often has delay slots - create complex patterns */
    for (int idx = 0; idx < size; idx++) {
        a = arr[idx];
        b = a + idx;
        
        /* Branch likely pattern for MIPS */
        if (a > 0) {
            c = b * 2;
            d = c - a;
            asm volatile("nop" ::: "memory"); /* Could be in delay slot */
        } else {
            c = b / 2;
            d = c + a;
            asm volatile("nop" ::: "memory");
        }
        
        /* More variables for pressure */
        e = d ^ c;
        f = e | b;
        g = f & a;
        h = g << 2;
        i = h >> 1;
        j = i + idx;
        k = j - g;
        l = k * h;
        m = l / (e + 1);
        n = m ^ f;
        o = n | i;
        p = o & j;
        
        arr[idx] = p;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int arr1[SIZE], arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0x7FFF;
        arr2[i] = (seed >> 8) & 0xFF;
    }
    
    int threshold = 10000;
    
    /* Call the scheduling-intensive kernel */
    complex_scheduling_kernel(arr1, arr2, SIZE, threshold);
    
#ifdef __mips__
    mips_specific_kernel(arr1, SIZE);
#endif
    
    /* Compute final checksum */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", sum);
    return 0;
}
