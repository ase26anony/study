/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -march=i386 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
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
    asm volatile (
        "# Clobbering eax, ecx, edx\n"
        :
        : 
        : "eax", "ecx", "edx", "memory"
    );
    /* Opaque memory operations */
    if (p1) *p1 ^= 1;
    if (p2) *p2 ^= 2;
    if (p3) *p3 ^= 4;
    if (p4) *p4 ^= 8;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    /* Clobber x87/MMX registers */
    asm volatile (
        "# Clobbering floating point registers\n"
        :
        :
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "memory"
    );
    if (f1) *f1 += 1.0f;
    if (f2) *f2 -= 1.0f;
}

/* Function with high register pressure around calls */
__attribute__((noinline, noclone))
int high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    volatile int vol = global_seed + iter;
    
    v1 = seed + vol;
    v2 = v1 * 3;
    v3 = v2 - vol;
    v4 = v3 ^ seed;
    v5 = v4 * 7;
    v6 = v5 + iter;
    v7 = v6 & 0xFF;
    v8 = v7 | 0x100;
    v9 = v8 * v1;
    v10 = v9 / (seed + 1);
    
    v11 = v10 + v2;
    v12 = v11 * v3;
    v13 = v12 - v4;
    v14 = v13 ^ v5;
    v15 = v14 | v6;
    v16 = v15 & v7;
    v17 = v16 * v8;
    v18 = v17 + v9;
    v19 = v18 - v10;
    v20 = v19 ^ v11;
    
    /* Floating point variables to increase pressure */
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = (float)v3 * 2.5f;
    f4 = (float)v4 * 3.5f;
    f5 = (float)v5 * 4.5f;
    
    /* Complex conditional to create different basic blocks */
    int result = 0;
    
    /* Path 1: High register pressure with call at block end */
    if ((seed & 0x3) == 0) {
        /* Use all variables in computation before call */
        int temp1 = v1 + v2 + v3 + v4 + v5;
        int temp2 = v6 + v7 + v8 + v9 + v10;
        int temp3 = v11 + v12 + v13 + v14 + v15;
        int temp4 = v16 + v17 + v18 + v19 + v20;
        
        /* Call with many live variables - forces caller-saves */
        clobber_callee(&temp1, &temp2, &temp3, &temp4);
        
        /* Use results after call - keeps variables live */
        result = temp1 + temp2 + temp3 + temp4;
        
        /* Another call in the same block */
        clobber_callee2(&f1, &f2);
        result += (int)(f1 + f2);
        
        /* BB_END should be the last call before insertion */
    }
    /* Path 2: Different pressure pattern */
    else if ((seed & 0x3) == 1) {
        /* Different computation */
        result = v1 * v3 * v5 * v7 * v9;
        
        /* Call in middle of block */
        clobber_callee(&v2, &v4, &v6, &v8);
        
        /* More computations after call */
        result += v10 * v12 * v14 * v16 * v18;
        
        /* Another conditional inside */
        if (iter & 1) {
            clobber_callee2(&f3, &f4);
            result += (int)(f3 * f4);
        }
    }
    /* Path 3: Loop with calls */
    else if ((seed & 0x3) == 2) {
        for (int i = 0; i < 3; i++) {
            /* Varying live variables */
            int mix = v1 + v3 * i;
            clobber_callee(&mix, &v5, &v7, &v9);
            result += mix + i;
        }
    }
    /* Path 4: Switch statement for more block variety */
    else {
        switch (iter & 0x7) {
            case 0:
                clobber_callee(&v1, &v3, &v5, &v7);
                result = v1 + v3;
                break;
            case 1:
                clobber_callee(&v2, &v4, &v6, &v8);
                result = v2 + v4;
                break;
            case 2:
                clobber_callee2(&f1, &f5);
                result = (int)(f1 + f5);
                break;
            default:
                /* Nested condition with call at end */
                if (v10 > 100) {
                    clobber_callee(&v10, &v11, &v12, &v13);
                    result = v10;
                } else {
                    result = v20;
                }
                break;
        }
    }
    
    /* Final computation using all variables to keep them live */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)(f1 + f2 + f3 + f4 + f5);
    
    return result;
}

/* Main function with varying conditions */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    int total = 0;
    
    /* Create varying conditions across iterations */
    for (int i = 0; i < 100; i++) {
        /* Mix of conditions to exercise different paths */
        int iter_seed = seed ^ (i * 137);
        
        /* Call high-pressure function multiple times */
        int result = high_pressure_function(iter_seed, i);
        
        /* Use result to prevent elimination */
        total += result;
        
        /* Occasionally call clobbering functions directly */
        if ((i & 0xF) == 0) {
            int a = i, b = i*2, c = i*3, d = i*4;
            clobber_callee(&a, &b, &c, &d);
            total += a + b + c + d;
        }
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
