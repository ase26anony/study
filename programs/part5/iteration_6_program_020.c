/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
    
    /* Live values in call-clobbered registers across multiple asm blocks */
    int r1 = vi + 1;      /* Will use rax/rbx */
    int r2 = vi * 2;      /* Will use rcx/rdx */
    float f1 = vf + 1.0f; /* Will use xmm0-xmm5 */
    v4si vec1 = {vi, vi+1, vi+2, vi+3};
    v4sf vec2 = {vf, vf+1.0f, vf+2.0f, vf+3.0f};
    
    int sum = 0;
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ---- BLOCK 1: Integer register pressure ---- */
        /* Compute with integer registers */
        int temp1 = r1 * r2 + i;
        float temp2 = f1 * i;
        
        /* Clobber multiple integer registers (simulating a call) */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x9ABCDEF0, %%rbx\n\t"
            "mov $0x11111111, %%rcx\n\t"
            "mov $0x22222222, %%rdx\n\t"
            "add %%rbx, %%rax\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber (forces save/restore) */
        sum += temp1 + (int)temp2;
        r1 = sum % 100;
        r2 = (sum + i) % 100;
        
        /* ---- BLOCK 2: SSE register pressure ---- */
        /* Vector computations using SSE registers */
        v4si vec3 = vec1 + (v4si){i, i*2, i*3, i*4};
        v4sf vec4 = vec2 + (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
        
        /* Clobber SSE registers */
        asm volatile (
            "# CLOBBER SSE REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vector results after clobber */
        vec1 = vec3 + (v4si){sum, sum, sum, sum};
        vec2 = vec4 * (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
        
        /* Extract elements to force materialization */
        sum += vec1[0] + vec1[1];
        sum += (int)vec2[0];
        
        /* ---- BLOCK 3: Mixed register pressure with conditional jump ---- */
        /* Create a basic block ending with clobber */
        if (i % 3 == 0) {
            /* More computations */
            double d1 = (double)sum / 100.0;
            double d2 = d1 * 3.14159;
            
            /* Clobber more registers including xmm for doubles */
            asm volatile (
                "# CLOBBER MIXED REGS\n\t"
                "mov $0x33333333, %%r8\n\t"
                "mov $0x44444444, %%r9\n\t"
                "mov $0x55555555, %%r10\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                :
                :
                : "r8", "r9", "r10", "xmm6", "xmm7", "memory"
            );
            
            /* Use after clobber - this creates need for save AFTER the asm */
            sum += (int)(d1 + d2);
            
            /* Label to create control flow edge after the asm */
            after_asm:
            /* This jump creates a basic block ending with the asm above */
            if (sum > 1000) {
                sum = sum % 1000;
            }
        } else if (i % 3 == 1) {
            /* Alternative path with MMX registers */
            typedef long long v1di __attribute__((vector_size(8)));
            v1di mmx1 = {sum * 2LL};
            v1di mmx2 = {sum * 3LL};
            
            /* Clobber MMX registers */
            asm volatile (
                "# CLOBBER MMX REGS\n\t"
                "pxor %%mm0, %%mm0\n\t"
                "pxor %%mm1, %%mm1\n\t"
                "pxor %%mm2, %%mm2\n\t"
                :
                :
                : "mm0", "mm1", "mm2", "memory"
            );
            
            /* Use MMX values after clobber */
            v1di mmx3 = mmx1 + mmx2;
            sum += (int)mmx3[0];
        }
        
        /* Force spill by using all live values */
        vi = sum;
        vf = (float)sum;
    }
    
    return sum;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int z = x + y;
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
        int result = test_caller_save(iterations, i * 100);
        total += result;
        
        /* Call another function between test_caller_save calls */
        /* This creates more opportunities for caller-save around actual calls */
        int helper_result = helper_func(i, result);
        total += helper_result;
        
        /* Inline asm that clobbers registers around a real function call */
        int pre_asm = total * 2;
        asm volatile (
            "# PRE-CALL CLOBBER\n\t"
            "mov $0x66666666, %%r11\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            :
            :
            : "r11", "xmm8", "memory"
        );
        
        /* Actual function call - forces caller-save */
        helper_result = helper_func(pre_asm, i);
        
        asm volatile (
            "# POST-CALL CLOBBER\n\t"
            "mov $0x77777777, %%r12\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            :
            :
            : "r12", "xmm9", "memory"
        );
        
        total += helper_result;
    }
    
    printf("Final checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
