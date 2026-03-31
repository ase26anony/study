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
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Callee that clobbers many registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber registers and memory */
    asm volatile ("" 
                  : /* no outputs */ 
                  : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
                  : CLOBBER_LIST);
    
    /* Force memory side effect */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another callee with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1, int *i2) {
    asm volatile (""
                  : /* no outputs */
                  : "r"(f1), "r"(f2), "r"(i1), "r"(i2)
                  : CLOBBER_LIST);
    
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
    if (i1) *i1 += 3;
    if (i2) *i2 += 4;
}

/* Function with high register pressure around calls */
int __attribute__((noinline, optimize("O3")))
high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-removable computations */
    v1 = seed + iter;
    v2 = v1 * 3;
    v3 = v2 - seed;
    v4 = v3 ^ iter;
    v5 = v4 * 7;
    v6 = v5 + global_seed;  /* Use volatile global */
    v7 = v6 / 2;
    v8 = v7 | 0xFF;
    v9 = v8 << 3;
    v10 = v9 - iter;
    
    v11 = v10 * 11;
    v12 = v11 + seed;
    v13 = v12 ^ v1;
    v14 = v13 * 13;
    v15 = v14 - iter;
    v16 = v15 & 0xFFFF;
    v17 = v16 >> 2;
    v18 = v17 + global_seed;  /* Another volatile use */
    v19 = v18 * 17;
    v20 = v19 ^ seed;
    
    /* Float variables for mixed-type pressure */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 2.2f;
    f3 = (float)v3 * 3.3f;
    f4 = (float)v4 * 4.4f;
    f5 = (float)v5 * 5.5f;
    
    /* Complex conditional to create different basic blocks */
    int result = 0;
    
    /* First conditional path - high pressure with call at block end */
    if ((seed ^ iter) & 0x1) {
        /* Use most variables before call to keep them live */
        int t1 = v1 + v2 + v3 + v4 + v5;
        int t2 = v6 + v7 + v8 + v9 + v10;
        int t3 = v11 + v12 + v13 + v14 + v15;
        int t4 = v16 + v17 + v18 + v19 + v20;
        
        /* Call with many live variables - creates caller-save pressure */
        clobber_callee(&t1, &t2, &t3, &t4);
        
        /* Use results after call */
        result = t1 + t2 + t3 + t4;
        
        /* More computations to use float variables */
        f1 += (float)t1;
        f2 += (float)t2;
        f3 += (float)t3;
        f4 += (float)t4;
        
        /* Another call with mixed types */
        clobber_callee2(&f1, &f2, &v1, &v2);
        
        result += (int)f1 + (int)f2 + v1 + v2;
        
        /* BB_END should be the call instruction before save insertion */
    } 
    else {
        /* Alternative path with less pressure */
        result = v1 + v3 + v5 + v7 + v9;
    }
    
    /* Second conditional with different pressure pattern */
    if ((seed ^ iter) & 0x2) {
        /* Create another high-pressure scenario */
        int s1 = v20 * v19;
        int s2 = v18 * v17;
        int s3 = v16 * v15;
        int s4 = v14 * v13;
        
        /* Call in the middle of computations */
        clobber_callee(&s1, &s2, &s3, &s4);
        
        /* Continue using results */
        result += s1 - s2 + s3 - s4;
        
        /* Nested condition to create more complex CFG */
        if (s1 > s2) {
            clobber_callee2(&f3, &f4, &s1, &s2);
            result += (int)(f3 * f4);
        }
    }
    
    /* Use all variables in final computation to keep them live */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)(f1 + f2 + f3 + f4 + f5);
    
    return result;
}

/* Function with loop creating multiple call sites */
int __attribute__((noinline))
multi_call_site_function(int seed, int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying conditions create different basic block structures */
        if ((i % 3) == 0) {
            /* Path with call at potential block end */
            int a = seed + i;
            int b = a * 2;
            int c = b - i;
            int d = c ^ seed;
            
            clobber_callee(&a, &b, &c, &d);
            
            total += a + b + c + d;
        } 
        else if ((i % 3) == 1) {
            /* Different register pressure pattern */
            int x1 = seed * i;
            int x2 = x1 + 1;
            int x3 = x2 * 3;
            int x4 = x3 - 5;
            int x5 = x4 ^ 0xFF;
            int x6 = x5 << 2;
            
            /* Multiple calls in sequence */
            clobber_callee(&x1, &x2, &x3, &x4);
            clobber_callee(&x5, &x6, &x1, &x2);
            
            total += x1 + x2 + x3 + x4 + x5 + x6;
        }
        else {
            /* Path without calls */
            total += seed * i;
        }
        
        /* Mix in some volatile operations */
        total += global_seed;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    printf("Seed: %d\n", seed);
    
    /* Test high pressure function */
    int result1 = high_pressure_function(seed, 5);
    printf("High pressure result: %d\n", result1);
    
    /* Test multi-call-site function */
    int result2 = multi_call_site_function(seed, 10);
    printf("Multi-call result: %d\n", result2);
    
    /* Combine results */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;
}
