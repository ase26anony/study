/* test_caller_save.c - Target GCC's caller-save insertion logic */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to force actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Live values in call-clobbered registers across multiple asm blocks */
    int int_acc = vi;
    float float_acc = vf;
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    v2di mmx_val = {vi * 2LL, vi * 3LL};
    
    /* Loop prevents hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Clobber integer registers - simulates function call */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0xDEADBEEF, %%eax\n\t"
            "mov $0xCAFEBABE, %%ebx\n\t"
            "mov $0x12345678, %%ecx\n\t"
            "mov $0x9ABCDEF0, %%edx"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use int_acc after clobber - forces save/restore */
        vi = int_acc;
        int_acc = vi ^ 0x55555555;
        
        /* ========== BLOCK 2: SSE register pressure ========== */
        /* Use xmm0-xmm5 (call-clobbered) */
        vec_float = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        float_acc = vec_float[0] + vec_float[1];
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "xorps %%xmm0, %%xmm0\n\t"
            "movaps %%xmm0, %%xmm1\n\t"
            "movaps %%xmm0, %%xmm2\n\t"
            "movaps %%xmm0, %%xmm3"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vector after clobber */
        vec_int = (v4si)vec_float;
        vec_int = vec_int + (v4si){i, i*2, i*3, i*4};
        
        /* ========== BLOCK 3: Mixed register pressure ========== */
        /* Use MMX and general registers together */
        mmx_val = mmx_val + (v2di){i, -i};
        
        /* Clobber MMX and more integer registers */
        asm volatile (
            "# CLOBBER MIXED REGS\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "movq %%mm0, %%mm1\n\t"
            "mov $0xF0F0F0F0, %%r8d\n\t"
            "mov $0x0F0F0F0F, %%r9d"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "mm1", "mm2", "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use values after clobber - forces insertion after asm */
        vec_int[0] = mmx_val[0] & 0xFFFFFFFF;
        vec_int[1] = mmx_val[1] & 0xFFFFFFFF;
        
        /* Create control flow edge after clobbering asm */
        if (int_acc & 1) {
            /* Force basic block boundary */
            vi = vec_int[0];
        } else {
            vi = vec_int[1];
        }
        
        /* Another asm to create multiple insertion points */
        asm volatile (
            "# FINAL CLOBBER\n\t"
            "mov $0, %%eax"
            : /* no outputs */
            : /* no inputs */
            : "rax", "memory"
        );
        
        /* Use value after final asm - may trigger BB_END update */
        float_acc = float_acc + vi * 0.5f;
    }
    
    /* Emulate MMX state cleanup */
    asm volatile ("emms" ::: "memory");
    
    /* Final computation using all live values */
    int result = int_acc + (int)float_acc + vec_int[0] + vec_int[1] + 
                 vec_int[2] + vec_int[3] + (int)mmx_val[0] + (int)mmx_val[1];
    
    return result & 0x7FFFFFFF;  /* Keep positive */
}

/* External function call to force caller-save around actual call */
static __attribute__((noinline))
int external_helper(int x, int y) {
    return x ^ y;
}

int main(void) {
    int total = 0;
    
    /* Call multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Mix of direct calls and through test_caller_save */
        total ^= test_caller_save(10 + (i % 5), i * 100);
        
        /* Actual function call between test_caller_save calls */
        if (i % 3 == 0) {
            total += external_helper(total, i);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
