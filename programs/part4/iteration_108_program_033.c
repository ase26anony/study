/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to ensure a call site */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Function that appears to clobber many registers */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to convince GCC we clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    
    /* Modify through pointers to create side effects */
    if (p1) *p1 ^= 0x1234;
    if (p2) *p2 ^= 0x5678;
    if (p3) *p3 ^= 0x9ABC;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *f1, float *f2) {
    asm volatile("" : : "r"(f1), "r"(f2) : CLOBBER_LIST);
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    v1 = seed ^ 0x1111;
    v2 = seed * 3 + 1;
    v3 = seed / 2;
    v4 = seed << 3;
    v5 = seed | 0xAAAA;
    v6 = seed & 0x5555;
    v7 = global_seed++;  /* Volatile access */
    v8 = v1 + v2;
    v9 = v3 * v4;
    v10 = v5 ^ v6;
    
    /* More computations creating data dependencies */
    v11 = v7 + v8 * 2;
    v12 = v9 - v10;
    v13 = (v11 << 1) | (v12 >> 1);
    v14 = v13 * 3 + 7;
    v15 = v14 / 2;
    v16 = v15 ^ 0xDEAD;
    v17 = v16 + 0xBEEF;
    v18 = v17 * v13;
    v19 = v18 - v14;
    v20 = v19 & 0xFFFF;
    
    /* Float computations to use FP registers */
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = f1 + f2;
    f4 = f1 * f2;
    f5 = f3 / (f4 + 1.0f);
    
    /* Complex conditional to create basic block boundaries */
    if (seed & 1) {
        /* Path 1: High register pressure before call */
        int t1 = v1 + v2 + v3 + v4 + v5;
        int t2 = v6 + v7 + v8 + v9 + v10;
        int t3 = v11 + v12 + v13 + v14 + v15;
        int t4 = v16 + v17 + v18 + v19 + v20;
        
        /* All these variables are live across the call */
        clobber_callee(&t1, &t2, &t3);
        
        /* Use results after call */
        v1 = t1 ^ t2;
        v2 = t3 + t4;
        f1 = (float)(t1 + t2) * 0.25f;
        
        /* Another call with float registers live */
        clobber_callee2(&f1, &f2);
        
        /* More computations keeping variables live */
        v3 = (int)f1 * v1;
        v4 = (int)f2 * v2;
    } else if (seed & 2) {
        /* Path 2: Different high pressure pattern */
        int s1 = v20 - v19;
        int s2 = v18 - v17;
        
        /* Call at end of this basic block */
        clobber_callee(&s1, &s2, &v16);
        
        /* Variables remain live */
        v5 = s1 * s2 + v16;
        v6 = s1 ^ s2 ^ v15;
        
        /* Nested condition creating more blocks */
        if (v5 > v6) {
            clobber_callee2(&f3, &f4);
            v7 = (int)(f3 * 100.0f);
            v8 = (int)(f4 * 200.0f);
        }
    } else {
        /* Path 3: Simpler path for contrast */
        v9 = v1 + v2;
        v10 = v3 + v4;
    }
    
    /* Loop to create multiple call sites in same function */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        int loop_var1 = v1 + i;
        int loop_var2 = v2 + i * 2;
        int loop_var3 = v3 + i * 3;
        
        /* Call inside loop with loop variables live */
        clobber_callee(&loop_var1, &loop_var2, &loop_var3);
        
        /* Use results */
        sum += loop_var1 + loop_var2 + loop_var3;
        
        /* Alternate between call patterns */
        if (i & 1) {
            float f_loop = (float)loop_var1 * 0.5f;
            clobber_callee2(&f_loop, &f5);
            sum += (int)f_loop;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + sum;
    
    return result;
}

/* Another function with different register pressure pattern */
int __attribute__((noinline)) another_high_pressure(int base) {
    /* Even more variables */
    int a1 = base, a2 = base * 2, a3 = base * 3, a4 = base * 4;
    int a5 = base * 5, a6 = base * 6, a7 = base * 7, a8 = base * 8;
    int a9 = base * 9, a10 = base * 10;
    
    volatile int vol = global_seed;  /* Force memory access */
    
    /* Switch statement to create multiple basic blocks with calls at ends */
    switch (base % 4) {
        case 0:
            a1 = vol + a2;
            a3 = a4 * a5;
            /* Call at potential block end */
            clobber_callee(&a1, &a3, &a6);
            a7 = a1 + a3;
            break;
            
        case 1:
            a2 = vol - a1;
            a4 = a3 / 2;
            clobber_callee(&a2, &a4, &a8);
            a9 = a2 * a4;
            /* Another call in same block */
            clobber_callee2((float*)&a9, (float*)&a10);
            break;
            
        case 2:
            a5 = a6 ^ a7;
            clobber_callee(&a5, &a8, &a9);
            /* Fall through */
            
        default:
            a10 = vol | 0xFF;
            clobber_callee(&a10, &a1, &a2);
            break;
    }
    
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

int main(int argc, char **argv) {
    /* Deterministic but input-dependent seed */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    printf("Seed: %d\n", seed);
    
    /* Call high-pressure functions multiple times */
    int result1 = high_pressure_function(seed);
    int result2 = another_high_pressure(seed + 1);
    int result3 = high_pressure_function(seed + 2);
    
    /* Use results to prevent elimination */
    int final = result1 + result2 + result3;
    printf("Result: %d (0x%08x)\n", final, final);
    
    /* Loop with varying conditions to create more caller-save opportunities */
    for (int i = 0; i < 5; i++) {
        int loop_seed = seed + i * 100;
        int temp = high_pressure_function(loop_seed);
        final ^= temp;
        
        /* Call within loop with intermediate results live */
        clobber_callee(&final, &temp, &i);
    }
    
    printf("Final: %d\n", final);
    return final & 0xFF;  /* Non-zero exit code based on computation */
}
