/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Mixed register type computations */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile double vd1 = seed * 0.5;
    volatile double vd2 = seed * 1.5;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    volatile v4si vec_int = {seed, seed + 1, seed + 2, seed + 3};
    volatile v4sf vec_float = {seed * 0.1f, seed * 0.2f, seed * 0.3f, seed * 0.4f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ====== BLOCK 1: Integer register pressure ====== */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 5 - i;
        
        /* Force caller-save for rax, rbx, rcx, rdx */
        asm volatile (
            "# Clobber integer regs\n\t"
            : 
            : "a"(r1), "b"(r2), "c"(i), "d"(vi1)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        result += r1 + r2;
        
        /* ====== BLOCK 2: Floating point pressure ====== */
        double d1 = vd1 * i;
        double d2 = vd2 / (i + 1);
        
        /* Force caller-save for xmm0-xmm5 */
        asm volatile (
            "# Clobber xmm regs\n\t"
            : 
            : "x"(d1), "x"(d2)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        result += (int)(d1 + d2);
        
        /* ====== BLOCK 3: Vector register pressure ====== */
        v4si v1 = vec_int * i;
        v4sf v2 = vec_float + (float)i;
        
        /* Force caller-save for vector registers */
        asm volatile (
            "# Clobber vector regs\n\t"
            : 
            : "x"(v1), "x"(v2)
            : "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "memory"
        );
        
        /* Use vector results */
        int sum = 0;
        for (int j = 0; j < 4; j++) {
            sum += v1[j] + (int)v2[j];
        }
        result += sum;
        
        /* ====== BLOCK 4: Mixed register pressure with call ====== */
        /* Create a basic block ending with asm that looks like a call */
        int temp = result * 7;
        
        /* This asm simulates a function call that clobbers many registers */
        asm volatile (
            "# Simulate call clobber\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : 
            : "r"(temp)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* Label to create control flow edge after the "call" */
        if (temp > 1000) {
            /* This creates a basic block boundary */
            result = result / 2;
        }
        
        /* Force spill around another asm block */
        int r3 = vi1 + vi2 + i;
        asm volatile (
            "# Another clobber point\n\t"
            : 
            : "r"(r3)
            : "rax", "rbx", "memory"
        );
        
        result ^= r3;
        
        /* Modify volatiles to extend liveness */
        vi1 += i;
        vi2 -= i;
        vd1 *= 1.01;
        vd2 /= 1.01;
    }
    
    return result;
}

/* External function to force actual calls */
static __attribute__((noinline)) 
int helper(int a, int b) {
    return a * b + 1;
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
    total += test_caller_save(iterations / 2, 2);
    total += test_caller_save(iterations * 2, 3);
    
    /* Mix with actual function calls */
    for (int i = 0; i < 10; i++) {
        total += helper(i, total);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
