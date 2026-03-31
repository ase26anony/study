/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types with volatile to extend liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 2.5;
    
    /* Vector types for SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*1.1f, seed*1.2f, seed*1.3f, seed*1.4f};
    
    /* MMX type (8-byte vector) */
    typedef int v2si __attribute__((vector_size(8)));
    volatile v2si mmx_vec = {seed, seed*2};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* === INTEGER REGISTERS === */
        /* Compute with integer values */
        int int_temp = vi1 * 3 + vi2 * 7;
        
        /* Clobber multiple integer registers - simulating a call */
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
        
        /* Use the computed value after clobber - forces save/restore */
        result += int_temp;
        
        /* === FLOATING POINT REGISTERS === */
        double fp_temp = vd1 * 2.0 + vd2 * 3.0;
        
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
        
        result += (int)fp_temp;
        
        /* === VECTOR REGISTERS === */
        v4si vec_temp = vec_int + (v4si){i, i*2, i*3, i*4};
        
        /* Another clobbering asm - this one ends basic block */
        asm volatile (
            "# Clobber more regs and end basic block\n\t"
            "mov $0, %%rax\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            :
            :
            : "rax", "xmm0", "memory"
        );
        
        /* Use vector result - creates live value across asm */
        for (int j = 0; j < 4; j++) {
            result += vec_temp[j];
        }
        
        /* === MMX REGISTERS === */
        v2si mmx_temp = mmx_vec + (v2si){i, i*3};
        
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
            "emms\n\t"  /* Switch back from MMX mode */
            :
            :
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
        );
        
        result += mmx_temp[0] + mmx_temp[1];
        
        /* === FLOATING VECTOR === */
        v4sf float_vec_temp = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Final clobber in the loop - positioned to potentially end a BB */
        asm volatile (
            "# Final clobber in loop iteration\n\t"
            "mov $0, %%r10\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            :
            :
            : "r10", "xmm5", "memory"
        );
        
        /* Conditional that creates control flow after asm */
        if (result & 1) {
            /* Use float vector result */
            for (int j = 0; j < 4; j++) {
                result += (int)float_vec_temp[j];
            }
        } else {
            result += i;
        }
        
        /* Update volatiles to prevent optimization */
        vi1 += i;
        vd1 += 0.5;
        vec_int += (v4si){1, 2, 3, 4};
    }
    
    return result;
}

/* Another function to create actual calls */
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
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        int seed = i * 17 + argc;
        
        /* Call helper to create additional call instructions */
        seed = helper_func(seed);
        
        int result = test_caller_save(iterations, seed);
        total += result;
        
        /* Another helper call between test_caller_save calls */
        total = helper_func(total) % 1000000;
    }
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
