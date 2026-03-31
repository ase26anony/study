/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex microarchitecture constraints */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many live variables across basic blocks */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    volatile int v1, v2; /* Prevent optimization of dependencies */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    a = v1;
    b = v2;
    c = a + b;
    d = a - b;
    
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch with unpredictable pattern */
        if (__builtin_expect((arr1[idx] ^ arr2[idx]) > threshold, 0)) {
            /* Path 1: Integer-heavy computation with many intermediates */
            e = arr1[idx] * 3;
            f = arr2[idx] / 7;
            g = e ^ f;
            h = g << 2;
            i = h | 0x7F;
            j = i & 0xFF;
            k = j + arr1[idx];
            l = k - arr2[idx];
            m = l * 2;
            n = m >> 1;
            o = n ^ arr1[idx];
            p = o + arr2[idx];
            
            /* Floating-point ops mixed in */
            fa = (float)arr1[idx] * 1.5f;
            fb = (float)arr2[idx] * 0.75f;
            fc = fa + fb;
            fd = fa - fb;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* Complex dependency chain */
            fe = fc * fd;
            ff = fe / 3.14159f;
            fg = ff + fa;
            fh = fg - fb;
            
            /* Update array with result */
            arr1[idx] = p + (int)fh;
        } else {
            /* Path 2: Different computation pattern to create scheduling conflicts */
            e = arr1[idx] + 17;
            f = arr2[idx] - 29;
            g = e & f;
            h = g | 0xAA;
            i = h ^ 0x55;
            j = i * 3;
            k = j / 5;
            l = k + 11;
            m = l - 13;
            n = m << 1;
            o = n >> 2;
            p = o ^ 0xFF;
            
            /* Different floating-point pattern */
            fa = (float)arr1[idx] / 2.0f;
            fb = (float)arr2[idx] * 3.0f;
            fc = fb - fa;
            fd = fc * 2.0f;
            
            /* Another memory barrier at different position */
            asm volatile("" ::: "memory");
            
            fe = fd + 1.0f;
            ff = fe * 0.5f;
            fg = ff - fa;
            fh = fg + fb;
            
            /* Update array */
            arr2[idx] = p - (int)fh;
        }
        
        /* Converge point with all variables live */
        a = (a + b) ^ c;
        b = (b - d) | e;
        c = c * f;
        d = d / g;
        
        /* Switch statement to create complex CFG */
        switch (arr1[idx] & 0x3) {
            case 0:
                e = h + i;
                f = j - k;
                goto common_label;
            case 1:
                e = l * m;
                f = n / o;
                goto common_label;
            case 2:
                e = p ^ a;
                f = b & c;
                /* fall through */
            default:
                e = d | e;
                f = f ^ 0x7F;
                common_label:
                g = e + f;
                h = g - a;
                break;
        }
        
        /* More operations keeping variables live */
        i = h * 2;
        j = i / 3;
        k = j | 0xAA;
        l = k & 0x55;
        
        /* Force potential spilling */
        m = l + arr1[idx];
        n = m - arr2[idx];
        o = n * arr1[idx];
        p = o / (arr2[idx] + 1);
        
        /* Update arrays to create loop-carried dependencies */
        arr1[idx] = (arr1[idx] + p) & 0xFFF;
        arr2[idx] = (arr2[idx] ^ p) & 0xFFF;
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    volatile float fresult = fa + fb + fc + fd + fe + ff + fg + fh;
    (void)result;
    (void)fresult;
}

/* MIPS-specific version with delay slot considerations */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    volatile int v;
    
    for (int i = 0; i < size; i++) {
        /* Create branch patterns that might use delay slots */
        if (arr1[i] > arr2[i]) {
            t0 = arr1[i] << 2;
            t1 = arr2[i] >> 1;
            t2 = t0 + t1;
            /* asm to prevent reordering */
            asm volatile("" ::: "memory");
            t3 = t2 * 3;
            t4 = t3 / 2;
        } else {
            t0 = arr1[i] >> 2;
            t1 = arr2[i] << 1;
            t2 = t0 - t1;
            asm volatile("" ::: "memory");
            t3 = t2 * 5;
            t4 = t3 / 3;
        }
        
        /* Complex dependency web */
        t5 = t4 ^ t0;
        t6 = t5 & t1;
        t7 = t6 | t2;
        t8 = t7 + t3;
        t9 = t8 - t4;
        
        arr1[i] = t9;
        arr2[i] = t5;
        
        /* Force register pressure */
        v = t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        arr1[i] ^= v;
    }
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
        arr1[i] = (seed >> 16) & 0xFFF;
        seed = seed * 1103515245 + 12345;
        arr2[i] = (seed >> 16) & 0xFFF;
    }
    
    /* Run the complex kernel multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 100; iter++) {
        complex_kernel(arr1, arr2, SIZE, 1500);
        
#ifdef __mips__
        mips_specific_kernel(arr1, arr2, SIZE);
#endif
        
        /* Alternate threshold to vary branch behavior */
        int threshold = (iter % 2) ? 1000 : 2000;
        complex_kernel(arr2, arr1, SIZE, threshold);
    }
    
    /* Compute checksum to prevent elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i];
        checksum += arr2[i] * 3;
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
