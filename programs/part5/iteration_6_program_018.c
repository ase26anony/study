/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector type using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that must stay live across clobbering asm statements */
    register long r1 asm("rax") = vi + 1;
    register long r2 asm("rbx") = vi + 2;
    register long r3 asm("rcx") = vi + 3;
    register double fd1 asm("xmm0") = vf + 1.0;
    register double fd2 asm("xmm1") = vf + 2.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore code */
    for (volatile int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use integer registers in computation */
        r1 = r1 * 1103515245 + 12345;
        r2 = r2 * 1103515245 + 12345;
        r3 = r3 * 1103515245 + 12345;
        
        /* Clobber integer call-clobbered registers - simulates a function call */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "movq $0x12345678, %%rax\n\t"
            "movq $0x87654321, %%rbx\n\t"
            "movq $0xABCDEF01, %%rcx\n\t"
            "addq $1, %%rdx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the original values after clobber - forces save/restore */
        sum += (r1 & 0xFF) + (r2 & 0xFF) + (r3 & 0xFF);
        
        /* ========== BLOCK 2: Floating-point register pressure ========== */
        /* Use floating-point registers */
        fd1 = fd1 * 1.1 + 0.5;
        fd2 = fd2 * 1.2 - 0.3;
        
        /* Clobber floating-point/vector registers */
        asm volatile (
            "# CLOBBER FP/SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Use original FP values */
        sum += (int)(fd1 * 100) + (int)(fd2 * 100);
        
        /* ========== BLOCK 3: Vector register pressure ========== */
        /* Vector operations */
        vec1 = vec1 + (v4si){1, 2, 3, 4};
        vec2 = vec2 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber MMX/vector registers */
        asm volatile (
            "# CLOBBER MMX/ADDITIONAL REGS\n\t"
            "emms\n\t"                    /* Empty MMX state */
            "pxor %%mm0, %%mm0\n\t"
            "movq $0, %%mm1\n\t"
            :
            :
            : "mm0", "mm1", "memory"
        );
        
        /* Use vector results */
        sum += vec1[0] + vec1[1] + (int)vec2[2];
        
        /* ========== BLOCK 4: Mixed register class usage ========== */
        /* Interleave usage of different register classes */
        r1 = r1 ^ vec1[0];
        fd1 = fd1 + vec2[0];
        
        /* Final clobber that might be at block end */
        asm volatile (
            "# FINAL CLOBBER AT POTENTIAL BLOCK END\n\t"
            "movq $0, %%r8\n\t"
            "movq $0, %%r9\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            :
            :
            : "r8", "r9", "xmm3", "memory"
        );
        
        /* Conditional that creates control flow edge after clobber */
        if (sum & 1) {
            /* Use clobbered values again */
            sum += (r1 & 1) ? 1 : 0;
            sum += (fd1 > 0) ? 1 : 0;
        }
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    /* This function clobbers registers */
    asm volatile (
        "# HELPER FUNCTION CLOBBERS\n\t"
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        : 
        : 
        : "rax", "rbx", "xmm0", "memory"
    );
    return x * y + 1;
}

/* Main test driver */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < 10; i++) {
        /* Call to helper creates register pressure */
        int tmp = helper_func(i, iterations);
        
        /* Main test - this should trigger caller-save insertions */
        int result = test_caller_save(iterations, i * 100 + tmp);
        
        total += result;
        
        /* Another helper call between test calls */
        tmp = helper_func(result & 0xFF, i);
        total += tmp;
    }
    
    printf("Result checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0x1234) {  /* Unlikely */
        printf("Impossible!\n");
    }
    
    return total & 0xFF;
}
