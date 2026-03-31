/* test_caller_save.c - Target GCC caller-save.cc lines 905-913 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int v1 = seed;  /* Force memory operations */
    volatile int v2 = seed * 2;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that will live across asm clobbers */
    register long r1 asm("rax") = v1 + 1;
    register long r2 asm("rbx") = v2 + 2;
    register long r3 asm("rcx") = v1 * 3;
    v4si vec1 = {v1, v2, v1 + 1, v2 + 1};
    v4sf vec2 = {v1 * 1.5f, v2 * 2.5f, v1 * 3.5f, v2 * 4.5f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use r1 in computation */
        long temp1 = r1 * 3 + i;
        
        /* Clobber rax - simulating a function call */
        asm volatile ("# Clobber rax\n\t" 
                      : : : "rax", "memory");
        
        /* Use original r1 value after clobber - forces save/restore */
        r1 = temp1 + r1;
        result += (int)r1;
        
        /* ========== BLOCK 2: Multiple register clobber ========== */
        /* Use r2 and r3 together */
        long temp2 = r2 + r3 * i;
        
        /* Clobber multiple integer registers */
        asm volatile ("# Clobber rbx, rcx\n\t"
                      : : : "rbx", "rcx", "memory");
        
        /* Use original values after clobber */
        r2 = temp2 ^ r2;
        r3 = r3 + i;
        result += (int)(r2 + r3);
        
        /* ========== BLOCK 3: SSE register pressure ========== */
        /* Vector computation */
        v4si vec_temp = vec1 + (v4si){i, i*2, i*3, i*4};
        
        /* Clobber xmm0-xmm2 */
        asm volatile ("# Clobber xmm0, xmm1, xmm2\n\t"
                      : : : "xmm0", "xmm1", "xmm2", "memory");
        
        /* Use original vector after clobber */
        vec1 = vec_temp * vec1;
        result += vec1[0] + vec1[2];
        
        /* ========== BLOCK 4: Mixed register types ========== */
        /* Use floating vector */
        v4sf vec_temp2 = vec2 * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
        
        /* Clobber more SSE and MMX registers */
        asm volatile ("# Clobber xmm3, xmm4, mm0\n\t"
                      : : : "xmm3", "xmm4", "mm0", "memory");
        
        /* Use original floating vector */
        vec2 = vec_temp2 + vec2;
        result += (int)vec2[1];
        
        /* ========== BLOCK 5: Create basic block ending with call ========== */
        /* Force a control flow edge after asm */
        if (i & 1) {
            /* Use volatile to prevent optimization */
            volatile int branch_var = result;
            
            /* Another clobber - this could be at block end */
            asm volatile ("# Potential block-end clobber\n\t"
                          : : : "rax", "rbx", "rcx", "rdx", 
                                "xmm0", "xmm1", "xmm5", "memory");
            
            /* Label to create control flow edge */
            if (branch_var > 1000) {
                /* This creates a jump target after the asm */
                result = result / 2;
            }
        }
        
        /* Modify volatile variables to extend liveness */
        v1 = result % 100;
        v2 = v1 * 2;
    }
    
    return result;
}

/* Another non-inlineable function to create actual calls */
static __attribute__((noinline, optimize("O0")))
int helper_function(int x) {
    /* Empty function that gets called */
    return x + 1;
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
        
        /* Call helper to create actual function calls in main */
        total = helper_function(total);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
