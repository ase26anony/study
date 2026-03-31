/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* MMX type */
    typedef long long v1di __attribute__((vector_size(8)));
    
    /* Live values in call-clobbered registers across multiple asm blocks */
    int int_acc = vi;
    float float_acc = vf;
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    v1di mmx_val = {vi * 2LL};
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* 1. Integer computation using rax, rbx, rcx */
        int_acc = (int_acc * 1103515245 + 12345) & 0x7fffffff;
        
        /* Clobber integer registers - simulating a call */
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
        
        /* Use the saved integer value - forces save/restore */
        vi = int_acc;
        
        /* 2. Floating-point computation using xmm registers */
        float_acc = float_acc * 1.6180339887f + (float)i;
        
        /* Clobber SSE registers */
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
        
        /* Use the saved float value */
        vf = float_acc;
        
        /* 3. Vector integer computation */
        vec_int = vec_int + (v4si){i, i*2, i*3, i*4};
        
        /* Another asm clobber - placed strategically */
        asm volatile (
            "# Clobber more regs\n\t"
            "mov $0, %%r12\n\t"
            "mov $0, %%r13\n\t"
            :
            :
            : "r12", "r13", "memory"
        );
        
        /* 4. Vector float computation */
        vec_float = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* 5. MMX computation */
        mmx_val = mmx_val + (v1di){i * 3LL};
        
        /* Clobber MMX registers */
        asm volatile (
            "# Clobber MMX regs\n\t"
            "emms\n\t"  /* Empty MMX state */
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            "pxor %%mm3, %%mm3\n\t"
            :
            :
            : "mm0", "mm1", "mm2", "mm3"
        );
        
        /* Use MMX value - forces save around the asm */
        vi = (int)(mmx_val[0] & 0x7fffffff);
        
        /* Conditional jump to create basic block boundary after asm */
        if (int_acc % 100 < 50) {
            /* Another clobbering asm at block end */
            asm volatile (
                "# Final clobber in conditional\n\t"
                "mov $0, %%r14\n\t"
                "mov $0, %%r15\n\t"
                :
                :
                : "r14", "r15", "memory"
            );
            /* Use values after clobber */
            vf = float_acc * 0.5f;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0, %%rax\n\t"
                "mov $0, %%xmm0\n\t"
                :
                :
                : "rax", "xmm0", "memory"
            );
        }
        
        /* Mix all values to create complex liveness */
        int_acc += (int)vf + vec_int[0] + (int)vec_float[0];
    }
    
    /* Final computation using all live values */
    int result = int_acc + (int)float_acc + vec_int[0] + (int)vec_float[0] + (int)mmx_val[0];
    return result & 0xffff;
}

/* External function call to force caller-save */
static __attribute__((noinline)) 
int external_helper(int x) {
    return x * 3 + 7;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    for (int i = 0; i < 10; i++) {
        /* Call test function multiple times with different seeds */
        int result = test_caller_save(iterations, i * 100);
        total += result;
        
        /* Call external function between test calls */
        total = external_helper(total);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
