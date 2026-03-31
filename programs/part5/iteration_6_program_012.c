/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Variables that must survive across asm clobbers */
    register long r1 asm("rax") = vi + 1;
    register long r2 asm("rbx") = vi + 2;  /* rbx is normally callee-saved on x86-64,
                                            but we'll treat it as clobbered via asm */
    register double fd1 asm("xmm0") = vf + 1.0;
    register double fd2 asm("xmm1") = vf + 2.0;
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < iterations; i++) {
        /* === BLOCK 1: Integer register pressure === */
        /* Use r1 in computation before clobber */
        int temp1 = (r1 * 3) / 2;
        
        /* Clobber rax - simulating a function call */
        asm volatile (
            "movq $0x12345678, %%rax\n\t"  /* Clobber rax */
            "addq $0x1111, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "cc"  /* Clobber rax and condition codes */
        );
        
        /* Use original value after clobber - forces save/restore */
        sum += temp1 + (r1 & 0xFF);
        
        /* === BLOCK 2: SSE register pressure === */
        /* Use xmm0 before clobbering */
        double dtemp = fd1 * 2.5;
        
        /* Clobber xmm0 and xmm1 */
        asm volatile (
            "xorpd %%xmm0, %%xmm0\n\t"
            "xorpd %%xmm1, %%xmm1\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "cc"
        );
        
        /* Use original values - forces save/restore */
        sum += (int)(dtemp + fd1 + fd2);
        
        /* === BLOCK 3: Vector register pressure === */
        /* Use vector values */
        vec_int += (v4si){1, 2, 3, 4};
        vec_float *= (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Clobber multiple vector registers */
        asm volatile (
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm2", "xmm3", "xmm4", "cc"
        );
        
        /* Use vectors after clobber */
        sum += vec_int[0] + vec_int[2];
        sum += (int)vec_float[1];
        
        /* === BLOCK 4: Mixed register pressure with call-like asm at block end === */
        /* This is designed to create a basic block ending with a clobbering asm */
        r2 = r2 * 2 + vi;
        
        /* Final clobbering asm at potential block end */
        /* This asm is placed right before a label/jump */
        asm volatile (
            "movq $0xABCD, %%rax\n\t"
            "movq $0xEF01, %%rbx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "cc"
        );
        
        /* Force control flow - creates block boundary */
        if (i & 1) {
            /* Use label to create potential block end */
            sum += 1000;
        } else {
            sum += 2000;
        }
        
        /* Re-initialize some values for next iteration */
        r1 = sum % 100;
        fd1 = (sum % 100) * 0.5;
        vi++;  /* Modify volatile to prevent loop optimizations */
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    /* Use asm to clobber registers */
    asm volatile (
        "movl $0xDEADBEEF, %%eax\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "cc"
    );
    return x * y + 1;
}

/* Main driver that creates multiple call sites */
int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < 5; i++) {
        int result = test_caller_save(iterations + i, i * 100);
        total += result;
        
        /* Call another function between test_caller_save calls */
        /* This creates more opportunities for caller-save */
        int helper_result = helper_func(i, iterations);
        total += helper_result;
        
        /* Use asm to clobber registers between function calls */
        asm volatile (
            "movq $0x1234, %%rax\n\t"
            "movq $0x5678, %%rbx\n\t"
            "xorpd %%xmm0, %%xmm0\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "xmm0", "cc"
        );
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Additional test with more register pressure */
    volatile double d1 = 1.23456;
    volatile double d2 = 7.89012;
    register double xmm2_val asm("xmm2") = d1;
    register double xmm3_val asm("xmm3") = d2;
    
    /* Sequence of asm clobbers with computations in between */
    for (int j = 0; j < 3; j++) {
        double temp = xmm2_val * xmm3_val;
        
        asm volatile (
            "xorpd %%xmm2, %%xmm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm2", "cc"
        );
        
        total += (int)(temp * 100);
        
        xmm2_val = d1 + j;
        xmm3_val = d2 - j;
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
