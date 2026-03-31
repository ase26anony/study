/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Live values in call-clobbered registers */
    register long rax_val asm("rax") = vi + 1;
    register long rbx_val asm("rbx") = vi + 2;
    register long rcx_val asm("rcx") = vi + 3;
    register double xmm0_val asm("xmm0") = vf + 1.0;
    register double xmm1_val asm("xmm1") = vf + 2.0;
    v4si vec_val = {vi, vi+1, vi+2, vi+3};
    v4sf vecf_val = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* Step 1: Use integer registers */
        rax_val = rax_val * 3 + i;
        rbx_val = rbx_val * 5 + i;
        rcx_val = rcx_val * 7 + i;
        
        /* Force caller-save for integer regs with asm clobber */
        asm volatile (
            "# Clobber integer registers\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0xABCDEF01, %%rcx\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Use original values after clobber (forces save/restore) */
        sum += (int)rax_val + (int)rbx_val + (int)rcx_val;
        
        /* Step 2: Use SSE registers */
        xmm0_val = xmm0_val * 1.5 + i;
        xmm1_val = xmm1_val * 2.5 + i;
        
        /* Force caller-save for SSE regs */
        asm volatile (
            "# Clobber SSE registers\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : 
            : 
            : "xmm0", "xmm1", "memory"
        );
        
        /* Use original SSE values */
        sum += (int)xmm0_val + (int)xmm1_val;
        
        /* Step 3: Use vector registers */
        vec_val = vec_val + (v4si){i, i*2, i*3, i*4};
        vecf_val = vecf_val + (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
        
        /* Force caller-save for more registers */
        asm volatile (
            "# Clobber multiple registers\n\t"
            "mov $0xDEADBEEF, %%r8\n\t"
            "mov $0xBEEFDEAD, %%r9\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : 
            : 
            : "r8", "r9", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vector values after clobber */
        sum += vec_val[0] + vec_val[2] + (int)vecf_val[1];
        
        /* Create basic block ending with call-like asm */
        if (i & 1) {
            /* This asm simulates a function call that clobbers registers */
            asm volatile (
                "# Simulated function call\n\t"
                "call 1f\n\t"
                "1:\n\t"
                "pop %%r10\n\t"
                : 
                : 
                : "r10", "r11", "xmm6", "xmm7", "xmm8", "xmm9", 
                  "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                  "cc", "memory"
            );
            
            /* Label/jump to create control flow edge after "call" */
            if (vi > 100) {
                /* This creates a basic block boundary */
                goto update_point;
            }
        }
        
        update_point:
        /* Update volatile to prevent optimization */
        vi = vi + i;
        vf = vf + i * 0.5f;
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    asm volatile ("# Helper function body" : : : "memory");
    return x * y + 123;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(iterations, 1);
    total += test_caller_save(iterations / 2, 42);
    total += test_caller_save(iterations * 2, 123);
    
    /* Mix with actual function calls */
    for (int i = 0; i < 10; i++) {
        total += helper_func(i, iterations);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
