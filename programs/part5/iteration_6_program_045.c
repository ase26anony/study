/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Updating BB_END when inserting save/restore after block-end instruction
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) int test_caller_save(int iterations, int seed);
static __attribute__((noinline)) int dummy_call(int x);

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

/* Dummy function that compiler can't analyze */
int dummy_call(int x) {
    /* Use asm to prevent inlining and optimization */
    asm volatile ("" : "+r" (x) : : "memory");
    return x ^ 0x55AA55AA;
}

/* Main test function with complex register usage */
static int test_caller_save(int iterations, int seed) {
    /* Volatile variables to extend liveness across asm clobbers */
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile float f1 = seed * 0.5f;
    volatile float f2 = seed * 1.5f;
    
    /* Vector variables - will use SSE registers */
    v4si vec_int = {v1, v2, v1 + 1, v2 + 1};
    v4sf vec_float = {f1, f2, f1 + 1.0f, f2 + 1.0f};
    
    /* MMX variable */
    mmx_t mmx_val = {seed * 3LL};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ====== PHASE 1: Integer register pressure ====== */
        /* Compute values in call-clobbered integer registers */
        int rax_val = v1 + i * 3;
        int rbx_val = v2 + i * 7;
        int rcx_val = rax_val ^ rbx_val;
        int rdx_val = rax_val * rbx_val;
        
        /* Clobber integer registers with asm - simulating a call */
        asm volatile (
            "# Clobber integer registers\n"
            "mov %0, %%rax\n"
            "mov %1, %%rbx\n"
            "mov %2, %%rcx\n"
            "mov %3, %%rdx\n"
            : /* no outputs */
            : "r" (rax_val), "r" (rbx_val), "r" (rcx_val), "r" (rdx_val)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += rax_val;
        result ^= rbx_val;
        result += rcx_val;
        result ^= rdx_val;
        
        /* Make a real function call that ends a basic block */
        result = dummy_call(result);
        /* Basic block likely ends here before the label */
        
        /* Label creates new basic block, making previous call a block end */
        if (result & 1) {
            /* ====== PHASE 2: SSE register pressure ====== */
            /* SSE computations */
            vec_int += (v4si){i, i*2, i*3, i*4};
            vec_float *= (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
            
            /* Extract and use SSE values */
            int sse_result = vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3];
            float sse_float = vec_float[0] + vec_float[1];
            
            /* Clobber SSE registers */
            asm volatile (
                "# Clobber SSE registers\n"
                "movdqa %0, %%xmm0\n"
                "movdqa %1, %%xmm1\n"
                "addps %%xmm1, %%xmm0\n"
                : /* no outputs */
                : "x" (vec_int), "x" (vec_float)
                : "xmm0", "xmm1", "memory"
            );
            
            result += sse_result + (int)sse_float;
            
            /* Another function call that could be a block end */
            result = dummy_call(result);
            /* Control flow continues - this call might be block end */
            
            /* ====== PHASE 3: Mixed register pressure ====== */
            /* Use MMX registers */
            mmx_val += (mmx_t){i * 5LL};
            
            /* Clobber MMX register */
            asm volatile (
                "# Clobber MMX register\n"
                "movq %0, %%mm0\n"
                "psllq $2, %%mm0\n"
                : /* no outputs */
                : "r" (mmx_val)
                : "mm0", "memory"
            );
            
            /* Extract MMX value */
            long long mmx_extract;
            asm volatile ("movq %%mm0, %0" : "=r" (mmx_extract));
            result += (int)(mmx_extract & 0xFFFFFFFF);
            
            /* EMMS to clear MMX state - acts like a call */
            asm volatile ("emms" ::: "memory");
        } else {
            /* Alternative path with different register usage */
            v4si alt_vec = {result, result*2, result*3, result*4};
            
            /* Clobber different SSE register */
            asm volatile (
                "movdqa %0, %%xmm2\n"
                "pslld $1, %%xmm2\n"
                : /* no outputs */
                : "x" (alt_vec)
                : "xmm2", "memory"
            );
            
            result += alt_vec[0];
        }
        
        /* Update volatiles to keep them live */
        v1 += result & 0xFF;
        v2 += (result >> 8) & 0xFF;
        f1 += (result & 0xFF) * 0.01f;
        f2 += ((result >> 8) & 0xFF) * 0.01f;
        
        /* Final call in loop iteration - potential block end */
        if (i < iterations - 1) {
            result = dummy_call(result);
        }
    }
    
    return result;
}

/* Multiple calls with different arguments to increase coverage */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    
    /* Multiple calls to test_caller_save with different seeds */
    for (int s = 0; s < 5; s++) {
        int result = test_caller_save(iterations, s * 1000 + 12345);
        total += result;
        printf("Call %d: result = %d (0x%08x)\n", s, result, result);
    }
    
    printf("Total checksum: %d (0x%08x)\n", total, total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0x12345678) {
        printf("Impossible!\n");
    }
    
    return total != 0 ? 0 : 1;
}
