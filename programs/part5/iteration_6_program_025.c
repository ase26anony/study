/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Live values in call-clobbered registers */
    register int64_t r12_val asm("r12") = vi + 1;  /* Callee-saved - won't be clobbered */
    register int rax_val asm("rax") = vi * 2;      /* Call-clobbered */
    register int rbx_val asm("rbx") = vi * 3;      /* Callee-saved */
    register float xmm0_val asm("xmm0") = vf + 1.0f;
    register double xmm1_val asm("xmm1") = vf * 2.0;
    v4si vec_val = {vi, vi+1, vi+2, vi+3};
    v4sf vecf_val = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* Basic block starting here */
        
        /* 1. Use integer register values before clobber */
        int temp1 = rax_val * 2 + rbx_val;
        
        /* 2. Clobber RAX (call-clobbered) with asm that looks like a call */
        asm volatile (
            "movq $0x12345678, %%rax\n\t"   /* Clobber RAX */
            "addq $1, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "cc"  /* Clobber RAX and flags */
        );
        
        /* 3. Use the original value after clobber - forces save/restore */
        /* This should trigger caller-save insertion AFTER the asm */
        sum += temp1 + rax_val;  /* rax_val needs restore here */
        
        /* 4. Use SSE register before clobber */
        float ftemp = xmm0_val * 2.0f + xmm1_val;
        
        /* 5. Clobber XMM0, XMM1 (call-clobbered) */
        asm volatile (
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "cc"
        );
        
        /* 6. Use original SSE values after clobber */
        sum += (int)(ftemp + xmm0_val + xmm1_val);
        
        /* 7. Use vector values before clobber */
        vec_val[0] += i;
        vecf_val[0] += (float)i;
        
        /* 8. Clobber multiple vector registers */
        asm volatile (
            "movq $0, %%mm0\n\t"      /* Clobber MMX register */
            "pxor %%xmm2, %%xmm2\n\t" /* Clobber XMM2 */
            : /* no outputs */
            : /* no inputs */
            : "mm0", "xmm2", "cc"
        );
        
        /* 9. Use vector values after clobber */
        sum += vec_val[0] + (int)vecf_val[0];
        
        /* 10. Another clobbering asm at potential basic block end */
        /* This asm might end a basic block if followed by label/jump */
        asm volatile (
            "movq $0xABCD, %%rcx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rcx", "cc"
        );
        
        /* Conditional jump to create basic block boundary */
        /* The previous asm might be BB_END before caller-save insertion */
        if (i & 1) {
            /* Use volatile to prevent optimization */
            vi = sum % 100;
        } else {
            vf = sum * 0.5f;
        }
        
        /* Update register values for next iteration */
        rax_val += vi;
        rbx_val += vi;
        xmm0_val += vf;
        xmm1_val += vf * 0.5;
        vec_val[1] += i;
        vecf_val[1] += (float)i;
    }
    
    /* Final use of all values */
    sum += rax_val + rbx_val + (int)xmm0_val + (int)xmm1_val;
    sum += vec_val[0] + vec_val[1] + (int)vecf_val[0] + (int)vecf_val[1];
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline, optimize("O0")))
int helper_func(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    /* Force register usage */
    asm volatile ("" : "+r"(a), "+r"(b));
    return a * b;
}

/* Main test driver */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < 10; i++) {
        /* Call real function between test_caller_save calls */
        int intermediate = helper_func(i, iterations);
        
        /* Call our test function - this should trigger caller-save */
        int result = test_caller_save(iterations + (i % 5), i * 100 + intermediate);
        
        total += result;
        
        /* Another real function call */
        intermediate = helper_func(result % 100, i + 1);
        total += intermediate;
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with mixed register types */
    {
        double d1 = 1.0, d2 = 2.0, d3 = 3.0;
        float f1 = 4.0f, f2 = 5.0f;
        int i1 = 6, i2 = 7, i3 = 8;
        
        /* Force use of various call-clobbered registers */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %2, %%xmm1\n\t"
            "movsd %3, %%xmm2\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "addsd %%xmm2, %%xmm0\n\t"
            "mov %4, %%eax\n\t"
            "mov %5, %%ebx\n\t"
            "mov %6, %%ecx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            : "+r"(i1), "+r"(i2)
            : "x"(d1), "x"(d2), "x"(d3), "r"(i1), "r"(i2), "r"(i3)
            : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "cc"
        );
        
        printf("Mixed: %d %f\n", i1, d1);
    }
    
    return total != 0 ? 0 : 1;
}
