/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Non-inlineable function to force calls */
static __attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Target function with complex caller-save requirements */
static __attribute__((noinline)) 
long test_caller_save(int iterations, int seed) {
    volatile long acc = seed;  /* Force memory traffic */
    
    /* Variables that will live in call-clobbered registers */
    volatile long v1 = acc;
    volatile double v2 = acc * 0.5;
    volatile v4si v3 = {acc, acc+1, acc+2, acc+3};
    volatile v4sf v4 = {acc*1.1f, acc*1.2f, acc*1.3f, acc*1.4f};
    
    /* MMX variable */
    volatile long long mmx_val = acc * 2;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* --- BLOCK 1: Integer register pressure --- */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        long rax_val = v1 + i * 3;
        long rbx_val = v1 * 2 - i;
        long rcx_val = v1 ^ (i << 2);
        long rdx_val = v1 % (i + 1);
        
        /* Clobber integer registers with asm (simulating a call) */
        asm volatile (
            "# Clobber integer regs\n\t"
            "movq $0x12345678, %%rax\n\t"
            "movq $0x87654321, %%rbx\n\t"
            "movq $0xABCDEF, %%rcx\n\t"
            "movq $0xFEDCBA, %%rdx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber (forces save/restore) */
        v1 = rax_val + rbx_val + rcx_val + rdx_val;
        
        /* Call to function - creates basic block boundary */
        int call_result = external_func(i);
        
        /* --- BLOCK 2: SSE register pressure --- */
        /* Use xmm0-xmm3 (call-clobbered) */
        double xmm0_val = v2 + i * 0.25;
        v4si xmm1_val = v3 + (v4si){i, i*2, i*3, i*4};
        v4sf xmm2_val = v4 * (v4sf){1.0f + i*0.1f, 1.0f + i*0.2f, 
                                    1.0f + i*0.3f, 1.0f + i*0.4f};
        __m128 xmm3_val = _mm_set_ps(i*1.0f, i*2.0f, i*3.0f, i*4.0f);
        
        /* Clobber SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use SSE values after clobber */
        v2 = xmm0_val * 2.0;
        v3 = xmm1_val + (v4si){call_result, 0, 0, 0};
        v4 = xmm2_val + _mm_cvtps_ps(xmm3_val);
        
        /* Another function call at potential block end */
        call_result = external_func(call_result + i);
        
        /* --- BLOCK 3: MMX register pressure --- */
        /* Use mm0 (call-clobbered) */
        long long mm0_val = mmx_val + i * 5LL;
        
        /* Clobber MMX register */
        asm volatile (
            "# Clobber MMX reg\n\t"
            "pxor %%mm0, %%mm0\n\t"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "memory"
        );
        
        /* Use MMX value after clobber */
        mmx_val = mm0_val ^ 0xFF00FF00FF00FF00LL;
        
        /* Final call that might end a basic block */
        int final_call = external_func(mmx_val & 0xFF);
        
        /* Conditional jump to create block structure */
        if (final_call & 1) {
            /* This creates a control flow edge after the call */
            v1 += final_call;
        } else {
            v1 -= final_call;
        }
        
        /* Accumulate results */
        acc += v1 + (long)v2 + v3[0] + (long)v4[0] + mmx_val;
    }
    
    /* EMMS to clear MMX state */
    asm volatile ("emms" ::: "memory");
    
    return acc;
}

/* Wrapper to create multiple call sites */
static __attribute__((noinline))
long run_multiple_calls(int base) {
    long total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(base % 5 + 3, base);
    total += test_caller_save(base % 7 + 2, base * 2);
    total += test_caller_save(base % 3 + 4, base * 3);
    total += test_caller_save(base % 6 + 1, base * 4);
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    long checksum = 0;
    
    /* Multiple invocations to increase coverage chance */
    for (int i = 0; i < iterations; i++) {
        checksum += run_multiple_calls(i);
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    volatile long sink = checksum;
    return (int)(checksum % 256);
}
