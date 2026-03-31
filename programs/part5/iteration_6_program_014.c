/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Variables that will live across asm clobbers */
    register long r1 asm("rax") = vi + 1;
    register long r2 asm("rbx") = vi + 2;
    register double d1 asm("xmm0") = vf + 1.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ===== BLOCK 1: Integer register pressure ===== */
        /* Use r1 in computation before clobber */
        int temp1 = (r1 * 3) / 2;
        
        /* Clobber rax - simulating a function call */
        asm volatile (
            "# Clobber rax\n\t"
            "mov $0x12345678, %%rax\n\t"
            "add $1, %%rax\n\t"
            : 
            : 
            : "rax", "cc"
        );
        
        /* Use r1 after clobber - forces save/restore */
        sum += temp1 + (r1 & 0xFF);
        
        /* ===== BLOCK 2: SSE register pressure ===== */
        /* Use xmm0 in computation */
        double temp2 = d1 * 2.5;
        
        /* Clobber xmm0 and xmm1 */
        asm volatile (
            "# Clobber xmm registers\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            : 
            : 
            : "xmm0", "xmm1", "cc"
        );
        
        /* Use d1 after clobber */
        sum += (int)(temp2 + d1);
        
        /* ===== BLOCK 3: Mixed register pressure ===== */
        /* Use rbx and vector registers */
        int temp3 = (r2 * vec1[0]) / 7;
        
        /* Clobber multiple call-clobbered registers */
        asm volatile (
            "# Clobber multiple registers\n\t"
            "mov $0x9ABCDEF0, %%rbx\n\t"
            "mov $0x55555555, %%r10\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : 
            : 
            : "rbx", "r10", "r11", "xmm2", "xmm3", "cc"
        );
        
        /* Use r2 and vec1 after clobber */
        sum += temp3 + (r2 % 31) + vec1[1];
        
        /* ===== BLOCK 4: Vector operations ===== */
        /* Vector computation */
        vec2 = vec2 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber more registers including MMX */
        asm volatile (
            "# Clobber MMX and more\n\t"
            "emms\n\t"  /* Empty MMX state */
            "mov $0x77777777, %%r12\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : 
            : 
            : "r12", "r13", "xmm4", "xmm5", "mm0", "mm1", "cc"
        );
        
        /* Use vec2 after clobber - this may force spill/fill */
        sum += (int)vec2[0] + (int)vec2[1];
        
        /* ===== BLOCK 5: Create basic block ending with call ===== */
        /* Use volatile to prevent optimization */
        vi = vi * 1103515245 + 12345;
        
        /* This asm looks like a call and ends a basic block */
        /* The following if statement creates control flow */
        if (vi & 1) {
            /* Clobber critical registers at block end */
            asm volatile (
                "# Final clobber at potential block end\n\t"
                "mov $0xFFFFFFFF, %%rax\n\t"
                "mov $0xAAAAAAAA, %%rbx\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                : 
                : 
                : "rax", "rbx", "xmm0", "xmm1", "cc"
            );
            /* Label to potentially end basic block */
            /* The caller-save may insert after the asm */
            sum += 1000;
        } else {
            sum += 2000;
        }
        
        /* Update variables to keep them live */
        r1 = (r1 * 3 + i) & 0xFFFF;
        r2 = (r2 * 5 + i) & 0xFFFF;
        d1 = d1 * 1.1 + i * 0.01;
        vec1[0] = (vec1[0] + i) & 0xFF;
        vec2[0] = vec2[0] + i * 0.1f;
    }
    
    return sum;
}

/* Another function to create actual call sites */
static __attribute__((noinline))
int helper_func(int x) {
    volatile int y = x;
    asm volatile (
        "# Helper function clobber\n\t"
        "add $1, %0\n\t"
        : "+r" (y)
        :
        : "cc"
    );
    return y * 2;
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
    total += test_caller_save(iterations / 4, 123);
    
    /* Call another function to create more caller-save contexts */
    for (int i = 0; i < 10; i++) {
        total += helper_func(i);
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
