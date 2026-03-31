/* test_caller_save.c - Target GCC's caller-save insertion logic */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to force actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register types with volatile to extend liveness */
    volatile int vi1 = seed * 3;
    volatile int vi2 = seed + 7;
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 2.5;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed+1, seed+2, seed+3};
    volatile v4sf vec_float = {seed*1.1f, seed*1.2f, seed*1.3f, seed*1.4f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTER CLOBBERING ========== */
        /* Use multiple general purpose registers */
        int r1 = vi1 * i + 123;
        int r2 = vi2 * i - 456;
        int r3 = r1 ^ r2;
        int r4 = r1 + r2 * 3;
        
        /* Clobber multiple integer registers - simulating function call */
        /* This forces caller-save for live values in these registers */
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
        result += r3 + r4;
        vi1 = r1 ^ result;
        vi2 = r2 + result;
        
        /* ========== FLOATING POINT REGISTER CLOBBERING ========== */
        double d1 = vd1 * i + 3.14159;
        double d2 = vd2 * i - 2.71828;
        double d3 = d1 * d2;
        
        /* Clobber floating point/SSE registers */
        asm volatile (
            "# CLOBBER FP/SSE REGS\n\t"
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
        
        /* Use FP values after clobber */
        result += (int)(d3 * 1000);
        vd1 = d1 + result;
        vd2 = d2 - result;
        
        /* ========== VECTOR REGISTER CLOBBERING ========== */
        v4si v1 = vec_int * i;
        v4sf v2 = vec_float * (float)i;
        v4si v3 = v1 + (v4si){result, result, result, result};
        
        /* Clobber MMX registers too */
        asm volatile (
            "# CLOBBER MMX REGS\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            "pxor %%mm3, %%mm3\n\t"
            "pxor %%mm4, %%mm4\n\t"
            "pxor %%mm5, %%mm5\n\t"
            "pxor %%mm6, %%mm6\n\t"
            "pxor %%mm7, %%mm7\n\t"
            "emms\n\t"  /* Switch back from MMX state */
            :
            :
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "memory"
        );
        
        /* Use vector values after clobber */
        for (int j = 0; j < 4; j++) {
            result += v3[j];
        }
        vec_int = v1 ^ result;
        vec_float = v2 * (float)(result % 100);
        
        /* Create basic block ending with asm clobber followed by jump */
        if (i % 2 == 0) {
            /* This creates a control flow edge after the asm */
            asm volatile (
                "# FINAL CLOBBER BEFORE BRANCH\n\t"
                "mov $0, %%rax\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                :
                :
                : "rax", "xmm0", "memory"
            );
            /* Label/jump creates block boundary */
            result ^= 0xAAAA;
        } else {
            result ^= 0x5555;
        }
        
        /* Force another clobber at potential block end */
        if (i == iterations - 1) {
            /* This asm could be at block end before insertion */
            asm volatile (
                "# POTENTIAL BLOCK-END CLOBBER\n\t"
                "mov $0, %%rbx\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                :
                :
                : "rbx", "xmm1", "memory"
            );
            /* Computation after - forces save insertion after block-end asm */
            result += vi1 * vi2;
        }
    }
    
    return result;
}

/* External function call to force caller-save across actual call */
static __attribute__((noinline))
int external_helper(int x, int y) {
    return x * y + 12345;
}

/* Main driver with multiple calls to create pressure */
int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    int total = 0;
    
    /* Multiple calls with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call external function between test calls */
        /* This creates additional caller-save pressure */
        int tmp = external_helper(res, s);
        total ^= tmp;
        
        /* Another asm clobber in main */
        asm volatile (
            "# MAIN CLOBBER\n\t"
            "mov $0, %%r12\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            :
            :
            : "r12", "xmm4", "memory"
        );
    }
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
