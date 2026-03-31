/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -march=i386 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile variable to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to prevent optimization */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    asm volatile("" 
                 : 
                 : "r"(*p1), "r"(*p2), "r"(*p3), "r"(*p4)
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    
    /* Additional operations to ensure the function isn't optimized away */
    *p1 ^= *p2;
    *p3 += *p4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    /* Clobber floating point registers too */
    asm volatile("" 
                 : 
                 : "r"(*f1), "r"(*f2), "r"(*i1)
                 : "eax", "ecx", "edx", "st", "st(1)", "st(2)", "memory");
    
    *f1 = *f1 * *f2 + *i1;
}

/* Function to create complex control flow */
__attribute__((noinline))
int complex_condition(int seed) {
    /* Volatile read to prevent optimization */
    volatile int v = global_seed;
    return (seed ^ v) & 0x3;
}

int main(int argc, char **argv) {
    /* Use argc as a seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Declare MANY local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    a = seed * 1;
    b = seed * 2 + argc;
    c = seed * 3 - argc;
    d = seed * 4 ^ argc;
    e = seed * 5 | argc;
    f = seed * 6 & argc;
    g = seed * 7 + (argc << 2);
    h = seed * 8 - (argc >> 1);
    i = seed * 9 ^ (argc * 3);
    j = seed * 10 | (argc + 1);
    
    /* More variables with different computations */
    k = a + b - c;
    l = d * e / (f + 1);
    m = g ^ h ^ i;
    n = j << (argc & 3);
    o = (a * b) + (c * d) - (e * f);
    p = (g | h) & (i ^ j);
    q = (a + b) * (c - d);
    r = (e & f) | (g ^ h);
    s = i * j - a * b;
    t = c + d + e + f + g + h;
    
    /* Floating point variables for additional pressure */
    fa = (float)a / 3.14159f;
    fb = (float)b * 2.71828f;
    fc = (float)c + fa;
    fd = (float)d - fb;
    fe = fa * fb;
    ff = fc / (fd + 1.0f);
    
    /* Create conditional branches with different register pressure */
    int condition = complex_condition(seed);
    
    /* Switch creates multiple basic blocks */
    switch (condition & 0x3) {
        case 0: {
            /* High pressure path - many live variables across call */
            /* Use volatile to prevent reordering */
            volatile int barrier = getchar();
            
            /* All these variables are live across the call */
            int sum1 = a + b + c + d + e;
            int sum2 = f + g + h + i + j;
            int sum3 = k + l + m + n + o;
            int sum4 = p + q + r + s + t;
            
            /* Call that clobbers registers with many live variables */
            clobber_callee(&sum1, &sum2, &sum3, &sum4);
            
            /* Use results after call */
            a = sum1 ^ sum2;
            b = sum3 & sum4;
            c = sum1 | sum3;
            break;
        }
        
        case 1: {
            /* Different high pressure path with floating point */
            volatile int barrier2 = getchar();
            
            float fsum1 = fa + fb + fc;
            float fsum2 = fd + fe + ff;
            int isum = a + b + c;
            
            clobber_callee2(&fsum1, &fsum2, &isum);
            
            fa = fsum1 * 0.5f;
            fb = fsum2 * 2.0f;
            a = isum ^ barrier2;
            break;
        }
        
        case 2: {
            /* Path with multiple calls creating multiple save/restore sites */
            int tmp1 = a * b - c * d;
            int tmp2 = e * f - g * h;
            int tmp3 = i * j - k * l;
            int tmp4 = m * n - o * p;
            
            /* First call */
            clobber_callee(&tmp1, &tmp2, &a, &b);
            
            /* Computation between calls keeps variables live */
            tmp3 = tmp3 + tmp1;
            tmp4 = tmp4 + tmp2;
            
            /* Second call - different register pressure */
            clobber_callee(&tmp3, &tmp4, &c, &d);
            
            a = tmp1 + tmp3;
            b = tmp2 + tmp4;
            break;
        }
        
        default: {
            /* Lower pressure path for contrast */
            a = b + c;
            d = e + f;
            break;
        }
    }
    
    /* Loop to create multiple instances of caller-save scenarios */
    int checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        /* Recompute some values to keep them live */
        int x = a + iter;
        int y = b - iter;
        int z = c * iter;
        int w = d ^ iter;
        
        /* Nested condition with call */
        if ((seed + iter) & 1) {
            /* Call at what might be end of basic block */
            clobber_callee(&x, &y, &z, &w);
            
            /* Additional computation after call */
            x = x + y;
            z = z - w;
        } else {
            /* Alternative path without call */
            x = x * 2;
            y = y / 2;
        }
        
        /* Use results to prevent elimination */
        checksum += x + y + z + w;
        
        /* Modify some variables for next iteration */
        a ^= x;
        b += y;
        c -= z;
        d |= w;
    }
    
    /* Final computation using many variables to keep them all live */
    int final_result = 
        a + b + c + d + e + f + g + h + i + j +
        k + l + m + n + o + p + q + r + s + t +
        (int)fa + (int)fb + (int)fc + (int)fd + (int)fe + (int)ff +
        checksum;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    return final_result & 0xFF;
}
