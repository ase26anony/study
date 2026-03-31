/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#elif defined(__i386__)
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
#elif defined(__riscv)
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7"
#else
#define CLOBBER_LIST "memory"
#endif

void __attribute__((noinline, noclone)) clobber_callee(int *p) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p) : CLOBBER_LIST);
    *p += 1; /* Ensure the pointer is used */
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *p, int *q) {
    asm volatile("" : : "r"(p), "r"(q) : CLOBBER_LIST);
    *p += *q;
}

/* Function to create complex control flow */
int __attribute__((noinline)) compute_condition(int seed) {
    volatile int v = seed;
    return (v * 1103515245 + 12345) & 0x7fffffff;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed */
    global_seed = argc;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Initialize with complex, non-optimizable computations */
    volatile int init = global_seed;
    
    v1 = init * 1;
    v2 = init * 2 + v1;
    v3 = init * 3 ^ v2;
    v4 = init * 4 | v3;
    v5 = init * 5 & v4;
    v6 = init * 6 + v5;
    v7 = init * 7 - v6;
    v8 = init * 8 * v7;
    v9 = init * 9 / (v8 ? v8 : 1);
    v10 = init * 10 % (v9 ? v9 : 1);
    
    v11 = v1 + v2;
    v12 = v3 + v4;
    v13 = v5 + v6;
    v14 = v7 + v8;
    v15 = v9 + v10;
    v16 = v11 * v12;
    v17 = v13 * v14;
    v18 = v15 * v16;
    v19 = v17 * v18;
    v20 = v19 ^ v18;
    
    /* Float computations */
    f1 = v1 * 0.1f;
    f2 = v2 * 0.2f + f1;
    f3 = v3 * 0.3f + f2;
    f4 = v4 * 0.4f + f3;
    f5 = v5 * 0.5f + f4;
    f6 = v6 * 0.6f + f5;
    f7 = v7 * 0.7f + f6;
    f8 = v8 * 0.8f + f7;
    f9 = v9 * 0.9f + f8;
    f10 = v10 * 1.0f + f9;
    
    int result = 0;
    
    /* Complex control flow with multiple paths */
    for (int i = 0; i < 10; i++) {
        int cond = compute_condition(global_seed + i);
        
        if (cond & 0x100) {
            /* Path 1: High register pressure before call */
            /* Use many variables in computation before call */
            int t1 = v1 + v2 + v3 + v4 + v5;
            int t2 = v6 + v7 + v8 + v9 + v10;
            float ft1 = f1 + f2 + f3 + f4 + f5;
            float ft2 = f6 + f7 + f8 + f9 + f10;
            
            /* Call that clobbers registers - variables are live across call */
            clobber_callee(&v1);
            
            /* Use results after call */
            t1 += v1;
            t2 += v2;
            ft1 += f1;
            ft2 += f2;
            
            result += t1 + t2 + (int)ft1 + (int)ft2;
        } 
        else if (cond & 0x80) {
            /* Path 2: Different high pressure pattern */
            int t3 = v11 * v12 - v13;
            int t4 = v14 / (v15 ? v15 : 1) + v16;
            float ft3 = f3 * f4 - f5;
            float ft4 = f6 / (f7 ? f7 : 0.1f) + f8;
            
            /* Call with different signature */
            clobber_callee2(&f3, &v11);
            
            t3 += v11;
            t4 += v12;
            ft3 += f3;
            ft4 += f4;
            
            result += t3 + t4 + (int)ft3 + (int)ft4;
        }
        else if (cond & 0x40) {
            /* Path 3: Even more pressure with multiple calls */
            int t5 = v17 ^ v18 | v19;
            int t6 = v20 & v19 | v18;
            float ft5 = f9 * 2.0f - f10;
            
            /* Multiple calls in same basic block */
            clobber_callee(&v17);
            clobber_callee(&v18);
            
            t5 += v17;
            t6 += v18;
            ft5 += f9;
            
            result += t5 + t6 + (int)ft5;
        }
        else {
            /* Path 4: Low pressure path for contrast */
            result += v1 + v2;
        }
        
        /* Modify some variables to create data dependencies between iterations */
        v1 += result;
        v2 ^= result;
        f1 += result * 0.01f;
        
        /* Another volatile operation to prevent reordering */
        global_seed = result;
    }
    
    /* Use all variables in final computation to keep them live */
    int final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    
    printf("Result: %d (final: %d)\n", result, final);
    
    return final != 0;
}
