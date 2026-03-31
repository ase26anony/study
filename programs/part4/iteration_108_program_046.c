/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
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
    /* Inline asm to clobber specific x86 registers */
    #ifdef __x86_64__
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    #elif defined(__i386__)
    asm volatile("" : : : "eax", "ecx", "edx", "esi", "edi");
    #endif
    
    /* Opaque memory operations */
    *p1 = *p1 + 1;
    *p2 = *p2 - 1;
    if (p3) *p3 = *p3 ^ 0x55;
    if (p4) *p4 = *p4 | 0xAA;
    
    /* Memory clobber */
    asm volatile("" : : : "memory");
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    #ifdef __x86_64__
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    #elif defined(__i386__)
    asm volatile("" : : : "st", "st(1)", "st(2)", "st(3)");
    #endif
    
    if (f1) *f1 = *f1 * 2.0f;
    if (f2) *f2 = *f2 / 2.0f;
    asm volatile("" : : : "memory");
}

/* Function with high register pressure around calls */
__attribute__((noinline))
int high_pressure_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 3 + 1;
    v2 = seed / 2 - 5;
    v3 = seed ^ 0x1234;
    v4 = seed | 0xABCD;
    v5 = (seed << 3) | (seed >> 5);
    v6 = seed * seed;
    v7 = seed + global_seed;
    v8 = seed - global_seed;
    v9 = ~seed;
    v10 = seed % 17;
    
    /* More variables with data dependencies */
    v11 = v1 + v2;
    v12 = v3 * v4;
    v13 = v5 ^ v6;
    v14 = v7 | v8;
    v15 = v9 & v10;
    v16 = v11 - v12;
    v17 = v13 + v14;
    v18 = v15 * v16;
    v19 = v17 ^ v18;
    v20 = v19 + seed;
    
    /* Float variables for floating point register pressure */
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = (float)v3 * 0.5f;
    f4 = (float)v4 * 3.5f;
    f5 = (float)v5 * 4.5f;
    
    /* Complex conditional to create basic block boundaries */
    if (seed & 1) {
        /* Path 1: High register pressure call at end of basic block */
        
        /* More computations to keep variables live */
        v1 = v1 + v20;
        v2 = v2 * v19;
        v3 = v3 | v18;
        v4 = v4 ^ v17;
        v5 = v5 & v16;
        
        f1 = f1 + f5;
        f2 = f2 * f4;
        f3 = f3 - f1;
        
        /* Call that clobbers registers - many variables are live across this call */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* This is the end of the basic block - the call instruction should be BB_END */
        /* The caller-save pass should insert save/restore around the call */
        
    } else if (seed & 2) {
        /* Path 2: Different high pressure scenario */
        v6 = v6 + v15;
        v7 = v7 * v14;
        v8 = v8 | v13;
        v9 = v9 ^ v12;
        v10 = v10 & v11;
        
        f4 = f4 + f2;
        f5 = f5 * f3;
        
        /* Call with different arguments */
        clobber_callee(&v6, &v7, &v8, &v9);
        clobber_callee2(&f4, &f5);
        
    } else {
        /* Path 3: Simpler path for contrast */
        v20 = v20 * 2;
    }
    
    /* Use all variables after the call to keep them live across calls */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    /* Another conditional with call at block end */
    if (result > 1000) {
        v1 = v1 ^ result;
        v2 = v2 | result;
        clobber_callee(&v1, &v2, &v3, &v4);
        /* Another potential BB_END with call */
    }
    
    return result;
}

/* Function with loop creating multiple call sites */
__attribute__((noinline))
int loop_with_calls(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying conditions create different basic blocks */
        int cond = i & 3;
        
        /* Local variables in loop create register pressure */
        int a = i * 3;
        int b = i + 7;
        int c = i ^ 0xFF;
        int d = i | 0xAA;
        int e = i & 0x55;
        int f = i << 2;
        int g = i >> 1;
        int h = i % 13;
        int j = i * i;
        int k = i + global_seed;
        
        if (cond == 0) {
            /* Call at end of block with many live variables */
            a = a + b + c;
            b = b * d;
            c = c ^ e;
            clobber_callee(&a, &b, &c, &d);
            total += a + b;
        } else if (cond == 1) {
            /* Different call pattern */
            e = e + f + g;
            f = f * h;
            clobber_callee(&e, &f, &g, &h);
            total += e + f;
        } else if (cond == 2) {
            /* Nested condition with call */
            if (i & 1) {
                j = j + k;
                clobber_callee(&j, &k, &a, &b);
                total += j;
            } else {
                k = k * 2;
                total += k;
            }
        } else {
            /* Call in default case */
            clobber_callee(&total, &i, NULL, NULL);
        }
        
        /* Volatile operation to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use argc for deterministic but varying behavior */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    printf("Seed: %d\n", seed);
    
    /* Create register pressure in main as well */
    int x1 = seed * 2;
    int x2 = seed + 100;
    int x3 = seed ^ 0xDEAD;
    int x4 = seed | 0xBEEF;
    int x5 = seed & 0xCAFE;
    int x6 = seed << 4;
    int x7 = seed >> 2;
    int x8 = seed % 23;
    int x9 = seed * 11;
    int x10 = seed - 50;
    
    /* Call high pressure function */
    int result1 = high_pressure_function(seed);
    
    /* More computations to keep variables live */
    x1 = x1 + result1;
    x2 = x2 * result1;
    x3 = x3 | result1;
    
    /* Another call with different arguments */
    clobber_callee(&x1, &x2, &x3, &x4);
    
    /* Call loop function */
    int result2 = loop_with_calls(10 + (seed % 5));
    
    /* Use all results to prevent elimination */
    int final_result = result1 + result2 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
