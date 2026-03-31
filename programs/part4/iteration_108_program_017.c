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
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific x86 registers */
    asm volatile("" 
                 : 
                 : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    
    /* Force memory side effect */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    /* Clobber floating point registers too */
    asm volatile("" 
                 : 
                 : "r"(f1), "r"(f2)
                 : "memory", "st(0)", "st(1)", "st(2)", "st(3)", 
                   "st(4)", "st(5)", "st(6)", "st(7)");
    
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 1;
    v2 = seed * 2 + global_seed;
    v3 = seed * 3 - global_seed;
    v4 = seed * 4 ^ global_seed;
    v5 = seed * 5 | global_seed;
    v6 = seed * 6 & global_seed;
    v7 = seed * 7 + v1;
    v8 = seed * 8 + v2;
    v9 = seed * 9 + v3;
    v10 = seed * 10 + v4;
    
    v11 = v1 * v2 - v3;
    v12 = v4 * v5 - v6;
    v13 = v7 * v8 - v9;
    v14 = v10 * v1 - v2;
    v15 = v3 * v4 - v5;
    v16 = v6 * v7 - v8;
    v17 = v9 * v10 - v1;
    v18 = v2 * v3 - v4;
    v19 = v5 * v6 - v7;
    v20 = v8 * v9 - v10;
    
    /* Floating point variables for FP register pressure */
    f1 = (float)v1 / 3.14159f;
    f2 = (float)v2 / 2.71828f;
    f3 = f1 * f2 + (float)seed;
    f4 = f2 * f3 - (float)global_seed;
    f5 = f3 * f4 / (float)(seed + 1);
    
    /* Read volatile global again to prevent reordering */
    int temp = global_seed;
    
    /* Complex conditional that creates separate basic blocks */
    if (temp & 0x1) {
        /* Path 1: Call clobbering function with many live variables */
        /* All these variables are live across the call */
        clobber_callee(&v11, &v12, &v13, &v14);
        
        /* More computations to keep variables live */
        v15 = v11 + v12;
        v16 = v13 + v14;
        
        /* Another call with different arguments */
        clobber_callee(&v15, &v16, &v17, &v18);
        
        /* Call floating point clobberer */
        clobber_callee2(&f1, &f2);
        
        /* More live variables across another call */
        f3 = f1 + f2;
        clobber_callee2(&f3, &f4);
    } 
    else if (temp & 0x2) {
        /* Path 2: Different call pattern */
        clobber_callee(&v19, &v20, &v1, &v2);
        clobber_callee2(&f5, &f1);
    }
    else {
        /* Path 3: No calls, simpler computation */
        v11 = v1 + v2;
        v12 = v3 + v4;
    }
    
    /* Use all variables in final computation to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Function with loop creating multiple call sites */
int loop_with_calls(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many live variables in the loop */
        int a = i * 1;
        int b = i * 2 + global_seed;
        int c = i * 3 - global_seed;
        int d = i * 4 ^ global_seed;
        int e = i * 5 | global_seed;
        int f = i * 6 & global_seed;
        int g = i * 7 + a;
        int h = i * 8 + b;
        
        /* Conditional inside loop creates different basic blocks */
        if ((i + global_seed) % 3 == 0) {
            /* Call at what might become BB_END */
            clobber_callee(&a, &b, &c, &d);
            
            /* More live variables */
            e = a + b;
            f = c + d;
            
            /* Another call */
            clobber_callee(&e, &f, &g, &h);
        } 
        else if ((i + global_seed) % 3 == 1) {
            /* Different call site */
            clobber_callee(&g, &h, &a, &b);
        }
        
        /* Use variables after conditional */
        total += a + b + c + d + e + f + g + h;
        
        /* Volatile read to prevent loop optimizations */
        asm volatile("" ::: "memory");
    }
    
    return total;
}

/* Main function with multiple high-pressure paths */
int main(int argc, char *argv[]) {
    /* Use argc as seed for deterministic but varying behavior */
    int seed = argc;
    
    /* Force seed to be in memory */
    volatile int vol_seed = seed;
    
    printf("Starting caller-save test with seed %d\n", vol_seed);
    
    /* Call high-pressure function multiple times */
    int result1 = high_pressure_path(vol_seed);
    printf("Result 1: %d\n", result1);
    
    /* Call loop function */
    int result2 = loop_with_calls(10);
    printf("Result 2: %d\n", result2);
    
    /* Another test with different conditions */
    global_seed = vol_seed * 17 + 12345;
    
    int result3 = high_pressure_path(vol_seed * 3);
    printf("Result 3: %d\n", result3);
    
    /* Final checksum to use all results */
    int final = result1 + result2 + result3;
    printf("Final checksum: %d\n", final);
    
    return final != 0 ? 0 : 1;
}
