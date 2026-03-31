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
    /* Inline asm to clobber specific registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
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
                 : "eax", "ecx", "edx", "st", "st(1)", "st(2)", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2) : "memory");
    #endif
    
    if (f1) *f1 += 1.5f;
    if (f2) *f2 += 2.5f;
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    v1 = seed * 3;
    v2 = seed + global_seed;  /* Use volatile global */
    v3 = v1 ^ v2;
    v4 = v2 * 7;
    v5 = v3 - v4;
    v6 = v4 / 3;
    v7 = v5 | v6;
    v8 = v6 & v7;
    v9 = v7 ^ v8;
    v10 = v8 * v9;
    
    v11 = v9 + 11;
    v12 = v10 - 12;
    v13 = v11 * v12;
    v14 = v12 ^ v13;
    v15 = v13 + v14;
    v16 = v14 - v15;
    v17 = v15 * v16;
    v18 = v16 ^ v17;
    v19 = v17 + v18;
    v20 = v18 * v19;
    
    /* Float computations to use FP registers */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 2.2f;
    f3 = f1 + f2;
    f4 = f2 - f1;
    f5 = f3 * f4;
    
    /* Call clobbering function - many registers live across this call */
    clobber_callee(&v5, &v10, &v15, &v20);
    
    /* More computations after call to keep variables live */
    v1 = v5 + v10;
    v2 = v15 - v20;
    v3 = v1 * v2;
    
    /* Another call with float registers live */
    clobber_callee2(&f3, &f5);
    
    /* Use results to prevent elimination */
    return v3 + (int)f3 + (int)f5 + v19;
}

int low_pressure_path(int seed) {
    /* Simpler path with less register pressure */
    int a = seed * 2;
    int b = a + 5;
    return a * b;
}

/* Function with multiple call sites in different basic blocks */
int complex_control_flow(int seed, int mode) {
    int result = 0;
    
    /* First conditional block */
    if (mode & 1) {
        /* High pressure path */
        int x1 = seed * 3, x2 = seed + 7, x3 = x1 ^ x2;
        int x4 = x2 * 11, x5 = x3 - x4, x6 = x4 / 5;
        
        /* Call at what might be end of basic block */
        clobber_callee(&x1, &x3, &x5, NULL);
        
        result += x1 + x3 + x5 + x6;
    } else {
        /* Low pressure alternative */
        result += seed * 2;
    }
    
    /* Second conditional with different structure */
    if (mode & 2) {
        /* Different set of variables */
        int y1 = result * 5, y2 = result + global_seed;
        int y3 = y1 & y2, y4 = y2 | y3;
        float fy1 = (float)y1 * 0.5f, fy2 = (float)y2 * 1.5f;
        
        /* This call might be at block end */
        clobber_callee2(&fy1, &fy2);
        
        result += y3 + y4 + (int)fy1;
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 3; i++) {
        int z1 = result + i * 7;
        int z2 = z1 ^ (i * 3);
        
        if ((seed + i) % 3 == 0) {
            /* Call inside loop with live variables */
            int z3 = z1 * z2;
            clobber_callee(&z1, &z2, &z3, NULL);
            result += z3;
        } else {
            result += z1 + z2;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    /* Use volatile intermediate to prevent optimization */
    volatile int vol_seed = seed;
    
    int total = 0;
    
    /* Multiple test cases with different register pressure scenarios */
    for (int i = 0; i < 5; i++) {
        int mode = (vol_seed + i) % 4;
        
        if (mode == 0) {
            /* Path that should trigger high register pressure */
            total += high_pressure_path(vol_seed + i);
        } else if (mode == 1) {
            /* Complex control flow path */
            total += complex_control_flow(vol_seed + i, i);
        } else {
            /* Lower pressure path */
            total += low_pressure_path(vol_seed + i);
        }
        
        /* Use getchar to create side effects and prevent reordering */
        if (i % 2 == 0) {
            /* This call also affects register allocation */
            int c = getchar();
            if (c != EOF) {
                total += c;
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total % 256;
}
