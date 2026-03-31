/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;  /* Force memory operations */
    volatile float vf = seed * 1.5f;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Variables that will live across asm clobbers */
    register int r1 asm("rax") = vi + 1;
    register int r2 asm("rbx") = vi + 2;  /* rbx is callee-saved on x86-64, but we'll still use it */
    register int r3 asm("r12") = vi + 3;  /* r12 is callee-saved */
    volatile float f1 = vf + 1.0f;
    volatile double d1 = vf + 2.0;
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vecf = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use r1 in computation before clobbering */
        int temp1 = r1 * 2 + i;
        
        /* Clobber rax (call-clobbered) - simulates a function call */
        asm volatile ("# Clobber rax\n\t" 
                      : : : "rax", "memory");
        
        /* Use the value after clobber - forces save/restore */
        sum += temp1 - r1;
        
        /* ========== BLOCK 2: SSE register pressure ========== */
        /* Use vector before clobbering */
        v4si temp_vec = vec1 + (v4si){i, i, i, i};
        int vec_sum = temp_vec[0] + temp_vec[1];
        
        /* Clobber xmm0, xmm1 (call-clobbered SSE registers) */
        asm volatile ("# Clobber xmm0, xmm1\n\t"
                      : : : "xmm0", "xmm1", "memory");
        
        /* Use vector after clobber */
        sum += vec_sum + vec1[2];
        
        /* ========== BLOCK 3: Mixed register pressure ========== */
        /* Use float/double before clobbering */
        float ftemp = f1 * 2.0f + i;
        double dtemp = d1 * 3.0 + i;
        
        /* Clobber multiple registers including xmm2, xmm3 */
        asm volatile ("# Clobber xmm2, xmm3, rcx, rdx\n\t"
                      : : : "xmm2", "xmm3", "rcx", "rdx", "memory");
        
        /* Use float values after clobber */
        sum += (int)(ftemp + dtemp + f1);
        
        /* ========== BLOCK 4: Create basic block ending with asm ========== */
        /* This block should end with the asm statement */
        int block4_val = r2 * 3 + r3;
        
        /* This asm ends the basic block when followed by label */
        asm volatile ("# Block-ending clobber\n\t"
                      : : : "rax", "rbx", "r12", "xmm0", "xmm1", "xmm2", 
                            "xmm3", "xmm4", "xmm5", "memory");
        
        /* Label creates new basic block - forces BB_END update if asm was BB_END */
        if (block4_val > 1000) {
            /* This label creates control flow edge */
            sum += 1000;
        }
        
        /* ========== BLOCK 5: Another sequence with call-clobber ========== */
        /* Use floating vector */
        v4sf temp_vecf = vecf * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        sum += (int)(temp_vecf[0] + temp_vecf[1]);
        
        /* Final clobber of many registers */
        asm volatile ("# Massive clobber\n\t"
                      : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                            "memory");
        
        /* Use values after massive clobber - forces many saves/restores */
        sum += r1 + r2 + r3 + (int)f1 + (int)d1 + vec1[3];
        
        /* Modify variables to keep them live across iterations */
        r1 += i;
        r2 += i * 2;
        r3 += i * 3;
        f1 += 0.5f;
        d1 += 0.25;
        vec1 += (v4si){1, 2, 3, 4};
        vecf += (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x) {
    volatile int y = x * 2;
    /* Use asm to prevent optimization */
    asm volatile ("# Helper function body" : : : "memory");
    return y + 1;
}

/* Main function that creates multiple call sites */
int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments to create different
       register allocation patterns */
    for (int i = 0; i < 5; i++) {
        /* Call helper function - creates actual call instruction */
        int helper_result = helper_func(i);
        
        /* Call our test function - should trigger caller-save */
        int result = test_caller_save(iterations + i, i * 100 + helper_result);
        
        total += result;
        
        /* Another helper call between test calls */
        helper_result = helper_func(result % 100);
        total += helper_result;
    }
    
    printf("Result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return total != 0 ? 0 : 1;
}
