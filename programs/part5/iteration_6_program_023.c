/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Volatile variables to extend register liveness */
    volatile int vol_int = seed;
    volatile float vol_float = seed * 1.5f;
    volatile v4si vol_vec_int;
    volatile v4sf vol_vec_float;
    
    /* Variables that will be kept in call-clobbered registers */
    int int_val1 = vol_int;
    int int_val2 = vol_int + 1;
    float float_val1 = vol_float;
    float float_val2 = vol_float + 1.0f;
    v4si vec_int_val = {vol_int, vol_int + 1, vol_int + 2, vol_int + 3};
    v4sf vec_float_val = {vol_float, vol_float + 1.0f, 
                          vol_float + 2.0f, vol_float + 3.0f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int_val1 = int_val1 * 1103515245 + 12345;
        int_val2 = int_val2 * 1664525 + 1013904223;
        
        /* Simulate a call that clobbers specific integer registers */
        asm volatile (
            "# Clobber integer registers\n"
            "mov $0, %%rax\n"
            "mov $0, %%rbx\n"
            "mov $0, %%rcx\n"
            "mov $0, %%rdx\n"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += int_val1 + int_val2;
        
        /* ========== BLOCK 2: SSE register pressure ========== */
        /* Use xmm0-xmm5 (call-clobbered on x86-64 System V) */
        float_val1 = float_val1 * 1.1f + 0.5f;
        float_val2 = float_val2 * 0.9f - 0.3f;
        vec_float_val = vec_float_val * 1.05f;
        
        /* Simulate a call that clobbers SSE registers */
        asm volatile (
            "# Clobber SSE registers\n"
            "pxor %%xmm0, %%xmm0\n"
            "pxor %%xmm1, %%xmm1\n"
            "pxor %%xmm2, %%xmm2\n"
            "pxor %%xmm3, %%xmm3\n"
            "pxor %%xmm4, %%xmm4\n"
            "pxor %%xmm5, %%xmm5\n"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use SSE values after clobber */
        result += (int)float_val1 + (int)float_val2;
        result += (int)vec_float_val[0];
        
        /* ========== BLOCK 3: Mixed register pressure ========== */
        /* Create a basic block ending with a clobbering asm */
        vec_int_val = vec_int_val + (v4si){1, 2, 3, 4};
        
        /* This asm simulates a call at the end of a basic block */
        asm volatile (
            "# Clobber mixed registers - potential block end\n"
            "mov $0, %%r8\n"
            "mov $0, %%r9\n"
            "pxor %%xmm6, %%xmm6\n"
            "pxor %%xmm7, %%xmm7\n"
            :
            :
            : "r8", "r9", "xmm6", "xmm7", "memory"
        );
        
        /* Jump target to create control flow after the "call" */
        if (i & 1) {
            /* Use the vector value - forces save before asm, restore after */
            result += vec_int_val[0] + vec_int_val[1];
        } else {
            result += vec_int_val[2] + vec_int_val[3];
        }
        
        /* ========== BLOCK 4: Actual function call ========== */
        /* Use values before a real function call */
        int_val1 = result ^ int_val1;
        float_val1 = float_val1 + (float)result;
        
        /* Real function call - will definitely trigger caller-save */
        vol_int = rand();  /* rand() is a real function call */
        
        /* Use values after the call - forces save/restore */
        result = result ^ int_val1;
        result += (int)float_val1;
        
        /* Force spill/reload by alternating register usage */
        if (i % 3 == 0) {
            asm volatile (
                "# Extra clobber to create more save points\n"
                "mov $0, %%r10\n"
                "mov $0, %%r11\n"
                "pxor %%xmm8, %%xmm8\n"
                "pxor %%xmm9, %%xmm9\n"
                :
                :
                : "r10", "r11", "xmm8", "xmm9", "memory"
            );
        }
    }
    
    return result;
}

/* Another function to create more call sites */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int v = x;
    asm volatile (
        "# Helper function clobber\n"
        "mov $0, %%r12\n"
        "pxor %%xmm10, %%xmm10\n"
        :
        :
        : "r12", "xmm10", "memory"
    );
    return v * y + 123;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call helper function to create additional call sites */
        total += helper_func(res, s);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
