/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to prevent optimization */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Callee that clobbers many registers */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : CLOBBER_LIST);
    
    /* Modify memory to create side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another callee with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *f1, float *f2, int *i1) {
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : CLOBBER_LIST);
    if (f1) *f1 += 1.5f;
    if (f2) *f2 += 2.5f;
    if (i1) *i1 += 5;
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-removable computations */
    v1 = seed + iter;
    v2 = v1 * 3;
    v3 = v2 - seed;
    v4 = v3 ^ iter;
    v5 = v4 * v1;
    v6 = v5 / (seed + 1);
    v7 = v6 << 2;
    v8 = v7 | iter;
    v9 = v8 - v2;
    v10 = v9 * v3;
    
    v11 = v10 + global_seed;  /* Use volatile global */
    v12 = v11 * 2;
    v13 = v12 ^ v4;
    v14 = v13 + v5;
    v15 = v14 - v6;
    v16 = v15 * v7;
    v17 = v16 | v8;
    v18 = v17 ^ v9;
    v19 = v18 + v10;
    v20 = v19 * v11;
    
    /* Floating point variables to use FP registers */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 2.2f;
    f3 = (float)v3 * 3.3f;
    f4 = (float)v4 * 4.4f;
    f5 = (float)v5 * 5.5f;
    
    /* Complex conditional to create different basic blocks */
    int result = 0;
    
    /* First conditional path - high pressure with call at block end */
    if ((seed ^ iter) & 0x1) {
        /* More computations to keep variables live */
        v1 = v20 + v19;
        v2 = v19 + v18;
        v3 = v18 + v17;
        v4 = v17 + v16;
        
        f1 = f5 * 2.0f;
        f2 = f4 * 3.0f;
        
        /* Call that clobbers registers - positioned to be at block end */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* BB_END was the call, now save/restore should be inserted after */
        result = v1 + v2 + v3 + v4;
    } 
    else if ((seed ^ iter) & 0x2) {
        /* Alternative path with different call pattern */
        v5 = v16 + v15;
        v6 = v15 + v14;
        
        f3 = f2 * 4.0f;
        f4 = f1 * 5.0f;
        
        clobber_callee2(&f3, &f4, &v5);
        
        result = v5 + v6 + (int)f3 + (int)f4;
    }
    else {
        /* Path without call but with computations */
        v7 = v14 + v13;
        v8 = v13 + v12;
        result = v7 * v8;
    }
    
    /* Use all variables after conditional to keep them live across calls */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result + (checksum & 0xFF);
}

/* Function with loop creating multiple call sites */
int __attribute__((noinline)) multi_call_site_function(int seed) {
    int total = 0;
    
    /* Loop creates multiple basic blocks with calls */
    for (int i = 0; i < 10; i++) {
        /* Varying conditions create different block structures */
        if (i & 1) {
            /* Block with call at end */
            int a = seed + i;
            int b = a * 3;
            int c = b - i;
            int d = c ^ seed;
            
            /* Call with many live variables */
            clobber_callee(&a, &b, &c, &d);
            
            total += a + b + c + d;
        } else {
            /* Block ending with different call */
            float x = (float)(seed * i) * 1.5f;
            float y = (float)(seed + i) * 2.5f;
            int z = seed ^ i;
            
            clobber_callee2(&x, &y, &z);
            
            total += (int)x + (int)y + z;
        }
        
        /* Additional computation between iterations */
        volatile int barrier = global_seed;  /* Memory barrier */
        total += barrier & 1;
    }
    
    return total;
}

/* Main function with varying conditions */
int main(int argc, char *argv[]) {
    /* Deterministic but input-dependent seed */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    /* Call high-pressure function multiple times */
    int result1 = high_pressure_function(seed, 0);
    int result2 = high_pressure_function(seed + 1, 1);
    int result3 = high_pressure_function(seed + 2, 2);
    
    /* Function with multiple call sites */
    int result4 = multi_call_site_function(seed);
    
    /* Use results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Print to create side effect */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    return final_result & 0xFF;
}
