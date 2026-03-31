/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_seed;

/* Function that clobbers many registers - prevents inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    #else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Do some actual work to prevent optimization */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) 
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    if (f1) *f1 += 1.5f;
    if (f2) *f2 += 2.5f;
    if (i1) *i1 += 5;
}

/* Function to create complex control flow */
__attribute__((noinline))
int complex_control(int seed, int mode) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with arithmetic to create data dependencies */
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed ^ 0x1234;
    v4 = seed - 100;
    v5 = (seed << 3) | (seed >> 5);
    
    /* Use volatile read to prevent optimization */
    v6 = global_seed;
    v7 = v6 * v1;
    v8 = v2 + v3;
    v9 = v4 ^ v5;
    v10 = v7 - v8;
    
    /* More computations creating web of dependencies */
    v11 = v9 * v10;
    v12 = v1 + v2 + v3 + v4 + v5;
    v13 = v6 ^ v7 ^ v8;
    v14 = v9 + v10 + v11;
    v15 = v12 * v13;
    
    /* Float variables for mixed-type pressure */
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = f1 + f2;
    f4 = (float)v3 * 2.5f;
    f5 = f3 * f4;
    
    v16 = (int)f5 + v14;
    v17 = v15 - v16;
    v18 = v11 * v17;
    v19 = v13 + v18;
    v20 = v19 ^ seed;
    
    /* Complex conditional creating basic block boundaries */
    if (mode == 0) {
        /* High register pressure path with function call at block end */
        /* All variables are live across this call */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* The call above should be at the end of its basic block */
        /* After call, continue using the variables */
        v5 = v1 + v2;
        v6 = v3 * v4;
    } 
    else if (mode == 1) {
        /* Another high pressure path with different call */
        clobber_callee2(&f1, &f2, &v1);
        
        v7 = (int)f1 + v2;
        v8 = v3 ^ (int)f2;
    }
    else if (mode == 2) {
        /* Path with multiple calls creating multiple save/restore sites */
        clobber_callee(&v5, &v6, &v7, &v8);
        /* Some computation between calls */
        v9 = v5 + v6 + v7 + v8;
        clobber_callee2(&f3, &f4, &v9);
    }
    else {
        /* Low pressure path - no function calls */
        v10 = v1 * v2 * v3;
    }
    
    /* Use all variables after conditional to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Function with loop creating multiple call sites */
__attribute__((noinline))
int loop_with_calls(int iterations) {
    int total = 0;
    volatile int loop_counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Read volatile to prevent loop optimizations */
        loop_counter = i;
        
        /* Create many live variables in the loop */
        int a = i * 3;
        int b = i + 100;
        int c = i ^ 0xABCD;
        int d = i << 2;
        int e = i - 50;
        int f = a * b;
        int g = c + d;
        int h = e ^ f;
        int j = g * h;
        int k = a + b + c + d + e;
        
        /* Conditional inside loop with call at block end */
        if (i % 3 == 0) {
            clobber_callee(&a, &b, &c, &d);
            total += a + b;
        } 
        else if (i % 3 == 1) {
            float f1 = (float)a * 0.1f;
            float f2 = (float)b * 0.2f;
            clobber_callee2(&f1, &f2, &e);
            total += c + d + e;
        }
        else {
            /* No call, but still use variables */
            total += f + g + h + j + k;
        }
        
        /* Use variables after conditional */
        total += a + b + c + d + e + f + g + h + j + k;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    global_seed = argc;
    
    /* Get some input-dependent but deterministic value */
    int seed = argc * 12345;
    
    /* Test different modes to exercise different paths */
    int results[4];
    
    for (int mode = 0; mode < 4; mode++) {
        results[mode] = complex_control(seed + mode, mode);
    }
    
    /* Test loop with multiple iterations */
    int loop_result = loop_with_calls(10);
    
    /* Combine all results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 4; i++) {
        final_result ^= results[i];
    }
    final_result += loop_result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
