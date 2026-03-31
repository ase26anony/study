/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ====== BLOCK 1: Integer register pressure ====== */
        /* Use values in call-clobbered integer registers */
        int temp1 = (rax_val * 3) / 2;
        int temp2 = (rbx_val * 5) / 3;
        int temp3 = (rcx_val * 7) / 4;
        
        /* Simulate call that clobbers integer registers */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0xABCDEF01, %%rcx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "r10", "r11", "memory"
        );
        
        /* Use original values after clobber - forces save/restore */
        result += temp1 + rax_val;
        result += temp2 + rbx_val;
        result += temp3 + rcx_val;
        
        /* ====== BLOCK 2: SSE register pressure ====== */
        /* Use SSE registers */
        float f1 = xmm0_val * 2.0f;
        float f2 = xmm1_val * 3.0f;
        
        /* Vector operations */
        v4si vec_temp = vec_int + (v4si){i, i, i, i};
        v4sf vec_ftemp = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Simulate call that clobbers SSE/MMX registers */
        asm volatile (
            "# CLOBBER SSE/MMX REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
        );
        
        /* Use original values after clobber */
        result += (int)(f1 + xmm0_val);
        result += (int)(f2 + xmm1_val);
        result += vec_temp[0] + vec_temp[1];
        result += (int)vec_ftemp[0];
        
        /* ====== BLOCK 3: Mixed register pressure with conditional ====== */
        /* Create a basic block ending with asm clobber */
        if (i % 3 == 0) {
            /* More integer work */
            rax_val = (rax_val * 1103515245 + 12345) & 0x7fffffff;
            rbx_val = (rbx_val * 214013 + 2531011) & 0x7fffffff;
            
            /* Call-like asm at potential block end */
            asm volatile (
                "# CLOBBER MIXED REGS - POTENTIAL BLOCK END\n\t"
                "mov $0xDEADBEEF, %%r10\n\t"
                "mov $0xCAFEBABE, %%r11\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                :
                :
                : "r10", "r11", "xmm6", "xmm7", "memory"
            );
            
            /* Label/jump target after the asm */
            if (vi > 1000) {
                result += 1000;
            }
        } else {
            /* Alternative path */
            result += i * 7;
        }
        
        /* Update volatile to prevent optimization */
        vi += i;
        vf += i * 0.5f;
        
        /* Force register reloads */
        asm volatile ("# FORCE REGISTER RELOAD\n\t" : : : "memory");
    }
    
    return result & 0xFF;  /* Return something based on computation */
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int a, int b) {
    volatile int x = a + b;
    asm volatile ("# HELPER FUNCTION BODY\n\t" : : : "memory");
    return x * 2;
}

/* Main driver that creates multiple call sites */
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
    
    /* Insert actual function calls between computations */
    for (int i = 0; i < 5; i++) {
        total += helper_func(i, total);
        
        /* More register pressure around calls */
        volatile double d1 = total * 1.234;
        volatile double d2 = total * 5.678;
        
        asm volatile (
            "# MAIN LOOP CLOBBER\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        total = (total * 31 + 17) & 0xFFFF;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
