/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that must stay live across clobbering asm statements */
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    /* MMX type (8-byte vector) */
    typedef long long v1di __attribute__((vector_size(8)));
    v1di mmx_val = {vi * 2LL};
    
    int result = 0;
    int i;
    
    /* Loop to prevent hoisting of save/restore code */
    for (i = 0; i < iterations; i++) {
        /* ====== INTEGER REGISTER CLOBBERING ====== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = vi * 3 + i;
        int b = vi * 5 + i * 2;
        int c = vi * 7 + i * 3;
        int d = vi * 11 + i * 4;
        
        /* Force values to be in registers, then clobber them */
        asm volatile (
            "/* Clobber integer regs */\n\t"
            "mov %0, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "mov %2, %%ecx\n\t"
            "mov %3, %%edx\n\t"
            "add $1, %%eax\n\t"
            "add $2, %%ebx\n\t"
            : /* no outputs */
            : "r" (a), "r" (b), "r" (c), "r" (d)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobbering - forces save/restore */
        result += a + b + c + d;
        
        /* ====== SSE REGISTER CLOBBERING ====== */
        /* Use xmm0-xmm3 (call-clobbered) */
        v4sf f1 = vec_float + (float)i;
        v4sf f2 = vec_float * 2.0f + (float)i;
        
        asm volatile (
            "/* Clobber SSE regs */\n\t"
            "movaps %0, %%xmm0\n\t"
            "movaps %1, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            : /* no outputs */
            : "x" (f1), "x" (f2)
            : "xmm0", "xmm1", "memory"
        );
        
        /* Use vector values after clobbering */
        vec_float = f1 + f2;
        result += (int)vec_float[0];
        
        /* ====== MIXED REGISTER CLOBBERING WITH CALL ====== */
        /* Create a situation where a basic block ends with call-like asm */
        int temp = vi * 13 + i;
        
        /* This asm simulates a function call that clobbers multiple regs */
        asm volatile (
            "/* Simulate call that clobbers many regs */\n\t"
            "mov %0, %%r8d\n\t"
            "add $100, %%r8d\n\t"
            : /* no outputs */
            : "r" (temp)
            : "r8", "r9", "r10", "r11", "rax", "rcx", "rdx", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
              "xmm6", "xmm7", "mm0", "mm1", "memory"
        );
        
        /* CRITICAL: Place a label/jump target right after the clobbering asm */
        if (temp > 1000) {
            /* This creates a control flow edge, making the asm potentially
               the end of a basic block */
            goto skip_point;  /* Forward jump */
        }
        
        /* Use the value after clobbering */
        result += temp;
        
        /* ====== MMX REGISTER CLOBBERING ====== */
        /* Use MMX registers (call-clobbered) */
        v1di mmx_tmp = mmx_val + (long long)i;
        
        asm volatile (
            "/* Clobber MMX regs */\n\t"
            "movq %0, %%mm0\n\t"
            "paddq %0, %%mm0\n\t"
            : /* no outputs */
            : "x" (mmx_tmp)
            : "mm0", "mm1", "memory"
        );
        
        mmx_val = mmx_tmp;
        result += (int)mmx_val[0];
        
        /* Label for the forward jump */
        skip_point:
        vi++;  /* Modify volatile to prevent loop optimizations */
    }
    
    /* Final computation using all values */
    result += (int)vec_int[0] + (int)vec_float[0] + (int)mmx_val[0];
    return result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x) {
    volatile int y = x;
    asm volatile ("" : "+r" (y));
    return y * 2;
}

/* Main function that creates pressure on caller-save */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    int i;
    
    /* Call test function multiple times with different seeds */
    for (i = 0; i < 10; i++) {
        int seed = i * 100 + argc;
        
        /* Call helper function between test_caller_save calls */
        /* This creates additional register pressure */
        int helper_result = helper_func(seed);
        
        /* Main call that should trigger caller-save insertions */
        int result = test_caller_save(iterations, seed + helper_result);
        
        total += result;
        
        /* Another helper call to split basic blocks */
        helper_result = helper_func(result % 100);
        total += helper_result;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional complexity: conditional with function calls */
    if (total > 1000) {
        total += helper_func(total);
    } else {
        total -= helper_func(total);
    }
    
    printf("Final: %d\n", total);
    return total != 0 ? 0 : 1;
}
