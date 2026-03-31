/* haifa_sched_trigger.c
 * Designed to trigger GCC's Haifa scheduler state save/restore mechanism
 * and exercise the free_sched_context cleanup path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
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
    e = a * b; f = b / (a | 1); g = a ^ b;
    h = a & b; i = a | b; j = ~a; k = ~b;
    
    fa = (float)a; fb = (float)b; fc = fa * fb;
    fd = fa / (fb + 1.0f); fe = fa - fb; ff = fa + fb;
    
    /* Main computation with data-dependent branching */
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch - hard to predict */
        if (__builtin_expect((arr1[idx] ^ arr2[idx]) & 1, 0)) {
            /* Path 1: Integer-heavy operations */
            m = arr1[idx] * arr2[idx];
            n = arr1[idx] + arr2[idx];
            o = arr1[idx] - arr2[idx];
            p = arr1[idx] & arr2[idx];
            
            /* Chain of dependent operations */
            a = a + m;
            b = b - n;
            c = c * o;
            d = d ^ p;
            e = e & m;
            f = f | n;
            g = g ^ o;
            h = h + p;
            
            /* Floating-point operations mixed in */
            fg = (float)m + fc;
            fh = (float)n - fd;
            fa = fa * fg;
            fb = fb / (fh + 1.0f);
            
            /* Memory barrier - creates serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            i = i + (int)fa;
            j = j - (int)fb;
            k = k * (int)fc;
            l = l ^ (int)fd;
        } else {
            /* Path 2: Different operation mix */
            m = arr1[idx] | arr2[idx];
            n = arr1[idx] ^ arr2[idx];
            o = arr1[idx] + arr2[idx] * 3;
            p = arr1[idx] - arr2[idx] / 2;
            
            /* Different dependency chain */
            a = a ^ m;
            b = b & n;
            c = c + o;
            d = d - p;
            e = e * m;
            f = f ^ n;
            g = g & o;
            h = h | p;
            
            /* Different FP operations */
            fg = (float)o * 1.5f;
            fh = (float)p / 2.0f;
            fc = fc + fg;
            fd = fd - fh;
            fe = fe * fg;
            ff = ff / (fh + 1.0f);
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Continue computation */
            i = i & (int)fe;
            j = j | (int)ff;
            k = k + (int)fg;
            l = l - (int)fh;
        }
        
        /* Complex switch to create CFG complexity */
        switch (arr1[idx] & 0x3) {
            case 0:
                a = a * 2;
                /* fall through */
            case 1:
                b = b + a;
                /* Jump to common label */
                goto common_ops;
            case 2:
                c = c - b;
                /* fall through */
            case 3:
                d = d ^ c;
                /* Jump to common label */
                goto common_ops;
            default:
                break;
        }
        
        /* This label creates multiple predecessors */
        common_ops:
        e = e + (arr2[idx] & 0xFF);
        f = f - (arr1[idx] & 0xFF);
        
        /* Force potential spilling with many live variables */
        g = g * e;
        h = h / (f | 1);
        i = i + g;
        j = j - h;
        k = k * i;
        l = l ^ j;
        m = m + k;
        n = n - l;
        o = o * m;
        p = p / (n | 1);
        
        /* Use all float variables to keep them live */
        fa = fa + (float)o;
        fb = fb - (float)p;
        fc = fc * fa;
        fd = fd / (fb + 1.0f);
        fe = fe + fc;
        ff = ff - fd;
        fg = fg * fe;
        fh = fh / (ff + 1.0f);
    }
    
    /* Use results to prevent elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    volatile float fresult = fa + fb + fc + fd + fe + ff + fg + fh;
    (void)result;
    (void)fresult;
}

/* MIPS-specific version with delay slot patterns */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_pattern(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Pattern that might engage MIPS delay slot scheduling */
        asm volatile(
            "lw $0, %0\n\t"
            "nop\n\t"
            "addu $0, $0, %1\n\t"
            : : "m"(arr[i]), "r"(i) : "memory"
        );
        sum += arr[i];
    }
    volatile int keep = sum;
    (void)keep;
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
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        array2[i] = (int)(seed >> 8) & 0x7FFF;
    }
    
    /* Execute the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    /* Include MIPS-specific patterns if compiling for MIPS */
    mips_specific_pattern(array1, SIZE);
#endif
    
    /* Compute checksum to prevent dead code elimination */
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
