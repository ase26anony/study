/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Force register clobbering with inline asm */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #elif __x86_64__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Modify through pointers to create dependencies */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2) 
                 : "eax", "ecx", "edx", "st", "st(1)", "st(2)", "st(3)", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2) : "memory");
    #endif
    
    if (f1) *f1 += 1.5f;
    if (f2) *f2 += 2.5f;
}

int main(int argc, char **argv) {
    /* Use argc for some runtime variation */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    volatile int vseed = seed; /* volatile to prevent optimizations */
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Declare MANY local variables to create register pressure */
        int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
        float fa, fb, fc, fd, fe, ff;
        
        /* Initialize with complex, non-optimizable computations */
        a = vseed + iteration * 7;
        b = a * 3 - vseed;
        c = b ^ (a << 3);
        d = c + global_seed;
        e = d * 2 - b;
        f = e ^ (iteration * 0x1234);
        g = f + a - c;
        h = g * 3;
        i = h ^ d;
        j = i + e;
        k = j * 5;
        l = k - f;
        m = l ^ g;
        n = m + h;
        o = n * 7;
        p = o ^ i;
        q = p + j;
        r = q * 11;
        s = r ^ k;
        t = s + l;
        
        /* Float computations */
        fa = (float)a * 1.1f;
        fb = (float)b * 1.2f;
        fc = (float)c * 1.3f;
        fd = (float)d * 1.4f;
        fe = (float)e * 1.5f;
        ff = (float)f * 1.6f;
        
        /* Complex conditional to create different basic blocks */
        if ((vseed + iteration) % 3 == 0) {
            /* Path 1: High register pressure before call */
            /* Use most variables in computation before call */
            int temp1 = a + b + c + d + e;
            int temp2 = f + g + h + i + j;
            int temp3 = k + l + m + n + o;
            int temp4 = p + q + r + s + t;
            
            /* Call with many live variables - forces caller-saves */
            clobber_callee(&temp1, &temp2, &temp3, &temp4);
            
            /* Use results after call - keeps variables live across call */
            a = temp1 + 1;
            b = temp2 + 2;
            c = temp3 + 3;
            d = temp4 + 4;
            
            /* Another call with float registers */
            clobber_callee2(&fa, &fb);
            
            /* More computations to use float results */
            fc = fa + fb;
            fd = fc * 2.0f;
            
        } else if ((vseed + iteration) % 3 == 1) {
            /* Path 2: Medium pressure with different call pattern */
            int temp1 = a * b - c;
            int temp2 = d * e - f;
            
            clobber_callee(&temp1, &temp2, &g, &h);
            
            a = temp1 + temp2;
            b = g * h;
            
            clobber_callee2(&fc, &fd);
            
        } else {
            /* Path 3: Low pressure path - no function calls in this block */
            a = b + c + d;
            e = f + g + h;
            i = j + k + l;
            m = n + o + p;
            q = r + s + t;
        }
        
        /* Use all variables after conditional to keep them live */
        /* Complex computation that can't be easily optimized away */
        int sum = a + b + c + d + e + f + g + h + i + j;
        sum += k + l + m + n + o + p + q + r + s + t;
        
        /* Mix in float values via conversion */
        sum += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe + (int)ff;
        
        /* Add some volatile operations to prevent reordering */
        sum ^= global_seed;
        sum += vseed;
        
        /* Use result to prevent dead code elimination */
        result += sum;
        
        /* Another conditional with call at the end of block */
        if (sum % 2 == 0) {
            int x = a * 3 + b * 5;
            int y = c * 7 + d * 11;
            int z = e * 13 + f * 17;
            int w = g * 19 + h * 23;
            
            /* Call at what might be block end */
            clobber_callee(&x, &y, &z, &w);
            
            a = x + y;
            b = z + w;
        } else {
            a = b * 2;
            b = c * 3;
        }
        
        /* Final use of variables */
        result += a * 3 - b * 7;
    }
    
    /* Ensure result is used */
    printf("Result: %d (seed was: %d)\n", result, seed);
    
    return result != 0 ? 0 : 1;
}
