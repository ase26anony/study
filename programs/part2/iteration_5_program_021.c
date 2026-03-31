/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS: gcc -O3 -mips64 -march=mips64r2 -mtune=mips64r2 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS has complex scheduling models with delay slots */
#else
/* Generic fallback */
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* High register pressure: many distinct variables */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    volatile int v1, v2; /* Prevent optimization */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    a = v1;
    b = v2;
    
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch with unpredictable pattern */
        if (__builtin_expect((arr1[idx] ^ arr2[idx]) > threshold, 0)) {
            /* Path 1: Integer-heavy computation */
            c = a + arr1[idx];
            d = b - arr2[idx];
            e = c * d;
            f = e ^ arr1[idx];
            g = f | arr2[idx];
            h = g & 0x7FFFFFFF;
            i = h << 3;
            j = i >> 1;
            k = j + arr1[idx % 16];
            l = k - arr2[idx % 16];
            m = l * 3;
            n = m / 2;
            o = n ^ m;
            p = o | n;
            
            /* Floating-point ops mixed in */
            fa = (float)a * 1.5f;
            fb = (float)b / 2.0f;
            fc = fa + fb;
            fd = fc - fa;
            fe = fd * 1.25f;
            ff = fe / 1.1f;
            fg = ff + fc;
            fh = fg - fd;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Update shared variables for next iteration */
            a = p + (int)fh;
            b = o - (int)fe;
        } else {
            /* Path 2: Different operation mix */
            c = a - arr1[idx];
            d = b + arr2[idx];
            e = d * c;
            f = e & arr1[idx];
            g = f ^ arr2[idx];
            h = g | 0x0FFFFFFF;
            i = h >> 2;
            j = i << 1;
            k = j - arr1[idx % 16];
            l = k + arr2[idx % 16];
            m = l * 5;
            n = m / 3;
            o = n & m;
            p = o ^ n;
            
            /* Different floating-point sequence */
            fa = (float)b * 2.5f;
            fb = (float)a / 1.5f;
            fc = fb - fa;
            fd = fc + fb;
            fe = fd * 0.75f;
            ff = fe / 1.3f;
            fg = ff - fc;
            fh = fg + fd;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Update with different pattern */
            a = p - (int)fh;
            b = o + (int)fe;
        }
        
        /* Complex control flow with goto to create CFG complexity */
        if (idx % 7 == 0) {
            goto merge_point;
        }
        
        /* Additional computation to increase basic block size */
        int t1 = a * b;
        int t2 = c + d;
        float ft1 = (float)t1 * 0.1f;
        float ft2 = (float)t2 * 0.2f;
        
        merge_point:
        /* Use all variables to prevent dead code elimination */
        arr1[idx % 16] = (a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p +
                         (int)fa + (int)fb + (int)fc + (int)fd + (int)fe + (int)ff + 
                         (int)fg + (int)fh) ^ (t1 + t2 + (int)ft1 + (int)ft2);
    }
    
    /* Final computation using all variables */
    int checksum = a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k ^ l ^ m ^ n ^ o ^ p;
    float fchecksum = fa + fb + fc + fd + fe + ff + fg + fh;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(checksum), "+r"(fchecksum) : : "memory");
    
    /* Store result to volatile location */
    volatile int result = checksum + (int)fchecksum;
    (void)result;
}

/* Function with switch statement for additional CFG complexity */
static void switch_based_computation(int *arr, int size) {
    int x = 0, y = 0, z = 0;
    float fx = 0.0f, fy = 0.0f, fz = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Switch creates multiple basic blocks */
        switch (arr[i] % 5) {
            case 0:
                x = arr[i] * 2;
                y = x + i;
                fx = (float)x * 1.1f;
                fy = fx / 2.0f;
                asm volatile("" ::: "memory");
                break;
            case 1:
                x = arr[i] / 2;
                y = x - i;
                fx = (float)y * 0.9f;
                fy = fx * 1.5f;
                asm volatile("" ::: "memory");
                break;
            case 2:
                x = arr[i] ^ 0xAA;
                y = x | i;
                fx = (float)x + 1.0f;
                fy = fx - 0.5f;
                asm volatile("" ::: "memory");
                break;
            case 3:
                x = arr[i] & 0x55;
                y = x ^ i;
                fx = (float)y / 3.0f;
                fy = fx * 4.0f;
                asm volatile("" ::: "memory");
                break;
            default:
                x = arr[i] + i;
                y = x * 3;
                fx = (float)x * 0.75f;
                fy = fx + 1.0f;
                asm volatile("" ::: "memory");
                break;
        }
        
        z = x + y + (int)fx + (int)fy;
        fz = fx + fy + (float)z;
        
        /* Store back to create dependencies */
        arr[i % 32] = z + (int)fz;
    }
}

int main() {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed % 1000);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed % 1000);
    }
    
    /* Run multiple times to increase scheduling pressure */
    for (int iter = 0; iter < 100; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE, 500);
        switch_based_computation(array1, SIZE);
        
        /* Modify threshold to change branch behavior */
        int threshold = (iter % 2) ? 400 : 600;
        complex_scheduling_kernel(array2, array1, SIZE, threshold);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum ^= array1[i];
        final_checksum += array2[i];
    }
    
    printf("Result: %d\n", final_checksum);
    return 0;
}
