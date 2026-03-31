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
    /* Inline asm to clobber specific x86 registers */
    asm volatile("" 
                 : 
                 : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
                 : "memory", "eax", "ecx", "edx", "esi", "edi", "ebx");
    
    /* Opaque memory operation */
    if (p1) *p1 ^= 0x1234;
    if (p2) *p2 ^= 0x5678;
    if (p3) *p3 ^= 0x9abc;
    if (p4) *p4 ^= 0xdef0;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    /* Clobber floating point registers too */
    asm volatile("" 
                 : 
                 : "r"(f1), "r"(f2)
                 : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    if (f1) *f1 = *f1 * 2.0f;
    if (f2) *f2 = *f2 / 2.0f;
}

/* Function with high register pressure around calls */
__attribute__((noinline))
int high_pressure_function(int seed, int iter) {
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6;
    
    /* Initialize with complex, non-optimizable computations */
    volatile int vol = global_seed + iter;
    
    v1 = seed * 1 + vol;
    v2 = seed * 2 + vol;
    v3 = seed * 3 + vol;
    v4 = seed * 4 + vol;
    v5 = seed * 5 + vol;
    v6 = seed * 6 + vol;
    v7 = seed * 7 + vol;
    v8 = seed * 8 + vol;
    v9 = seed * 9 + vol;
    v10 = seed * 10 + vol;
    
    v11 = v1 ^ v2;
    v12 = v3 ^ v4;
    v13 = v5 ^ v6;
    v14 = v7 ^ v8;
    v15 = v9 ^ v10;
    
    v16 = v11 + v12;
    v17 = v13 + v14;
    v18 = v15 + v16;
    v19 = v17 + v18;
    v20 = v19 * seed;
    
    /* Floating point variables also need registers */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = (float)v5 * 1.5f;
    f6 = (float)v6 * 1.6f;
    
    /* Complex conditional to create different basic blocks */
    int result;
    if ((seed & 3) == 0) {
        /* PATH 1: High register pressure path with call at block end */
        /* Many variables are live across this call */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* Use results after call - keeps variables live */
        v5 = v1 + v2 + v3 + v4;
        v6 = v5 * 2;
        
        /* Another call with different register pressure */
        clobber_callee2(&f1, &f2);
        
        /* More computations keeping variables live */
        v7 = (int)f1 + (int)f2 + v6;
        v8 = v7 ^ v20;
        
        result = v8;
    } 
    else if ((seed & 3) == 1) {
        /* PATH 2: Different pressure pattern */
        clobber_callee(&v5, &v6, &v7, &v8);
        
        /* Mix integer and float */
        clobber_callee2(&f3, &f4);
        
        v9 = v5 + v6 + v7 + v8 + (int)f3 + (int)f4;
        result = v9 ^ v19;
    }
    else if ((seed & 3) == 2) {
        /* PATH 3: Nested condition with call at end of inner block */
        if (v10 > 100) {
            clobber_callee(&v11, &v12, &v13, &v14);
            v15 = v11 + v12 + v13 + v14;
        } else {
            v15 = v10 * 2;
        }
        
        /* Call at what might be block end */
        clobber_callee2(&f5, &f6);
        
        result = v15 + (int)f5 + (int)f6;
    }
    else {
        /* PATH 4: Loop with call at end of loop body */
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            /* Variables live across loop iteration */
            v16 += i;
            v17 ^= i;
            
            /* Call at potential block end */
            if (i == 1) {
                clobber_callee(&v16, &v17, &v18, &v19);
            }
            
            sum += v16 + v17;
        }
        result = sum;
    }
    
    /* Final computation using ALL variables to keep them live */
    /* This creates maximum register pressure */
    int final = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6;
    
    return result + (final & 0xFF);
}

/* Main driver with multiple call sites */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    int total = 0;
    
    /* Multiple iterations with different conditions */
    for (int i = 0; i < 100; i++) {
        int iter_seed = seed + i * 37;
        
        /* Call high-pressure function multiple times */
        int r1 = high_pressure_function(iter_seed, i);
        int r2 = high_pressure_function(iter_seed ^ 0x5555, i + 1);
        int r3 = high_pressure_function(iter_seed ^ 0xAAAA, i + 2);
        
        /* Use results to prevent elimination */
        total += r1 + r2 + r3;
        
        /* Volatile memory op to prevent reordering */
        global_seed ^= r1;
    }
    
    printf("Result: %d\n", total);
    return total & 1;
}
