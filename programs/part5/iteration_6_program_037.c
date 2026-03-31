/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory operations */
    volatile float vf = seed * 0.5f;
    
    /* Vector types for SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that will be in call-clobbered registers */
    register long r1 asm("rax") = vi + 1;
    register long r2 asm("rbx") = vi + 2;  /* rbx is callee-saved on x86-64, but we'll clobber it anyway */
    register long r3 asm("rcx") = vi + 3;
    register double d1 asm("xmm0") = vf + 1.0;
    register double d2 asm("xmm1") = vf + 2.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ====== BLOCK 1: Integer register pressure ====== */
        /* Use integer registers in computation */
        r1 = r1 * 1103515245 + 12345;
        r2 = r2 * 1103515245 + 12345;
        r3 = r3 * 1103515245 + 12345;
        
        /* Clobber integer registers with asm (simulating a call) */
        asm volatile (
            "# Clobber integer regs\n\t"
            "movq $0x12345678, %%rax\n\t"
            "movq $0x87654321, %%rbx\n\t"
            "movq $0xABCDEF01, %%rcx\n\t"
            "movq $0xFFFFFFFF, %%rdx\n\t"
            "movq $0xAAAAAAAA, %%rsi\n\t"
            "movq $0xBBBBBBBB, %%rdi\n\t"
            "movq $0xCCCCCCCC, %%r8\n\t"
            "movq $0xDDDDDDDD, %%r9\n\t"
            "movq $0xEEEEEEEE, %%r10\n\t"
            "movq $0x11111111, %%r11\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use the original values after clobber (forces save/restore) */
        sum += (r1 ^ r2 ^ r3) & 0xFF;
        
        /* ====== BLOCK 2: Floating-point register pressure ====== */
        /* Use FP/vector registers */
        d1 = d1 * 1.1 + 0.5;
        d2 = d2 * 1.2 - 0.3;
        vec1 = vec1 + (v4si){1, 2, 3, 4};
        vec2 = vec2 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber FP/vector registers */
        asm volatile (
            "# Clobber FP/vector regs\n\t"
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
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* Use vector values after clobber */
        sum += vec1[0] + vec1[1] + vec1[2] + vec1[3];
        sum += (int)(vec2[0] + vec2[1] + vec2[2] + vec2[3]);
        
        /* ====== BLOCK 3: Mixed register pressure with call ====== */
        /* Create a basic block ending with value usage */
        int temp = vi + i;
        
        /* Use all register types */
        r1 = (r1 + temp) | 1;
        d1 = d1 + temp * 0.01;
        
        /* This asm simulates a function call that clobbers everything */
        /* The key: this asm statement ends a basic block */
        asm volatile (
            "# Simulate function call\n\t"
            "call dummy_label%=\n\t"
            "dummy_label%=:\n\t"
            "addq $1, %%rsp\n\t"  /* Adjust for the fake call */
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory", "cc"
        );
        
        /* CRITICAL: This use after the asm creates need for save AFTER the call */
        /* The save instruction insertion after the call may trigger BB_END update */
        sum += r1 + (int)d1;
        
        /* Force control flow to create basic block boundaries */
        if (sum & 1) {
            /* This label creates a jump target after the asm */
            vi = vi * 2;
        } else {
            vi = vi / 2;
        }
        
        /* Another clobber to increase pressure */
        asm volatile (
            "# Final clobber\n\t"
            :
            :
            : "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Final use of all values */
        sum = (sum * 31 + r1) ^ (int)d1 ^ vec1[0];
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int v = x + y;
    asm volatile ("# Helper function body" : : : "memory");
    return v * 3;
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
    for (int i = 0; i < 5; i++) {
        int result = test_caller_save(iterations, i * 100);
        total += result;
        
        /* Also call regular functions to create call instructions */
        total += helper_func(i, result & 0xFF);
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
