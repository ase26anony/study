/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at end of basic block
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types to stress caller-save logic */
    volatile int vi1 = seed;      /* Force memory traffic */
    volatile int vi2 = seed + 1;
    volatile float vf1 = seed * 1.5f;
    volatile float vf2 = seed * 2.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*1.0f, seed*2.0f, seed*3.0f, seed*4.0f};
    
    int result = 0;
    int i;
    
    /* Loop prevents hoisting of save/restore code */
    for (i = 0; i < iterations; i++) {
        /* === INTEGER REGISTERS === */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = vi1 * 3 + i;
        int b = vi2 * 7 - i;
        int c = a * b;
        int d = c ^ (a + b);
        
        /* Clobber integer registers - simulating function call */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%eax\n\t"
            "mov $0x9ABCDEF0, %%ebx\n\t"
            "mov $0x11111111, %%ecx\n\t"
            "mov $0x22222222, %%edx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        result += d + a - b;
        vi1 = result;  /* Update volatile to extend liveness */
        
        /* === FLOATING POINT REGISTERS === */
        /* Use xmm0-xmm3 (call-clobbered) */
        float f1 = vf1 * 2.0f + i;
        float f2 = vf2 * 3.0f - i;
        float f3 = f1 * f2;
        float f4 = f3 / (f1 + 1.0f);
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        result += (int)(f4 * 100.0f);
        vf1 = f4;  /* Update volatile */
        
        /* === VECTOR REGISTERS === */
        /* Use vector registers that overlap with SSE/MMX */
        v4si v1 = vec_int + i;
        v4si v2 = vec_int * 2 - i;
        v4si v3 = v1 * v2;
        
        /* Clobber more registers including MMX */
        asm volatile (
            "# CLOBBER MIXED REGS\n\t"
            "emms\n\t"  /* Clear MMX state */
            "mov $0x33333333, %%r8\n\t"
            "mov $0x44444444, %%r9\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r8", "r9", "xmm4", "xmm5", "mm0", "mm1", "memory"
        );
        
        /* Use vector results - forces save/restore of vector regs */
        result += v3[0] + v3[1] - v3[2];
        vec_int = v3;  /* Update volatile */
        
        /* === CREATE BASIC BLOCK ENDING WITH CLOBBER === */
        /* This creates a control flow edge right after a clobbering asm */
        if (result % 7 == 0) {
            /* Additional clobber at potential block end */
            asm volatile (
                "# POTENTIAL BLOCK-END CLOBBER\n\t"
                "mov $0x55555555, %%r10\n\t"
                "mov $0x66666666, %%r11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r10", "r11", "memory"
            );
            /* Label/jump creates control flow edge */
            result *= 2;
        } else {
            /* Alternative path also with clobber */
            asm volatile (
                "# ALTERNATIVE CLOBBER\n\t"
                "mov $0x77777777, %%r12\n\t"
                "mov $0x88888888, %%r13\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r12", "r13", "memory"
            );
            result /= 3;
        }
        
        /* Update volatiles to ensure liveness across asm statements */
        vi2 = result & 0xFF;
        vf2 = result * 0.01f;
    }
    
    return result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int r = x * y + 12345;
    /* Clobber more registers */
    asm volatile (
        "# HELPER CLOBBER\n\t"
        "mov $0x99999999, %%r14\n\t"
        "mov $0xAAAAAAAA, %%r15\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r14", "r15", "memory"
    );
    return r;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    int seed = 1;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < 5; i++) {
        int r1 = test_caller_save(iterations, seed + i * 10);
        int r2 = helper_func(r1, i + 1);
        total += r1 + r2;
        
        /* Call with different iteration counts */
        int r3 = test_caller_save(iterations / 2, seed + i * 20);
        total += r3;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
