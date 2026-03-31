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
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Callee that clobbers registers */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber registers and create a memory barrier */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : CLOBBER_LIST);
    
    /* Modify through pointers to create side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another callee with different signature to create varied call sites */
void __attribute__((noinline, noclone)) clobber_callee2(int *p1, int *p2, int *p3, int *p4, int *p5) {
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : CLOBBER_LIST);
    
    if (p1) *p1 ^= 0x55;
    if (p2) *p2 ^= 0xAA;
    if (p3) *p3 ^= 0xFF;
    if (p4) *p4 ^= 0x33;
    if (p5) *p5 ^= 0xCC;
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent constant propagation */
    v1 = seed + 1;
    v2 = seed * 2 + global_seed;
    v3 = seed ^ 0x1234;
    v4 = seed - global_seed;
    v5 = (seed << 3) | (seed >> 5);
    v6 = seed * seed;
    v7 = v1 + v2 + v3;
    v8 = v4 * v5 - v6;
    v9 = (v7 ^ v8) & 0xFF;
    v10 = v9 * 3 + 7;
    
    v11 = v10 + seed;
    v12 = v11 * 2;
    v13 = v12 / 3;
    v14 = v13 | v11;
    v15 = v14 & v12;
    v16 = v15 ^ v13;
    v17 = v16 + v14;
    v18 = v17 * 3 - v16;
    v19 = v18 / 2 + v17;
    v20 = v19 ^ v18;
    
    v21 = v20 + seed;
    v22 = v21 * 5;
    v23 = v22 + 12345;
    v24 = v23 - 6789;
    v25 = v24 * v23;
    v26 = v25 / 7;
    v27 = v26 | v25;
    v28 = v27 & v26;
    v29 = v28 ^ v27;
    v30 = v29 + v28;
    
    /* Use volatile read to prevent reordering */
    volatile int vol = global_seed;
    v1 += vol;
    v2 -= vol;
    
    /* Complex conditional to create basic block boundaries */
    if (seed & 1) {
        /* High pressure path - many variables live across call */
        int temp1 = v1 + v2 + v3 + v4 + v5;
        int temp2 = v6 + v7 + v8 + v9 + v10;
        
        /* Call with many live variables - will need caller-save */
        clobber_callee(&v11, &v12, &v13, &v14);
        
        /* Use results after call */
        v15 = v11 + v12 + v13 + v14 + temp1;
        v16 = v15 * 2 - temp2;
        
        /* Another call with different live set */
        clobber_callee2(&v17, &v18, &v19, &v20, &v21);
        
        v22 = v17 ^ v18 ^ v19 ^ v20 ^ v21;
    } else {
        /* Lower pressure path */
        v11 = v1 + v2;
        v12 = v3 + v4;
        v13 = v5 + v6;
    }
    
    /* More computations to keep variables live */
    int sum1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    int sum2 = v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    int sum3 = v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    /* Another conditional with call at block end */
    if (seed & 2) {
        /* Call at the end of a basic block */
        clobber_callee(&sum1, &sum2, &sum3, &v30);
        
        /* This makes the call instruction potentially be BB_END */
        return sum1 + sum2 + sum3 + v30;
    } else {
        return sum1 * 2 + sum2 * 3 + sum3 * 4;
    }
}

/* Function with loop creating multiple call sites */
int __attribute__((noinline)) loop_with_calls(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables in loop */
        int a = i * 3 + 1;
        int b = i * 5 + 2;
        int c = i * 7 + 3;
        int d = i * 11 + 4;
        int e = i * 13 + 5;
        int f = i * 17 + 6;
        int g = i * 19 + 7;
        int h = i * 23 + 8;
        
        /* Complex computation */
        int x = a * b - c * d + e * f - g * h;
        int y = (a ^ b) | (c & d) ^ (e | f) & (g ^ h);
        
        /* Conditional with call at different positions */
        if (i & 1) {
            /* Call in the middle */
            clobber_callee(&a, &b, &x, &y);
            
            total += a + b + x + y + i;
            
            /* Another call later */
            clobber_callee2(&c, &d, &e, &f, &g);
            
            total += c + d + e + f + g;
        } else {
            /* Different pattern */
            clobber_callee2(&x, &y, &a, &b, &c);
            total += x + y + a + b + c + h;
        }
        
        /* Mix in global volatile */
        total += global_seed & 0xFF;
    }
    
    return total;
}

int main(int argc, char **argv) {
    /* Deterministic but input-dependent seed */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    /* Create register pressure in main as well */
    int result1 = high_pressure_function(seed);
    int result2 = loop_with_calls(seed % 10 + 5);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Final computation using both results */
    int final = result1 ^ result2;
    final += high_pressure_function(final);
    
    printf("Final: %d\n", final);
    
    return final & 0xFF;
}
