/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    
    /* Variables that must survive across asm clobbers */
    int int_acc = vi;
    float float_acc = vf;
    v4si vec_acc = {vi, vi+1, vi+2, vi+3};
    v4sf vecf_acc = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < iterations; i = i + 1) {
        /* --- Integer register pressure --- */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int r1 = int_acc * 3 + i;
        int r2 = r1 ^ 0x12345678;
        int r3 = r2 << 3;
        
        /* Clobber integer registers - simulating a function call */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0xDEADBEEF, %%rax\n\t"
            "mov $0xCAFEBABE, %%rbx\n\t"
            "mov $0x8BADF00D, %%rcx\n\t"
            "mov $0xFACEB00C, %%rdx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        int_acc = r3 + (r1 & r2);
        
        /* --- Floating point register pressure --- */
        /* Use xmm0-xmm5 (call-clobbered) */
        float f1 = float_acc * 1.618034f + i;
        float f2 = f1 * f1 - 2.0f;
        
        /* Clobber SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use float values after clobber */
        float_acc = f2 / (f1 + 1.0f);
        
        /* --- Vector register pressure --- */
        /* Vector operations using SSE registers */
        v4si v1 = vec_acc + (v4si){i, i+1, i+2, i+3};
        v4si v2 = v1 * (v4si){2, 3, 4, 5};
        
        /* Another clobber - this time mixed */
        asm volatile (
            "# Clobber mixed regs\n\t"
            "mov $0, %%r8\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r8", "r9", "r10", "r11", 
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "memory"
        );
        
        /* Use vector after clobber - critical for insertion after call */
        vec_acc = v2 | v1;
        
        /* --- Create basic block ending with asm clobber --- */
        /* This asm will be treated like a call for caller-save purposes */
        if (int_acc & 1) {
            /* Branch creates control flow, asm at end of basic block */
            asm volatile (
                "# End-of-block clobber\n\t"
                "mov $0x12345678, %%r12\n\t"
                "mov $0x87654321, %%r13\n\t"
                "pxor %%xmm12, %%xmm12\n\t"
                "pxor %%xmm13, %%xmm13\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r12", "r13", "r14", "r15",
                  "xmm12", "xmm13", "xmm14", "xmm15",
                  "memory"
            );
            /* Label/jump after asm creates block boundary */
            int_acc += 1000;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0x55555555, %%rax\n\t"
                "mov $0xAAAAAAAA, %%rbx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "rcx", "memory"
            );
            int_acc -= 500;
        }
        
        /* --- MMX register pressure (call-clobbered) --- */
        /* Use MMX registers if available */
        long long mmx_val = (long long)int_acc * 0x10001;
        asm volatile (
            "# Clobber MMX regs\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "mm1", "mm2", "memory"
        );
        
        /* Use MMX value after clobber */
        int_acc += (int)(mmx_val >> 32);
        
        /* Force spill/reload by using all values together */
        vi = int_acc + (int)float_acc + vec_acc[0];
    }
    
    /* Final computation using all live values */
    int result = int_acc + (int)float_acc + vec_acc[0] + vec_acc[1];
    return result & 0x7FFFFFFF;  /* Keep positive */
}

/* External function call to force caller-save around actual call */
static __attribute__((noinline))
int external_helper(int x, int y) {
    return x ^ y;
}

/* Another function to create call site with live values */
static __attribute__((noinline))
int complex_callsite(int base) {
    volatile int a = base * 3;
    volatile int b = base + 7;
    volatile float c = base * 2.5f;
    
    /* Live values across call */
    int x1 = a * 2 + 1;
    int x2 = b << 3;
    float x3 = c * 1.333f;
    
    /* Function call with clobbered registers */
    int tmp = external_helper(x1, x2);
    
    /* Use values after call - forces caller-save */
    return tmp + (int)x3 + a + b;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    int seed = 1;
    
    /* Multiple calls to test_caller_save with different seeds */
    for (int i = 0; i < 10; i++) {
        int result = test_caller_save(iterations, seed + i * 17);
        total += result;
        
        /* Also test with actual function calls */
        int call_result = complex_callsite(seed + i * 23);
        total += call_result;
        
        printf("Iteration %d: test=%d, callsite=%d, total=%d\n",
               i, result, call_result, total);
    }
    
    /* Final checksum */
    total = total & 0x7FFFFFFF;
    printf("Final checksum: %d (0x%08x)\n", total, total);
    
    return total == 0 ? 1 : 0;
}
