/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Variables that will be in call-clobbered registers */
    register int r1 asm("rax") = vi + 1;
    register int r2 asm("rbx") = vi + 2;  /* rbx is callee-saved on x86-64 */
    register int r3 asm("rcx") = vi + 3;
    register double fd1 asm("xmm0") = vf + 1.0;
    register double fd2 asm("xmm1") = vf + 2.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < iterations; i = i + 1) {
        /* --- Basic block that will end with a call-like asm --- */
        
        /* Use integer registers in computation */
        int t1 = r1 * 2 + r2;
        int t2 = r3 ^ t1;
        
        /* Clobber rax, rcx (call-clobbered) - simulating a call */
        asm volatile ("# Clobber rax, rcx"
                      : /* no outputs */
                      : /* no inputs */
                      : "rax", "rcx", "memory");
        
        /* Use the values after clobber - forces save/restore */
        sum += t2 + r1;  /* r1 (rax) was clobbered, needs restore */
        
        /* --- Another basic block with different register types --- */
        
        /* Use floating point registers */
        double ft1 = fd1 * 2.0 + fd2;
        
        /* Clobber xmm0, xmm1 - simulating another call */
        asm volatile ("# Clobber xmm0, xmm1"
                      : /* no outputs */
                      : /* no inputs */
                      : "xmm0", "xmm1", "memory");
        
        /* Use after clobber */
        sum += (int)(ft1 + fd1);  /* fd1 (xmm0) was clobbered */
        
        /* --- Vector operations --- */
        
        /* Use vector registers */
        v4si vtmp = vec1 + (v4si){1, 2, 3, 4};
        int vecl = vtmp[0] + vtmp[2];
        
        /* Clobber xmm2-xmm5 (SSE call-clobbered) */
        asm volatile ("# Clobber xmm2-xmm5"
                      : /* no outputs */
                      : /* no inputs */
                      : "xmm2", "xmm3", "xmm4", "xmm5", "memory");
        
        /* Use vector after clobber */
        sum += vecl + vec1[0];
        
        /* --- Mixed use with conditional jump at block end --- */
        
        /* Create a value that needs to survive across asm */
        int mixed = (r2 * 3) + (int)fd2;
        
        /* Final asm that clobbers multiple register classes */
        /* This asm is at the end of a basic block */
        asm volatile ("# Final clobber at potential block end"
                      : /* no outputs */
                      : /* no inputs */
                      : "rax", "rcx", "rdx", "rsi", "rdi",
                        "xmm0", "xmm1", "xmm2", "xmm3",
                        "xmm4", "xmm5", "xmm6", "xmm7",
                        "memory");
        
        /* Conditional that creates control flow edge after the asm */
        if (mixed & 1) {
            /* This creates a basic block boundary */
            sum += mixed;
        } else {
            sum -= mixed;
        }
        
        /* Modify register variables to keep them live across iterations */
        r1 += sum & 0xFF;
        fd1 += (sum % 100) * 0.01;
        vec1[0] += sum;
    }
    
    return sum;
}

/* Another function to force actual calls */
static __attribute__((noinline))
int helper(int x) {
    volatile int y = x;
    asm volatile ("# Helper function clobber"
                  : /* no outputs */
                  : /* no inputs */
                  : "rax", "rcx", "rdx", "xmm0", "xmm1", "memory");
    return y * 2;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls to increase caller-save opportunities */
    for (int i = 0; i < 10; i++) {
        int result = test_caller_save(iterations, i * 100);
        total += result;
        
        /* Call another function between test calls */
        total += helper(result);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return total != 0 ? 0 : 1;
}
