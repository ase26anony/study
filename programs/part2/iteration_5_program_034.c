/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    volatile int v1, v2; /* Prevent optimization */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    a = v1;
    b = v2;
    c = a + b;
    d = a - b;
    
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch with unpredictable pattern */
        if ((arr1[idx] ^ arr2[idx]) & 0x1) {
            /* Path 1: Integer-heavy computation */
            e = arr1[idx] * arr2[idx];
            f = arr1[idx] ^ arr2[idx];
            g = arr1[idx] | arr2[idx];
            h = arr1[idx] & arr2[idx];
            
            /* Create artificial serialization */
            asm volatile("" ::: "memory");
            
            /* More operations with dependencies */
            i = e + f;
            j = g - h;
            k = i * j;
            l = k ^ arr1[idx];
            m = l + arr2[idx];
            
            /* Floating point ops mixed in */
            fa = (float)arr1[idx] * 1.5f;
            fb = (float)arr2[idx] * 2.5f;
            fc = fa + fb;
            fd = fa - fb;
            
            /* Complex branching within the path */
            if (__builtin_expect((arr1[idx] > 1000), 0)) {
                n = m * 2;
                o = n + arr1[idx];
                fe = fc * 2.0f;
                ff = fd / 2.0f;
            } else {
                n = m / 2;
                o = n - arr1[idx];
                fe = fc / 2.0f;
                ff = fd * 2.0f;
            }
            
            /* More operations to increase pressure */
            p = o ^ n;
            fg = fe + ff;
            fh = fe - ff;
            
            /* Memory barrier to force scheduler state save */
            asm volatile("" ::: "memory");
            
            /* Merge point operations */
            a = p + l;
            b = o - m;
            fi = fg * fh;
            fj = fg / fh;
            
        } else {
            /* Path 2: Different computation pattern */
            e = arr1[idx] + arr2[idx];
            f = arr1[idx] - arr2[idx];
            g = arr1[idx] << 2;
            h = arr2[idx] >> 1;
            
            /* Different serialization pattern */
            asm volatile("" ::: "memory");
            
            i = e * f;
            j = g ^ h;
            k = i & j;
            l = k | arr1[idx];
            m = l ^ arr2[idx];
            
            /* Different floating point pattern */
            fa = (float)arr1[idx] / 1.3f;
            fb = (float)arr2[idx] / 2.7f;
            fc = fa * fb;
            fd = fa / fb;
            
            /* Another unpredictable branch */
            if (__builtin_expect((arr2[idx] < 500), 0)) {
                n = m << 1;
                o = n ^ arr2[idx];
                fe = fc + 1.0f;
                ff = fd - 1.0f;
            } else {
                n = m >> 1;
                o = n & arr2[idx];
                fe = fc - 1.0f;
                ff = fd + 1.0f;
            }
            
            p = o | n;
            fg = fe * ff;
            fh = fe / ff;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            /* Different merge operations */
            a = p - l;
            b = o + m;
            fi = fg + fh;
            fj = fg - fh;
        }
        
        /* Common post-processing with all variables */
        c = a + b + c;
        d = a - b + d;
        e = e * 3 + arr1[idx];
        f = f / 2 + arr2[idx];
        
        /* Use goto to create complex control flow */
        if (idx % 7 == 0) {
            goto special_case;
        }
        
        /* Normal continuation */
        g = g ^ arr1[idx];
        h = h | arr2[idx];
        continue;
        
    special_case:
        /* Alternative path that rejoins */
        g = g & arr1[idx];
        h = h ^ arr2[idx];
        
        /* Switch statement for additional complexity */
        switch (arr1[idx] % 4) {
            case 0:
                i = i + 1;
                fa = fa * 1.1f;
                break;
            case 1:
                i = i - 1;
                fa = fa / 1.1f;
                break;
            case 2:
                i = i * 2;
                fa = fa + 0.5f;
                break;
            case 3:
                i = i / 2;
                fa = fa - 0.5f;
                break;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    float fchecksum = fa + fb + fc + fd + fe + ff + fg + fh + fi + fj;
    
    /* Volatile store to ensure computation isn't optimized away */
    volatile int result __attribute__((unused)) = checksum + (int)fchecksum;
}

int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (seed >> 16) & 0x7FFF;
    }
    
    /* Call the scheduling-intensive kernel multiple times */
    for (int iter = 0; iter < 1000; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            array1[i] = (array1[i] * 13 + 7) & 0xFFFF;
            array2[i] = (array2[i] * 17 + 11) & 0xFFFF;
        }
    }
    
    free(array1);
    free(array2);
    
    printf("Scheduler test completed.\n");
    return 0;
}
