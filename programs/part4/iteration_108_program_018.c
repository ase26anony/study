/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_seed;

/* Function that clobbers many registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Force register clobbering with inline asm */
    #ifdef __x86_64__
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    #elif __i386__
    asm volatile("" ::: "eax", "ecx", "edx", "esi", "edi", "memory");
    #elif __riscv
    asm volatile("" ::: "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" ::: "memory");
    #endif
    
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __x86_64__
    asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
                       "rax", "rcx", "rdx", "rsi", "rdi", "memory");
    #elif __i386__
    asm volatile("" ::: "st", "st(1)", "st(2)", "st(3)", 
                       "eax", "ecx", "edx", "esi", "edi", "memory");
    #endif
    
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
    if (i1) *i1 += 5;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to prevent optimization */
    global_seed = argc;
    volatile int vseed = argc * 3 + 1;
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Declare MANY local variables to create register pressure */
        int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
        float fa, fb, fc, fd, fe, ff;
        
        /* Initialize with complex arithmetic to prevent optimization */
        a = vseed + iteration * 7;
        b = a * 3 - vseed;
        c = b / 2 + a;
        d = c ^ (vseed << 3);
        e = d * 5 - b;
        f = e + c * 2;
        g = f ^ (iteration * 11);
        h = g - d + 7;
        i = h * 3 / 2;
        j = i ^ (vseed >> 1);
        k = j + a + b + c;
        l = k * 2 - g;
        m = l ^ (iteration * 13);
        n = m + h - i;
        o = n * 3 / 4;
        p = o ^ j;
        q = p + k + l;
        r = q * 5 - m;
        s = r ^ (vseed * 17);
        t = s + n + o + p;
        
        /* Float variables for floating point register pressure */
        fa = (float)a / 3.0f;
        fb = (float)b * 1.5f;
        fc = fa + fb;
        fd = fc * 2.0f - fa;
        fe = fd / 1.7f;
        ff = fe + fb - fc;
        
        /* Read volatile global to create memory barrier */
        int barrier = global_seed;
        
        /* Complex conditional to create different basic blocks */
        if ((barrier + iteration) % 3 != 0) {
            /* High register pressure path - call at end of basic block */
            
            /* Use all variables in computation before call */
            int sum1 = a + b + c + d + e + f + g;
            int sum2 = h + i + j + k + l + m + n;
            int sum3 = o + p + q + r + s + t;
            float fsum = fa + fb + fc + fd + fe + ff;
            
            /* Call that clobbers registers - this should trigger caller-save */
            /* Pass addresses to prevent keeping values only in registers */
            clobber_callee(&sum1, &sum2, &sum3, NULL);
            
            /* Use results after call - keeps variables live across call */
            result += sum1 * 2 + sum2 - sum3;
            result += (int)fsum;
            
            /* Another conditional inside to potentially split block */
            if (sum1 > 100) {
                /* Second call with different register pressure */
                clobber_callee2(&fa, &fb, &sum1);
                result += fa * 3;
            }
        } else {
            /* Low pressure path - no call or different call pattern */
            int tmp = a + b + c;
            result += tmp * 7;
            
            /* Different call site with different live variables */
            if (iteration % 2 == 0) {
                clobber_callee(&d, &e, &f, &g);
                result += d + e;
            }
        }
        
        /* Mix in some I/O which can't be optimized away */
        if (iteration == 1) {
            volatile char ch = getchar();
            result += ch;
        }
        
        /* Switch statement to create more complex control flow */
        switch (iteration) {
            case 0: {
                int x = a * b + c;
                int y = d * e + f;
                /* Call at end of switch case block */
                clobber_callee(&x, &y, &g, &h);
                result += x + y;
                break;
            }
            case 1: {
                float fx = fa * fb;
                float fy = fc * fd;
                /* Different type of call */
                clobber_callee2(&fx, &fy, &i);
                result += (int)(fx + fy);
                break;
            }
            default: {
                /* Multiple calls in sequence */
                int tmp1 = j + k;
                clobber_callee(&tmp1, &l, NULL, NULL);
                int tmp2 = m + n;
                clobber_callee(&tmp2, &o, NULL, NULL);
                result += tmp1 + tmp2;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
