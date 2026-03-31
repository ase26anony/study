/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - x86 specific */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif defined(__i386__)
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
#else
/* Generic - will still work but may not clobber as many registers */
#define CLOBBER_LIST "memory"
#endif

/* Noinline function that clobbers call-clobbered registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4, int *p5) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : CLOBBER_LIST);
    
    /* Force memory side effect */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
    if (p5) *p5 += 5;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1) {
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : CLOBBER_LIST);
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
    if (i1) *i1 += 3;
}

/* Function with high register pressure around calls */
int __attribute__((noinline))
high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-removable computations */
    v1 = seed + iter * 1;
    v2 = seed * 2 - iter;
    v3 = (seed << 3) | (iter & 0xFF);
    v4 = seed ^ iter;
    v5 = seed % (iter + 1) + 1;
    
    /* Chain computations to create data dependencies */
    v6 = v1 + v2;
    v7 = v3 * v4;
    v8 = v5 - v6;
    v9 = v7 / (v8 ? v8 : 1);
    v10 = v6 ^ v7 ^ v8;
    
    v11 = v9 * 3;
    v12 = v10 + 7;
    v13 = v11 - v12;
    v14 = v13 * 2;
    v15 = v14 / 3;
    v16 = v15 | 0xABCD;
    v17 = v16 & 0x1234;
    v18 = v17 << 2;
    v19 = v18 >> 1;
    v20 = v19 + seed;
    
    /* Float variables for mixed-type pressure */
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = f1 + f2;
    f4 = f3 * 2.0f;
    f5 = f4 - 1.0f;
    
    /* Read volatile global to prevent reordering */
    int volatile_read = global_seed;
    
    /* Complex conditional to create different basic blocks */
    if ((seed ^ iter) & 0x3) {
        /* Path 1: High register pressure before call */
        /* Use all variables in computation before call */
        int temp1 = v1 + v2 + v3 + v4 + v5 + volatile_read;
        int temp2 = v6 * v7 - v8 + v9 - v10;
        int temp3 = v11 | v12 ^ v13 & v14;
        
        /* Call with many live variables - forces caller-saves */
        clobber_callee(&v1, &v2, &v3, &v4, &v5);
        
        /* Use results after call - keeps variables live across call */
        v1 = temp1 + v1;
        v2 = temp2 + v2;
        v3 = temp3 + v3;
        
        /* Another call with different register types */
        clobber_callee2(&f1, &f2, &v4);
        
        /* More computations with live variables */
        v5 = v1 * v2 + v3;
        v6 = v4 ^ v5;
        
    } else if ((seed ^ iter) & 0x4) {
        /* Path 2: Different high pressure pattern */
        int temp4 = v15 + v16 + v17 + v18 + v19 + v20;
        float ftemp = f3 + f4 + f5;
        
        clobber_callee(&v15, &v16, &v17, &v18, &v19);
        
        v20 = temp4 + v15 + v16;
        f5 = ftemp + f5;
        
    } else {
        /* Path 3: Simpler path for contrast */
        v1 = v1 + 1;
        v2 = v2 - 1;
    }
    
    /* Switch statement to create more complex control flow */
    switch ((seed + iter) & 0x7) {
        case 0:
            clobber_callee(&v7, &v8, &v9, &v10, &v11);
            v12 = v7 + v8 + v9;
            break;
        case 1:
            clobber_callee2(&f3, &f4, &v13);
            v14 = (int)f3 + v13;
            break;
        case 2:
            /* Nested condition inside switch */
            if (v15 > 100) {
                clobber_callee(&v15, &v16, &v17, NULL, NULL);
            }
            break;
        default:
            /* Loop to create multiple call sites in same block */
            for (int i = 0; i < 3; i++) {
                if (i == 1) {
                    clobber_callee(&v18, &v19, &v20, &v1, &v2);
                }
                v18 += i;
            }
            break;
    }
    
    /* Final computation using all variables to prevent elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Main function with varying conditions */
int main(int argc, char *argv[]) {
    /* Use argc for deterministic but varying behavior */
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    global_seed = seed;
    
    int total = 0;
    
    /* Loop to create multiple high-pressure contexts */
    for (int i = 0; i < 100; i++) {
        /* Vary the condition to exercise different paths */
        int result = high_pressure_function(seed + i, i);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Periodic volatile write to prevent optimizations */
        if (i % 10 == 0) {
            global_seed = i;
        }
    }
    
    /* Use getchar to create another call site with live variables */
    int c = getchar();
    total += c;
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
