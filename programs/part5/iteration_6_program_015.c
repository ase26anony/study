/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* MMX type */
    typedef long long v2si __attribute__((vector_size(8)));
    
    /* Live values in various call-clobbered registers */
    register int64_t r1 asm("rax") = vi + 1;
    register int64_t r2 asm("rbx") = vi + 2;  /* rbx is callee-saved on x86-64, but we'll use asm to clobber it */
    register double f1 asm("xmm0") = vf + 1.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    v2si mmx1 = {vi, vi+10};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use r1 in computation */
        int val1 = (r1 * 3) / 2;
        
        /* Clobber rax with asm (simulating a call) */
        asm volatile (
            "movq $0x12345678, %%rax\n\t"  /* Clobber rax */
            "addq $1, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "cc"  /* Clobber rax and condition codes */
        );
        
        /* Use original r1 value after clobber - forces save/restore */
        sum += val1 + (int)r1;
        
        /* ========== BLOCK 2: SSE register pressure ========== */
        /* Use xmm0 in computation */
        double temp = f1 * 2.5;
        
        /* Clobber xmm0 */
        asm volatile (
            "pxor %%xmm0, %%xmm0\n\t"  /* Zero xmm0 */
            : /* no outputs */
            : /* no inputs */
            : "xmm0"
        );
        
        /* Use original f1 after clobber */
        sum += (int)(temp + f1);
        
        /* ========== BLOCK 3: Vector register pressure ========== */
        /* Use vector values */
        vec1 += (v4si){1, 2, 3, 4};
        vec2 *= (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber multiple SSE registers */
        asm volatile (
            "movaps %%xmm1, %%xmm2\n\t"
            "xorps %%xmm3, %%xmm3\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Use vectors after clobber */
        sum += vec1[0] + vec1[2] + (int)vec2[1];
        
        /* ========== BLOCK 4: Mixed register pressure ========== */
        /* Create complex live range across asm */
        int complex_val = (r2 * 2) + (int)(f1 * 10.0) + vec1[3];
        
        /* Multiple clobbering asm statements in sequence */
        asm volatile (
            "movl $0xABCD, %%ebx\n\t"  /* Clobber rbx */
            : /* no outputs */
            : /* no inputs */
            : "rbx"
        );
        
        /* This asm creates a basic block boundary opportunity */
        asm volatile (
            "movq $0xFFFFFFFF, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx"
        );
        
        /* Use complex_val after clobbers */
        sum += complex_val;
        
        /* ========== BLOCK 5: Conditional to create BB end ========== */
        /* Create a basic block that ends with asm */
        if (sum & 1) {
            /* Use mmx register */
            mmx1 += (v2si){i, i*2};
            
            /* Clobber MMX register */
            asm volatile (
                "pxor %%mm0, %%mm0\n\t"
                : /* no outputs */
                : /* no inputs */
                : "mm0", "mm1"
            );
            
            /* Label to potentially mark BB end */
            asm volatile ("# BB_END_MARKER");
            
            /* Force BB_END update: next instruction after asm */
            sum += mmx1[0];
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "movq $0, %%r8\n\t"
                "movq $0, %%r9\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "r10", "r11"
            );
            sum -= 1;
        }
        
        /* Update volatile variables to prevent optimization */
        vi += i;
        vf += i * 0.5f;
        r1 = vi * 2;
        r2 = vi * 3;
        f1 = vf * 1.7;
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    asm volatile ("" : : "r"(x), "r"(y) : "memory");
    return x * y + 1;
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
    for (int i = 0; i < 10; i++) {
        /* Call that might be treated as BB end */
        int result = test_caller_save(iterations + i, i * 100);
        
        /* Use result in another call */
        result = helper_func(result, i + 1);
        
        total += result;
        
        /* Create register pressure around calls */
        register int64_t r10 asm("r10") = total;
        register double f2 asm("xmm1") = total * 0.01;
        
        asm volatile (
            "addq $5, %0\n\t"
            "addsd %1, %1\n\t"
            : "+r"(r10), "+x"(f2)
            :
            : "cc"
        );
        
        total = r10;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
