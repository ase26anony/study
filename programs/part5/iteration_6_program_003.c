/* test_caller_save.c - Target for GCC caller-save.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed);

/* Non-inlineable helper to force actual calls */
static __attribute__((noinline, noipa))
void clobber_helper(void) {
    /* Empty but prevents optimization */
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* MMX type */
typedef long long mmx_t __attribute__((vector_size(8)));

int main(void) {
    int result = 0;
    volatile int iter = 5; /* Volatile to prevent constant propagation */
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        result += test_caller_save(iter + i, i * 100);
    }
    
    printf("Result checksum: %d\n", result);
    return 0;
}

static __attribute__((noinline))
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;      /* Force memory traffic */
    volatile float vf = seed * 0.5f;
    
    /* Variables in call-clobbered registers */
    int a = vi + 1;
    int b = vi + 2;
    int c = vi + 3;
    float f1 = vf + 1.0f;
    float f2 = vf + 2.0f;
    
    /* Vector variables */
    v4si vec1 = {a, b, c, vi};
    v4si vec2 = {b, c, a, vi + 1};
    v4sf fvec1 = {f1, f2, f1 + 1.0f, f2 + 1.0f};
    
    /* MMX variable */
    mmx_t mmx_val = {seed * 2LL};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < iterations; i++) {
        /* === BLOCK 1: Integer register pressure === */
        int r1 = a * b + c;
        int r2 = b * c + a;
        
        /* Clobber multiple integer registers - simulating a call */
        asm volatile(
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
        );
        
        /* Use values after clobber - forces save/restore */
        sum += r1 + r2;
        a = sum + i;
        b = sum + i + 1;
        
        /* === BLOCK 2: SSE register pressure === */
        v4si vec3 = vec1 + vec2;
        v4sf fvec2 = fvec1 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
        
        /* Call-like instruction that ends basic block */
        clobber_helper();  /* This ends a BB, next insn is label */
        
        /* Label to create control flow edge after call */
        if (i & 1) {
            /* Clobber SSE registers */
            asm volatile(
                "# CLOBBER SSE REGS\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                : 
                : 
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* Use vector values after clobber */
            vec1 = vec3 + (v4si){i, i, i, i};
            fvec1 = fvec2 + (v4sf){vf, vf, vf, vf};
            sum += vec1[0] + (int)fvec1[0];
        }
        
        /* === BLOCK 3: Mixed register pressure === */
        /* Compute with current values */
        int r3 = a * 3 + b * 5;
        float f3 = f1 * 1.5f + f2 * 2.5f;
        
        /* Another clobbering asm that could be at BB end */
        asm volatile(
            "# CLOBBER MIXED REGS\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : 
            : 
            : "r8", "r9", "xmm4", "xmm5", "memory"
        );
        
        /* Conditional jump creates BB boundary */
        if (r3 > 100) {
            /* Clobber MMX register */
            asm volatile(
                "# CLOBBER MMX REG\n\t"
                "pxor %%mm0, %%mm0\n\t"
                : 
                : 
                : "mm0", "memory"
            );
            
            mmx_val = (mmx_t){r3 * 2LL};
            sum += (int)mmx_val;
        }
        
        /* Use all values to keep them live */
        f1 = f3 + (float)i;
        f2 = f1 * 2.0f;
        c = r3 + i;
        
        /* Another call at potential BB end */
        if (i % 2 == 0) {
            clobber_helper();
            /* This creates a scenario where save might be inserted
               after a call that's at BB end */
        }
    }
    
    /* Final computation using all variables */
    sum += a + b + c + (int)f1 + (int)f2 + vec1[0] + (int)fvec1[0];
    
    return sum;
}
