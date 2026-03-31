/* test_caller_save.c - Target GCC's caller-save optimization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Live values in call-clobbered registers */
    int r1 = vi + 1;                 /* Will use rax/rbx/etc */
    float f1 = vf * 2.0f;            /* Will use xmm0/xmm1 */
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ===== BLOCK 1: Integer register pressure ===== */
        int r2 = r1 * 3 + i;         /* Keep r1 live across asm */
        
        /* Clobber multiple integer registers (call-clobbered on x86-64) */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0x55555555, %%rcx\n\t"
            "mov $0xAAAAAAAA, %%rdx\n\t"
            "add $1, %%rax\n\t"
            "add %%rbx, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use r1 after clobber - forces save/restore */
        sum += r1 + r2;
        
        /* ===== BLOCK 2: SSE/vector register pressure ===== */
        v4si vec3 = vec1 + (v4si){i, i, i, i};
        v4sf vec4 = vec2 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use vector values after clobber */
        for (int j = 0; j < 4; j++) {
            sum += vec3[j];
            sum += (int)vec4[j];
        }
        
        /* ===== BLOCK 3: Mixed register pressure with call ===== */
        /* Create a basic block ending with value usage */
        float f2 = f1 * (i + 1);
        
        /* Simulate function call that clobbers registers */
        asm volatile (
            "# SIMULATE FUNCTION CALL\n\t"
            "call dummy_label%=\n\t"
            "dummy_label%=:\n\t"
            "pop %%rax\n\t"          /* Clobber rax */
            "pxor %%xmm0, %%xmm0\n\t" /* Clobber xmm0 */
            : /* no outputs */
            : /* no inputs */
            : "rax", "xmm0", "memory"
        );
        
        /* Use f1 after "call" - forces save before call, restore after */
        sum += (int)(f1 + f2);
        
        /* Force control flow edge after the asm */
        if (sum & 1) {
            vi++;  /* Side effect to prevent optimization */
        }
        
        /* Update live values for next iteration */
        r1 = sum & 0xFF;
        f1 = (sum % 100) * 0.01f;
        vec1 = vec1 + (v4si){1, 2, 3, 4};
        vec2 = vec2 * (v4sf){0.9f, 0.95f, 1.0f, 1.05f};
    }
    
    return sum;
}

/* External function to force real calls */
static __attribute__((noinline)) 
int helper(int x, int y) {
    return x * y + (x ^ y);
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(iterations, 1);
    total += test_caller_save(iterations / 2, 42);
    total += test_caller_save(iterations * 2, 123);
    
    /* Insert real function calls between test calls */
    total += helper(total, iterations);
    total += test_caller_save(10, total & 0xFF);
    total += helper(total, total);
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
