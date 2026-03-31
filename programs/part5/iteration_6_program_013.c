/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    volatile int vi = seed;          /* Force memory traffic */
    volatile float vf = seed * 1.5f;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* MMX type */
    typedef long long v1di __attribute__((vector_size(8)));
    
    /* Live values in call-clobbered registers */
    register long rax_val asm("rax") = vi + 1;
    register long rbx_val asm("rbx") = vi + 2;
    register long rcx_val asm("rcx") = vi + 3;
    v4si vec_int = {vi, vi+1, vi+2, vi+3};
    v4sf vec_float = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    v1di mmx_val = {vi * 2LL};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== BLOCK 1: Integer register pressure ========== */
        /* Use integer values before clobbering */
        int temp1 = (rax_val * rbx_val) ^ rcx_val;
        
        /* Clobber integer registers - simulates a function call */
        asm volatile(
            "# CLOBBER INTEGER REGS\n\t"
            "movq $0x12345678, %%rax\n\t"
            "movq $0x87654321, %%rbx\n\t"
            "movq $0xABCDEF01, %%rcx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Use original values after clobber - forces save/restore */
        result += temp1 + (rax_val & 0xFF) + (rbx_val % 256) + (rcx_val >> 8);
        
        /* ========== BLOCK 2: SSE register pressure ========== */
        /* Compute with vector values */
        v4si vec_temp = vec_int + (v4si){i, i*2, i*3, i*4};
        int sum_vec = vec_temp[0] + vec_temp[1] + vec_temp[2] + vec_temp[3];
        
        /* Clobber SSE registers */
        asm volatile(
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Use original vector after clobber */
        result += sum_vec + vec_int[0] + vec_int[3];
        
        /* ========== BLOCK 3: Mixed register pressure ========== */
        /* Use float vector */
        v4sf float_temp = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        int float_sum = (int)(float_temp[0] + float_temp[1]);
        
        /* Clobber more registers including MMX */
        asm volatile(
            "# CLOBBER MIXED REGS\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%mm0, %%mm0\n\t"
            :
            :
            : "xmm3", "xmm4", "mm0", "memory"
        );
        
        /* Use original values - forces more save/restore */
        result += float_sum + (int)vec_float[0] + (int)(mmx_val[0] & 0xFF);
        
        /* ========== CRITICAL: Create basic block ending with call-like asm ========== */
        /* This asm simulates a function call that ends a basic block */
        int call_result;
        asm volatile(
            "# FUNCTION CALL SIMULATION\n\t"
            "movl %1, %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (call_result)
            : "r" (result)
            : "rax", "cc", "memory"
        );
        
        /* IMPORTANT: Conditional jump right after "call" to create block boundary */
        if (call_result > 1000) {
            /* This creates a control flow edge after the asm "call" */
            result = call_result / 2;
        } else {
            result = call_result * 2;
        }
        
        /* Modify live values to keep them active across iterations */
        rax_val += i;
        rbx_val ^= i;
        vec_int += (v4si){1, 2, 3, 4};
        vec_float *= (v4sf){1.01f, 1.01f, 1.01f, 1.01f};
        mmx_val[0] += i * 3LL;
        
        /* Volatile access to prevent optimization */
        vi++;
        vf += 0.5f;
    }
    
    return result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int z = x + y;
    asm volatile("# HELPER FUNCTION BODY" : : : "memory");
    return z * 2;
}

/* Main driver that creates multiple call sites */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < 10; i++) {
        /* Call to test_caller_save - creates caller-save context */
        int r1 = test_caller_save(iterations, i * 100);
        
        /* Call to another function between test calls */
        int r2 = helper_func(r1, i);
        
        /* Use results to prevent dead code elimination */
        total += r1 + r2;
        
        /* Additional register pressure in main */
        register long r10_val asm("r10") = r1;
        register long r11_val asm("r11") = r2;
        
        asm volatile(
            "# MAIN LOOP CLOBBER\n\t"
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            :
            :
            : "r10", "r11", "memory"
        );
        
        total += (int)(r10_val & 0xFFFF) + (int)(r11_val & 0xFFFF);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
