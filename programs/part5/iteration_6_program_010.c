/* test_caller_save.c - Target GCC's caller-save pass insertion logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Volatile variables to extend register liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile float vf1 = seed * 0.5f;
    volatile float vf2 = seed * 1.5f;
    
    /* Variables that will be forced into call-clobbered registers */
    int a, b, c, d;
    float fa, fb, fc;
    v4si vec_int;
    v4sf vec_float;
    v2di vec_mmx;
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ====== PHASE 1: Integer registers ====== */
        /* Compute values in call-clobbered registers */
        a = vi1 * 3 + i;
        b = vi2 * 5 - i;
        c = a ^ b;
        d = (c << 3) | (c >> 29);
        
        /* Simulate a call that clobbers integer registers */
        /* Clobber rax, rbx, rcx, rdx, rsi, rdi on x86-64 */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0x55555555, %%rcx\n\t"
            "mov $0xAAAAAAAA, %%rdx\n\t"
            "mov $0x11111111, %%rsi\n\t"
            "mov $0x22222222, %%rdi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11" /* more clobbers */
        );
        
        /* Use the values after clobber - forces save/restore */
        result += a + b - c + d;
        
        /* ====== PHASE 2: Floating-point/SSE registers ====== */
        /* Compute in XMM registers */
        fa = vf1 * 2.0f + i;
        fb = vf2 * 3.0f - i;
        fc = fa * fb - fa / (fb + 1.0f);
        
        /* Vector computations */
        vec_int = (v4si){a, b, c, d};
        vec_float = (v4sf){fa, fb, fc, fa + fb};
        
        /* Simulate call clobbering SSE/MMX registers */
        /* Clobber xmm0-xmm5, mm0-mm2 */
        asm volatile (
            "# CLOBBER SSE/MMX REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
              "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
        );
        
        /* Use vector values after clobber */
        vec_int += (v4si){1, 2, 3, 4};
        vec_float *= (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Extract results */
        result += vec_int[0] + vec_int[1];
        result += (int)(vec_float[0] + vec_float[1]);
        
        /* ====== PHASE 3: Mixed computations with conditional ====== */
        /* Create a basic block ending with clobber */
        if (i % 3 == 0) {
            /* More computations in call-clobbered registers */
            int x = a * b + i;
            int y = c * d - i;
            float z = fa * fb + i;
            
            /* Another clobbering asm - this could be at block end */
            asm volatile (
                "# CLOBBER MIXED REGS\n\t"
                "mov $0x33333333, %%r8\n\t"
                "mov $0x44444444, %%r9\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "r10", "r11",
                  "xmm6", "xmm7", "xmm8", "xmm9"
            );
            
            /* Label to create control flow edge after asm */
            /* This helps create a basic block ending with the asm */
            if (x > y) {
                result += x - y + (int)z;
            } else {
                result += y - x - (int)z;
            }
        } else if (i % 3 == 1) {
            /* Alternative path with different clobber pattern */
            asm volatile (
                "# ALTERNATIVE CLOBBER\n\t"
                "mov $0x55555555, %%r10\n\t"
                "mov $0x66666666, %%r11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r10", "r11", "r12", "r13", "r14", "r15"
            );
            result += i * 7;
        }
        
        /* Update volatiles to prevent optimization */
        vi1 += i;
        vi2 -= i;
        vf1 += i * 0.1f;
        vf2 -= i * 0.1f;
    }
    
    return result;
}

/* Another non-inlineable function to force actual calls */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int v = x * y;
    asm volatile ("# HELPER FUNCTION\n\t"
                  : /* no outputs */
                  : /* no inputs */
                  : "rax", "rbx", "rcx");
    return v + (x ^ y);
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int s = 0; s < 5; s++) {
        int res = test_caller_save(iterations, s * 100);
        total += res;
        
        /* Call another function between test calls */
        /* This creates more opportunities for caller-save */
        int helper_res = helper_func(res, s);
        total ^= helper_res;
    }
    
    /* Also test with small iteration counts */
    for (int s = 5; s < 10; s++) {
        total += test_caller_save(3, s * 50);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
