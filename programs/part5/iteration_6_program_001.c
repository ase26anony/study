/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
#define NOINLINE __attribute__((noinline))

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

/* Non-inlineable helper that clobbers registers */
NOINLINE void clobber_all(void) {
    /* Empty function that compiler can't analyze */
    asm volatile ("");
}

/* Function that forces caller-save register usage */
NOINLINE static int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Prevent optimizations */
    volatile float vf = seed * 1.5f;
    volatile v4si vvec_i;
    volatile v4sf vvec_f;
    volatile mmx_t vmmx;
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ===== INTEGER REGISTERS ===== */
        /* Use multiple call-clobbered integer registers */
        int a = vi + i * 3;
        int b = vi * 2 - i;
        int c = vi ^ (i << 2);
        int d = vi | (i * 7);
        
        /* Clobber rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use values after clobber (forces save/restore) */
        result += a - b + c * d;
        
        /* ===== FLOATING POINT REGISTERS ===== */
        float f1 = vf + i * 0.5f;
        float f2 = vf * 2.0f - i;
        double d1 = (double)vf * 1.25;
        double d2 = (double)vf / (i + 1);
        
        /* Clobber xmm0-xmm5 (call-clobbered on x86-64) */
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
        
        /* Use FP values after clobber */
        result += (int)(f1 * 100.0f) - (int)(f2 * 50.0f);
        result += (int)(d1 * 10.0) + (int)(d2 * 20.0);
        
        /* ===== VECTOR REGISTERS ===== */
        v4si vec1 = {vi + i, vi - i, vi * i, vi ^ i};
        v4si vec2 = {i * 2, i * 3, i * 4, i * 5};
        v4si vec3 = vec1 + vec2;
        v4si vec4 = vec1 * vec2;
        
        /* Clobber more xmm registers */
        asm volatile (
            "# Clobber more vector regs\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm6", "xmm7", "xmm8", "xmm9", "memory"
        );
        
        /* Use vector results */
        vvec_i = vec3 + vec4;
        for (int j = 0; j < 4; j++) {
            result += vvec_i[j];
        }
        
        /* ===== MMX REGISTERS ===== */
        mmx_t mm1 = {vi * 3LL + i};
        mmx_t mm2 = {vi * 5LL - i};
        mmx_t mm3;
        
        /* Clobber MMX registers */
        asm volatile (
            "# Clobber MMX regs\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "mm1", "mm2", "memory"
        );
        
        /* Use MMX values (emulated with emms) */
        vmmx = mm1;
        result += (int)vmmx[0];
        
        /* ===== FUNCTION CALL AT BLOCK END ===== */
        /* Call that forces save/restore insertion at block end */
        clobber_all();
        
        /* Label to create basic block boundary */
        if (i & 1) {
            /* Conditional jump to create control flow */
            goto skip;
        }
        
        /* More computations to extend basic block */
        result ^= (vi << 3);
        
    skip:
        /* Empty target for goto */
        result += i;
        
        /* Another clobbering asm at potential block end */
        asm volatile (
            "# Another clobber\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r8", "r9", "r10", "memory"
        );
        
        /* Use volatile to prevent dead code elimination */
        vi = result & 0xFFF;
        vf = (float)(result % 100) * 0.01f;
    }
    
    return result;
}

/* Another function to create more call sites */
NOINLINE static int intermediate(int x, int y) {
    volatile int v = x;
    
    /* Force register usage across calls */
    int a = v * 3;
    int b = v + y;
    
    /* Call that clobbers registers */
    asm volatile (
        "# Intermediate clobber\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r11", "r12", "memory"
    );
    
    /* Use values after clobber */
    return a * b + test_caller_save(2, v);
}

int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
        if (iterations > 100) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    total += test_caller_save(iterations, 1);
    total += test_caller_save(iterations / 2, 42);
    total += intermediate(5, 7);
    total += intermediate(13, 17);
    
    /* Force use of result */
    volatile int print_me = total;
    printf("Result: %d\n", print_me);
    
    return print_me & 0xFF;
}
