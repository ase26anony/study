/* test_caller_save.c - Target GCC caller-save.cc lines 905-913 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types to stress caller-save */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 2.5;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*1.1f, seed*1.2f, seed*1.3f, seed*1.4f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* === INTEGER REGISTERS === */
        /* Force live values in call-clobbered integer registers */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 5 + i;
        int r3 = r1 ^ r2;
        int r4 = r1 + r2 * 7;
        
        /* Clobber multiple integer registers - simulating function call */
        asm volatile (
            "# Clobber integer regs\n\t"
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
        
        /* Use values after clobber - forces save/restore */
        result += r3;
        result += r4;
        
        /* === FLOATING POINT REGISTERS === */
        double d1 = vd1 * i;
        double d2 = vd2 / (i + 1);
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        
        /* Clobber floating point/SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
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
        
        /* Use FP values after clobber */
        result += (int)d3;
        result += (int)d4;
        
        /* === VECTOR REGISTERS === */
        v4si v1 = vec_int + i;
        v4si v2 = vec_int * i;
        v4si v3 = v1 + v2;
        
        v4sf f1 = vec_float * (i + 0.5f);
        v4sf f2 = vec_float / (i + 1.5f);
        v4sf f3 = f1 + f2;
        
        /* Clobber MMX registers */
        asm volatile (
            "# Clobber MMX regs\n\t"
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
        
        /* Use vector values after clobber */
        for (int j = 0; j < 4; j++) {
            result += v3[j];
            result += (int)f3[j];
        }
        
        /* Create basic block boundary with call-like asm at end */
        if (i % 3 == 0) {
            /* This creates a basic block ending with asm */
            asm volatile (
                "# Call-like clobber at potential BB end\n\t"
                "call dummy_label%=\n\t"
                "dummy_label%=:\n\t"
                "add $1, %%rax\n\t"
                :
                :
                : "rax", "rbx", "rcx", "memory"
            );
            /* Control flow continues here - previous asm could be BB_END */
            vi1 += result % 17;
        } else if (i % 3 == 1) {
            /* Alternative path to create more BB structure */
            asm volatile (
                "# Another clobber point\n\t"
                "mov $0, %%r12\n\t"
                "mov $0, %%r13\n\t"
                :
                :
                : "r12", "r13", "memory"
            );
            vi2 += result % 23;
        }
        
        /* Update volatiles to extend liveness */
        vd1 += 0.1;
        vd2 -= 0.1;
        vec_int += 1;
        vec_float *= 1.01f;
    }
    
    return result;
}

/* External function to force actual calls */
static __attribute__((noinline))
int helper_func(int x) {
    return x * 3 + 7;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call another function between test_caller_save calls */
        total = helper_func(total);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
