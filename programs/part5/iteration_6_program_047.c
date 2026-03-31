/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types that are call-clobbered on x86-64 */
    volatile int vi1 = seed;          /* Force memory traffic */
    volatile int vi2 = seed * 2;
    volatile double vd1 = seed * 0.5;
    volatile double vd2 = seed * 1.5;
    
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*0.1f, seed*0.2f, seed*0.3f, seed*0.4f};
    
    int result = 0;
    
    /* Loop with runtime iteration count to prevent hoisting */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTER SECTION ========== */
        /* Use multiple general purpose registers */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 7 - i;
        int r3 = r1 ^ r2;
        
        /* Clobber integer registers - simulating function call */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += r3;
        vi1 = r1;  /* Store to volatile extends liveness */
        
        /* ========== FLOATING POINT SECTION ========== */
        double f1 = vd1 * 1.1 + i;
        double f2 = vd2 * 0.9 - i;
        double f3 = f1 * f2;
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        result += (int)f3;
        vd1 = f1;
        
        /* ========== VECTOR SECTION ========== */
        v4si v1 = vec_int + i;
        v4si v2 = {i, i*2, i*3, i*4};
        v4si v3 = v1 * v2;
        
        /* Clobber MMX/vector registers */
        asm volatile (
            "# CLOBBER MMX REGS\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            "pxor %%mm3, %%mm3\n\t"
            "pxor %%mm4, %%mm4\n\t"
            "pxor %%mm5, %%mm5\n\t"
            "pxor %%mm6, %%mm6\n\t"
            "pxor %%mm7, %%mm7\n\t"
            "emms\n\t"
            :
            :
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
        );
        
        /* Use vector result */
        for (int j = 0; j < 4; j++) {
            result += v3[j];
        }
        vec_int = v1;
        
        /* ========== MIXED USE BEFORE CONDITIONAL ========== */
        /* Create a basic block ending with clobber */
        int mixed = (r1 & 0xFF) + (int)f3 + v3[0];
        
        /* Final clobber that could be at block end */
        asm volatile (
            "# FINAL CLOBBER AT BLOCK END\n\t"
            "mov $0, %%rax\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            :
            :
            : "rax", "xmm0"
        );
        
        /* Conditional jump after clobber - creates block boundary */
        if (mixed > 1000) {
            /* This creates a control flow edge after the asm */
            result -= 100;
        } else {
            result += mixed;
        }
        
        /* Update volatiles to prevent optimization */
        vi2 = result % 100;
        vd2 = result * 0.01;
        vec_float = (v4sf){result*0.001f, result*0.002f, 
                          result*0.003f, result*0.004f};
    }
    
    return result;
}

/* External function to force actual call */
static __attribute__((noinline)) 
int helper_call(int x) {
    return x * 3 + 7;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    
    /* Multiple calls with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call external function between test calls */
        total = helper_call(total);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
