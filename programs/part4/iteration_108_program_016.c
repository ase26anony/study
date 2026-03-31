/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4, int *p5) {
    /* Inline asm to clobber many x86 registers */
    #ifdef __x86_64__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) 
                 : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    #else
    /* For 32-bit x86, clobber all caller-saved registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) 
                 : "eax", "ecx", "edx", "esi", "edi", "memory");
    #endif
    
    /* Do some actual work to prevent removal */
    *p1 += *p2;
    *p3 ^= *p4;
    *p5 = *p1 + *p3;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __x86_64__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1)
                 : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
                   "rax", "rcx", "rdx", "rsi", "rdi", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1)
                 : "st", "st(1)", "st(2)", "st(3)", 
                   "eax", "ecx", "edx", "esi", "edi", "memory");
    #endif
    
    *f1 = *f2 * 2.0f;
    *i1 = (int)(*f1);
}

/* Function to create complex control flow */
__attribute__((noinline))
int complex_function(int seed, int mode) {
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed ^ 0x1234;
    v4 = global_seed;  /* volatile read */
    v5 = v1 + v2 + v3 + v4;
    
    v6 = (seed << 3) | (seed >> 5);
    v7 = v5 * v6;
    v8 = v7 - v4;
    v9 = v8 / (v1 | 1);
    v10 = v9 ^ v2;
    
    v11 = getchar();  /* I/O function - can't be moved */
    v12 = v11 + v10;
    v13 = v12 * v3;
    v14 = v13 & 0xFF;
    v15 = v14 | v5;
    
    v16 = v15 << 2;
    v17 = v16 >> 1;
    v18 = v17 + v8;
    v19 = v18 - v9;
    v20 = v19 ^ v10;
    
    /* Float variables for floating point register pressure */
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = f1 + f2;
    f4 = f3 * 0.75f;
    f5 = f4 - f1;
    
    /* Complex conditional creating different basic blocks */
    if (mode == 0) {
        /* High register pressure path - call at end of basic block */
        /* All variables are live across this call */
        clobber_callee(&v1, &v2, &v3, &v4, &v5);
        
        /* Use results after call to keep them live */
        v6 = v1 + v2;
        v7 = v3 * v4;
    } 
    else if (mode == 1) {
        /* Another high pressure path with different call */
        clobber_callee2(&f1, &f2, &v8);
        
        v9 = v8 + v10;
        v11 = v9 * v12;
    }
    else if (mode == 2) {
        /* Path with nested conditionals */
        if (seed & 1) {
            clobber_callee(&v13, &v14, &v15, &v16, &v17);
            v18 = v13 + v14;
        } else {
            clobber_callee2(&f3, &f4, &v19);
            v20 = v19 * 2;
        }
    }
    else {
        /* Low pressure path - no call, simpler computation */
        v1 = v2 + v3;
        v4 = v5 * v6;
    }
    
    /* Switch statement to create more control flow */
    switch (seed & 0x3) {
        case 0:
            /* Call at end of switch case basic block */
            clobber_callee(&v1, &v3, &v5, &v7, &v9);
            v11 = v1 + v3;
            break;
        case 1:
            clobber_callee2(&f1, &f5, &v2);
            v4 = v2 * 3;
            break;
        case 2:
            /* Nested loop with call */
            for (int i = 0; i < 3; i++) {
                v6 += i;
                /* Call inside loop - different block structure */
                if (i == 1) {
                    clobber_callee(&v8, &v10, &v12, &v14, &v16);
                    v18 = v8 + v10;
                }
            }
            break;
        default:
            v20 = v19 - v18;
            break;
    }
    
    /* Use all variables in final computation to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Main function with loops creating multiple call sites */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    int total = 0;
    
    /* Loop to create multiple caller-save scenarios */
    for (int i = 0; i < 100; i++) {
        int mode = (seed + i) % 4;
        
        /* Call function with high register pressure */
        int result = complex_function(seed + i, mode);
        
        total += result;
        
        /* Modify global volatile to prevent optimizations */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Another call site with different pressure */
        if (i % 3 == 0) {
            int a = result + 1;
            int b = result * 2;
            int c = result ^ 0xABCD;
            int d = global_seed;
            int e = a + b + c + d;
            
            clobber_callee(&a, &b, &c, &d, &e);
            
            total += a + b + c + d + e;
        }
        
        /* Call with float pressure every 5 iterations */
        if (i % 5 == 0) {
            float f1 = (float)result * 1.1f;
            float f2 = (float)total * 0.9f;
            int idx = result & 0xFF;
            
            clobber_callee2(&f1, &f2, &idx);
            
            total += idx + (int)f1 + (int)f2;
        }
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
