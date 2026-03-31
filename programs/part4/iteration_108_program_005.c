/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7"
#else
#define CLOBBER_LIST "memory"
#endif

/* Noinline function that clobbers registers */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4, int *p5) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : CLOBBER_LIST);
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
    *p4 += 4;
    *p5 += 5;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, float *f3) {
    asm volatile("" : : "r"(f1), "r"(f2), "r"(f3) : CLOBBER_LIST);
    *f1 += 1.0f;
    *f2 += 2.0f;
    *f3 += 3.0f;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed */
    global_seed = argc;
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 3; iter++) {
        /* Declare MANY local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        
        /* Initialize with complex arithmetic to prevent constant folding */
        v1 = global_seed + iter * 1;
        v2 = global_seed + iter * 2;
        v3 = global_seed + iter * 3;
        v4 = global_seed + iter * 4;
        v5 = global_seed + iter * 5;
        v6 = global_seed + iter * 6;
        v7 = global_seed + iter * 7;
        v8 = global_seed + iter * 8;
        v9 = global_seed + iter * 9;
        v10 = global_seed + iter * 10;
        v11 = global_seed + iter * 11;
        v12 = global_seed + iter * 12;
        v13 = global_seed + iter * 13;
        v14 = global_seed + iter * 14;
        v15 = global_seed + iter * 15;
        v16 = global_seed + iter * 16;
        v17 = global_seed + iter * 17;
        v18 = global_seed + iter * 18;
        v19 = global_seed + iter * 19;
        v20 = global_seed + iter * 20;
        
        f1 = (float)v1 * 0.1f;
        f2 = (float)v2 * 0.2f;
        f3 = (float)v3 * 0.3f;
        f4 = (float)v4 * 0.4f;
        f5 = (float)v5 * 0.5f;
        f6 = (float)v6 * 0.6f;
        f7 = (float)v7 * 0.7f;
        f8 = (float)v8 * 0.8f;
        
        /* Complex computations to keep variables live */
        v1 = v1 * v2 + v3;
        v2 = v2 * v3 + v4;
        v3 = v3 * v4 + v5;
        v4 = v4 * v5 + v6;
        v5 = v5 * v6 + v7;
        v6 = v6 * v7 + v8;
        v7 = v7 * v8 + v9;
        v8 = v8 * v9 + v10;
        v9 = v9 * v10 + v11;
        v10 = v10 * v11 + v12;
        
        f1 = f1 * f2 + f3;
        f2 = f2 * f3 + f4;
        f3 = f3 * f4 + f5;
        f4 = f4 * f5 + f6;
        
        /* Conditional to create different basic blocks */
        if ((global_seed + iter) % 2 == 0) {
            /* High register pressure path - call at end of basic block */
            
            /* More computations to increase live range */
            v11 = v11 * v12 + v13;
            v12 = v12 * v13 + v14;
            v13 = v13 * v14 + v15;
            v14 = v14 * v15 + v16;
            v15 = v15 * v16 + v17;
            v16 = v16 * v17 + v18;
            v17 = v17 * v18 + v19;
            v18 = v18 * v19 + v20;
            
            f5 = f5 * f6 + f7;
            f6 = f6 * f7 + f8;
            
            /* Call that clobbers registers - many variables are live */
            clobber_callee(&v1, &v2, &v3, &v4, &v5);
            
            /* This call is at the end of the basic block */
            clobber_callee2(&f1, &f2, &f3);
            
            /* BB_END should be the call instruction before insertion */
        } else {
            /* Alternative path with less pressure */
            v1 = v1 + v2;
            v3 = v3 + v4;
            f1 = f1 + f2;
        }
        
        /* Use all variables after conditional to keep them live */
        int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        float fsum = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
        
        result += sum + (int)fsum;
        
        /* Another conditional with different call pattern */
        if ((global_seed + iter) % 3 == 0) {
            int t1 = v1 * 2, t2 = v2 * 3, t3 = v3 * 4, t4 = v4 * 5, t5 = v5 * 6;
            float ft1 = f1 * 1.5f, ft2 = f2 * 2.5f, ft3 = f3 * 3.5f;
            
            /* Different call site */
            clobber_callee(&t1, &t2, &t3, &t4, &t5);
            
            /* Force BB_END to be the call */
            if (t1 > 0) {
                clobber_callee2(&ft1, &ft2, &ft3);
            }
            
            result += t1 + t2 + t3 + t4 + t5 + (int)(ft1 + ft2 + ft3);
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test case in separate function */
    test_another_case();
    
    return result != 0;
}

/* Another function with different register pressure pattern */
__attribute__((noinline, noclone))
void test_another_case(void) {
    volatile int seed = time(NULL) % 100;
    
    /* Many variables with different types */
    int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4, a5 = seed + 5;
    int b1 = seed * 2, b2 = seed * 3, b3 = seed * 4, b4 = seed * 5, b5 = seed * 6;
    int c1 = seed + 10, c2 = seed + 20, c3 = seed + 30, c4 = seed + 40, c5 = seed + 50;
    float fa = seed * 0.1f, fb = seed * 0.2f, fc = seed * 0.3f, fd = seed * 0.4f;
    
    /* Complex computation network */
    for (int i = 0; i < 2; i++) {
        a1 = a1 * a2 + a3;
        a2 = a2 * a3 + a4;
        a3 = a3 * a4 + a5;
        a4 = a4 * a5 + b1;
        a5 = a5 * b1 + b2;
        
        b1 = b1 * b2 + b3;
        b2 = b2 * b3 + b4;
        b3 = b3 * b4 + b5;
        b4 = b4 * b5 + c1;
        b5 = b5 * c1 + c2;
        
        fa = fa * fb + fc;
        fb = fb * fc + fd;
        fc = fc * fd + fa;
        fd = fd * fa + fb;
        
        /* Call inside loop with high pressure */
        if (i == 0) {
            clobber_callee(&a1, &a2, &a3, &a4, &a5);
        } else {
            clobber_callee2(&fa, &fb, &fc);
        }
    }
    
    /* Use results */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5),
                       "r"(c1), "r"(c2), "r"(c3), "r"(c4), "r"(c5));
}
