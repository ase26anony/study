/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
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
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "memory", "rax", "rcx", "rdx", "rsi", "rdi", 
                   "r8", "r9", "r10", "r11");
    #else
    /* For 32-bit x86 */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    #endif
    
    /* Opaque memory operation */
    if (p1) *p1 ^= 0x55;
    if (p2) *p2 ^= 0xAA;
    if (p3) *p3 += 1;
    if (p4) *p4 -= 1;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    #ifdef __x86_64__
    asm volatile("" : : "r"(f1), "r"(f2) 
                 : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                   "xmm4", "xmm5", "xmm6", "xmm7");
    #else
    asm volatile("" : : "r"(f1), "r"(f2) 
                 : "memory");
    #endif
    
    if (f1) *f1 = *f1 * 0.5f;
    if (f2) *f2 = *f2 * 2.0f;
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-removable computations */
    v1 = seed * 3;
    v2 = seed + global_seed;  /* Use volatile global */
    v3 = v1 ^ v2;
    v4 = v2 - v1;
    v5 = v3 * v4;
    v6 = v5 / (seed + 1);
    v7 = v6 << 3;
    v8 = v7 >> 1;
    v9 = v8 | 0xFF;
    v10 = v9 & 0xAA;
    
    v11 = v10 + v1;
    v12 = v11 * v2;
    v13 = v12 - v3;
    v14 = v13 ^ v4;
    v15 = v14 | v5;
    v16 = v15 & v6;
    v17 = v16 + v7;
    v18 = v17 * v8;
    v19 = v18 - v9;
    v20 = v19 ^ v10;
    
    /* Float computations to use FP registers */
    f1 = (float)v1 * 0.1f;
    f2 = (float)v2 * 0.2f;
    f3 = f1 + f2;
    f4 = f1 * f2;
    f5 = f3 / f4;
    
    /* Call that clobbers many registers - variables are LIVE across call */
    clobber_callee(&v5, &v10, &v15, &v20);
    
    /* More computations after call - keeps variables live */
    v1 = v5 + v10;
    v2 = v15 - v20;
    v3 = v1 * v2;
    
    /* Another call with float registers */
    clobber_callee2(&f1, &f3);
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

int low_pressure_path(int seed) {
    /* Simpler path with less register pressure */
    int a = seed * 2;
    int b = seed + 5;
    return a + b + global_seed;
}

int main(int argc, char **argv) {
    int i, total = 0;
    
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Loop to create multiple call sites with varying conditions */
    for (i = 0; i < 100; i++) {
        /* Complex condition that can't be optimized away */
        int condition = (seed + i) & 0x3F;
        
        if (condition > 32) {
            /* High register pressure path - call at end of basic block */
            total += high_pressure_path(seed + i);
        } else if (condition > 16) {
            /* Medium pressure with different call pattern */
            int x1 = seed * i;
            int x2 = seed + i;
            int x3 = x1 ^ x2;
            int x4 = x2 - x1;
            
            clobber_callee(&x1, &x2, NULL, NULL);
            
            total += x1 + x2 + x3 + x4;
        } else {
            /* Low pressure path */
            total += low_pressure_path(seed + i);
        }
        
        /* Mix in some I/O to prevent reordering */
        if (i % 23 == 0) {
            /* getchar() is a call with side effects */
            int c = getchar();
            if (c != EOF) {
                seed ^= c;
            }
        }
        
        /* Another conditional with nested calls */
        if ((i & 7) == 0) {
            int y1 = total * 3;
            int y2 = total / 2;
            int y3 = y1 | y2;
            int y4 = y1 & y2;
            
            /* Call in the middle of computations */
            clobber_callee(&y1, &y3, NULL, NULL);
            
            y2 = y1 + y3;
            y4 = y2 - y4;
            
            total = y4 ^ (seed + i);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
