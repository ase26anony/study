/* test_caller_save.c - Target GCC caller-save.cc lines 905-913 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types with volatile to extend liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile double vd1 = seed * 3.14159;
    volatile double vd2 = seed * 2.71828;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v2di vec_long = {seed*5LL, seed*7LL};
    volatile v8hi vec_short = {seed, seed+1, seed+2, seed+3, 
                               seed+4, seed+5, seed+6, seed+7};
    
    int result = 0;
    
    /* Loop with invariant calls to prevent hoisting */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTER SECTION ========== */
        /* Force live values in call-clobbered integer registers */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 5 - i;
        long r3 = (long)vi1 * vi2 + i * 7;
        
        /* Clobber multiple integer registers - simulating function call */
        /* This should trigger caller-save for r1, r2, r3 */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += r1 + r2 + (int)(r3 & 0xFFFFFFFF);
        
        /* ========== FLOATING POINT SECTION ========== */
        /* Force live values in XMM registers */
        double d1 = vd1 * i + 1.0;
        double d2 = vd2 * i - 2.0;
        float f1 = (float)d1 * 0.5f;
        float f2 = (float)d2 * 0.25f;
        
        /* Clobber XMM registers */
        asm volatile (
            "# CLOBBER XMM REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            : 
            : 
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* Use floating point values after clobber */
        result += (int)(d1 + d2 + f1 + f2);
        
        /* ========== VECTOR SECTION ========== */
        /* Vector computations in SSE registers */
        v4si v1 = vec_int + i;
        v4si v2 = {i, i*2, i*3, i*4};
        v4si v3 = v1 * v2;
        
        v2di vl1 = vec_long + i;
        v2di vl2 = {i*3LL, i*5LL};
        v2di vl3 = vl1 + vl2;
        
        /* Clobber MMX/vector registers specifically */
        asm volatile (
            "# CLOBBER MMX/ADDITIONAL VECTOR REGS\n\t"
            "emms\n\t"  /* Empty MMX state */
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            "pxor %%mm3, %%mm3\n\t"
            "pxor %%mm4, %%mm4\n\t"
            "pxor %%mm5, %%mm5\n\t"
            "pxor %%mm6, %%mm6\n\t"
            "pxor %%mm7, %%mm7\n\t"
            : 
            : 
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "memory"
        );
        
        /* Use vector results after clobber */
        for (int j = 0; j < 4; j++) {
            result += v3[j];
        }
        result += (int)(vl3[0] + vl3[1]);
        
        /* ========== CREATE BASIC BLOCK ENDING WITH CLOBBER ========== */
        /* Additional computation to create more basic blocks */
        if (i % 3 == 0) {
            /* This creates a basic block ending with the asm clobber */
            int temp = result * 2;
            asm volatile (
                "# FINAL CLOBBER IN CONDITIONAL BLOCK\n\t"
                "mov $0, %%rax\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                : 
                : 
                : "rax", "xmm0", "memory"
            );
            result = temp + 1;
            
            /* Label to create control flow edge after the asm */
            /* This helps create a basic block where the asm is at the end */
            if (result > 1000) {
                /* Another computation to extend the block */
                result -= 500;
            }
        }
        
        /* Mix with short vector operations */
        v8hi vs1 = vec_short + (short)i;
        for (int j = 0; j < 8; j++) {
            result += vs1[j];
        }
        
        /* Update volatiles to keep them live */
        vi1 += i;
        vi2 -= i;
        vd1 *= 1.001;
        vd2 /= 1.001;
    }
    
    return result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    /* This function itself uses many registers */
    asm volatile (
        "# HELPER FUNCTION CLOBBERS\n\t"
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        : 
        : "r"(x), "r"(y)
        : "rax", "memory"
    );
    return x * y + 123;
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
        /* Call to test_caller_save - should trigger caller-save logic */
        int r1 = test_caller_save(iterations, i * 100);
        total += r1;
        
        /* Interleave with another function call */
        int r2 = helper_func(i, iterations);
        total += r2;
        
        /* More register-intensive computation between calls */
        volatile double d = 1.0;
        for (int j = 0; j < 5; j++) {
            d = d * 1.5 + j;
        }
        total += (int)d;
    }
    
    printf("Result checksum: %d\n", total);
    
    /* Additional test with different iteration patterns */
    int r3 = test_caller_save(iterations / 2, 999);
    int r4 = test_caller_save(iterations / 4, 777);
    
    printf("Additional results: %d, %d\n", r3, r4);
    printf("Total: %d\n", total + r3 + r4);
    
    return 0;
}
