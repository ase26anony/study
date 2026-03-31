/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore
 * with subsequent cleanup of saved context.
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o trigger haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of specific scheduling model on x86 */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    volatile int v1, v2; /* Prevent optimization */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    a = v1; b = v2; c = a + b; d = a - b;
    
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch with hard-to-predict condition */
        if (__builtin_expect((arr1[idx] ^ arr2[idx]) & 1, 0)) {
            /* Path 1: Integer-heavy operations */
            e = arr1[idx] * arr2[idx];
            f = e >> 3;
            g = f ^ arr1[idx];
            h = g + arr2[idx];
            i = h * 7;
            j = i - arr1[idx];
            k = j & 0xFF;
            l = k | arr2[idx];
            m = l << 2;
            n = m / 3;
            o = n ^ m;
            p = o + idx;
            
            /* Floating-point operations mixed in */
            fa = (float)arr1[idx] * 1.5f;
            fb = fa + (float)arr2[idx];
            fc = fb * 0.75f;
            fd = fc - fa;
            
            /* Memory barrier creates serialization point */
            asm volatile("" ::: "memory");
            
            /* Use results to prevent elimination */
            a += p; b += (int)fd;
        } else {
            /* Path 2: Different operation mix */
            e = arr1[idx] + arr2[idx] * 3;
            f = e & arr1[idx];
            g = f | arr2[idx];
            h = g ^ idx;
            i = h * 13;
            j = i >> 1;
            k = j - arr2[idx];
            l = k * 2;
            m = l + arr1[idx];
            n = m & 0x7F;
            o = n ^ 0x55;
            p = o * idx;
            
            /* Different floating-point pattern */
            fe = (float)arr2[idx] * 2.0f;
            ff = fe / 1.3f;
            fg = ff + (float)arr1[idx];
            fh = fg - fe;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Use results */
            c += p; d += (int)fh;
        }
        
        /* Complex switch with goto to create CFG complexity */
        switch (arr1[idx] & 0x3) {
            case 0:
                a = b + c;
                goto common_label;
            case 1:
                d = e + f;
                goto common_label;
            case 2:
                g = h + i;
                /* fall through */
            default:
                j = k + l;
                common_label:
                m = n + o;
                /* Force register pressure */
                fa = fb + fc;
                break;
        }
        
        /* Volatile function pointer to inhibit optimizations */
        int (*volatile dummy)(void) = (int (*)(void))&complex_scheduling_kernel;
        if (idx % 16 == 0) {
            /* Create artificial dependency */
            asm volatile("" : "=r"(a) : "0"(a) : "memory");
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    checksum += (int)(fa + fb + fc + fd + fe + ff + fg + fh);
    printf("Checksum: %d\n", checksum);
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    volatile int v;
    
    for (int i = 0; i < size; i++) {
        v = arr1[i];
        r0 = v + i;
        r1 = r0 * arr2[i];
        r2 = r1 >> 2;
        r3 = r2 & 0xFF;
        r4 = r3 | arr1[i];
        r5 = r4 ^ arr2[i];
        r6 = r5 * 3;
        r7 = r6 + i;
        r8 = r7 - arr1[i];
        r9 = r8 & 0x7F;
        r10 = r9 | 0x80;
        r11 = r10 * 5;
        r12 = r11 >> 1;
        r13 = r12 ^ 0xAA;
        r14 = r13 + arr2[i];
        r15 = r14 * 7;
        
        /* Force delay slot scheduling pressure */
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
        arr1[i] = r15;
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    /* Execute the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    /* Also run MIPS-specific version if compiling for MIPS */
    mips_specific_kernel(array1, array2, SIZE);
#endif
    
    /* Final computation to use results */
    int total = 0;
    for (int i = 0; i < SIZE; i++) {
        total += array1[i] ^ array2[i];
    }
    printf("Total: %d\n", total);
    
    free(array1);
    free(array2);
    return 0;
}
