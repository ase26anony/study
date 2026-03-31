/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Updating BB_END when inserting save/restore after block-end instruction
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
#define NOINLINE __attribute__((noinline))

/* Vector types to use SSE/MMX registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Non-inlineable helper to prevent optimization */
NOINLINE static int external_func(int x) {
    return x * 3 + 7;
}

/* Another non-inlineable function */
NOINLINE static float external_float(float x) {
    return x * 1.5f - 2.0f;
}

/* Function that uses many call-clobbered registers across multiple calls */
NOINLINE static int test_caller_save(int iterations, int seed) {
    /* Volatile variables to extend register liveness */
    volatile int vol_int = seed;
    volatile float vol_float = seed * 0.5f;
    volatile v4si vol_vec = {seed, seed + 1, seed + 2, seed + 3};
    
    int result = 0;
    int i;
    
    /* Loop to prevent hoisting of save/restore code */
    for (i = 0; i < iterations; i++) {
        /* ===== INTEGER REGISTERS ===== */
        /* Use multiple integer call-clobbered registers */
        int rax_val = vol_int + i * 11;
        int rbx_val = vol_int * 3 - i;
        int rcx_val = rax_val ^ rbx_val;
        int rdx_val = rcx_val * 2 + 1;
        
        /* Clobber integer registers - simulating function call */
        asm volatile (
            "# Clobber integer registers\n\t"
            "movl $0x12345678, %%eax\n\t"
            "movl $0x9ABCDEF0, %%ebx\n\t"
            "movl $0x11111111, %%ecx\n\t"
            "movl $0x22222222, %%edx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += rax_val;
        result ^= rbx_val;
        result *= rcx_val;
        result -= rdx_val;
        
        /* ===== FLOATING POINT REGISTERS ===== */
        float xmm0_val = vol_float + i * 0.25f;
        float xmm1_val = vol_float * 2.0f - i * 0.1f;
        double xmm2_val = (double)vol_float * 3.14;
        
        /* Clobber SSE registers */
        asm volatile (
            "# Clobber SSE registers\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Use values after clobber */
        result += (int)(xmm0_val * 100.0f);
        result += (int)(xmm1_val * 50.0f);
        result += (int)xmm2_val;
        
        /* ===== VECTOR REGISTERS ===== */
        v4si vec_val = vol_vec;
        vec_val[0] += i;
        vec_val[1] -= i * 2;
        vec_val[2] ^= i;
        vec_val[3] *= (i + 1);
        
        /* Clobber more vector registers */
        asm volatile (
            "# Clobber more vector registers\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vector values */
        result += vec_val[0] + vec_val[1] + vec_val[2] + vec_val[3];
        
        /* ===== ACTUAL FUNCTION CALL AT BLOCK END ===== */
        /* This creates a basic block ending with a call */
        int call_result = external_func(result);
        
        /* CRITICAL: Immediate use after call with conditional jump */
        /* This creates control flow edge right after call */
        if (call_result > 1000) {
            /* Use clobbered registers again */
            float float_result = external_float(vol_float + i);
            result += (int)float_result;
            
            /* Another asm clobber after conditional block */
            asm volatile (
                "# Additional clobber in conditional path\n\t"
                "movl $0x33333333, %%r8d\n\t"
                "movl $0x44444444, %%r9d\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "memory"
            );
            
            result ^= (i << 3);
        } else {
            /* Alternative path also using clobbered registers */
            asm volatile (
                "# Clobber in else path\n\t"
                "movl $0x55555555, %%r10d\n\t"
                "movl $0x66666666, %%r11d\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r10", "r11", "memory"
            );
            
            result |= (i << 2);
        }
        
        /* Update volatile to keep values live */
        vol_int = result & 0xFF;
        vol_float = (result % 100) * 0.01f;
        vol_vec[0] = result;
        vol_vec[1] = result + 1;
        vol_vec[2] = result + 2;
        vol_vec[3] = result + 3;
    }
    
    return result;
}

/* Another test function with different pattern */
NOINLINE static int test_caller_save2(int iterations, int seed) {
    volatile double vol_double = seed * 1.234;
    volatile v2di vol_v2di = {seed, seed * 2LL};
    
    int result = seed;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Use MMX-style registers (call-clobbered) */
        long long mmx_val = vol_v2di[0] + i * 100LL;
        double xmm_val = vol_double * i;
        
        /* Clobber MMX/SSE registers */
        asm volatile (
            "# Clobber mixed registers\n\t"
            "emms\n\t"  /* Empty MMX state */
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "xmm6", "xmm7", "memory"
        );
        
        result += (int)mmx_val;
        result += (int)xmm_val;
        
        /* Function call that might be at block end */
        int tmp = external_func(result);
        
        /* Label after call to potentially create block boundary */
        after_call:
        result = tmp ^ (i * 7);
        
        /* Another asm clobber */
        asm volatile (
            "# Final clobber in iteration\n\t"
            "movl $0x77777777, %%r12d\n\t"
            "movl $0x88888888, %%r13d\n\t"
            "movl $0x99999999, %%r14d\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r12", "r13", "r14", "memory"
        );
        
        vol_double += result * 0.001;
        vol_v2di[0] += result;
        vol_v2di[1] -= result;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 10;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Testing caller-save optimization patterns...\n");
    
    /* Call test functions multiple times with different arguments */
    int result1 = test_caller_save(iterations, seed);
    printf("Result 1: %d\n", result1);
    
    int result2 = test_caller_save2(iterations, seed + 1);
    printf("Result 2: %d\n", result2);
    
    int result3 = test_caller_save(iterations / 2, seed + 2);
    printf("Result 3: %d\n", result3);
    
    int final_result = result1 ^ result2 ^ result3;
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
