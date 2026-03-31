/* test_caller_save.c - Target GCC's caller-save insertion logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Vector types for SSE/MMX registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Volatile variables to extend register liveness */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile float vf1 = seed * 0.5f;
    volatile float vf2 = seed * 1.5f;
    
    /* Variables that will be forced into call-clobbered registers */
    int int_acc = 0;
    float float_acc = 0.0f;
    v4si vec_acc = {0, 0, 0, 0};
    v4sf fvec_acc = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTER PRESSURE ========== */
        /* Force values into specific integer registers */
        int r1 = vi1 + i * 3;
        int r2 = vi2 - i * 7;
        int r3 = r1 * r2;
        int r4 = r1 ^ r2;
        
        /* Clobber integer call-clobbered registers */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        int_acc += r3 + r4;
        
        /* ========== FLOATING POINT REGISTER PRESSURE ========== */
        /* Force values into xmm registers */
        float f1 = vf1 + i * 0.1f;
        float f2 = vf2 - i * 0.2f;
        float f3 = f1 * f2;
        float f4 = f1 / (f2 + 1.0f);
        
        /* Clobber SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            :
            :
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* Use floating values after clobber */
        float_acc += f3 + f4;
        
        /* ========== VECTOR REGISTER PRESSURE ========== */
        /* Vector computations */
        v4si v1 = {vi1 + i, vi2 - i, i * 2, i * 3};
        v4si v2 = {i, i * 4, i * 5, i * 6};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        
        /* Clobber MMX registers (x86-64 still has them) */
        asm volatile (
            "# Clobber MMX regs\n\t"
            "pxor %%mm0, %%mm0\n\t"
            "pxor %%mm1, %%mm1\n\t"
            "pxor %%mm2, %%mm2\n\t"
            "pxor %%mm3, %%mm3\n\t"
            "pxor %%mm4, %%mm4\n\t"
            "pxor %%mm5, %%mm5\n\t"
            "pxor %%mm6, %%mm6\n\t"
            "pxor %%mm7, %%mm7\n\t"
            :
            :
            : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
        );
        
        /* Use vector values after clobber */
        for (int j = 0; j < 4; j++) {
            vec_acc[j] += v3[j] + v4[j];
        }
        
        /* ========== MIXED REGISTER PRESSURE WITH CALL ========== */
        /* Create a basic block ending with a clobbering asm */
        if (i % 3 == 0) {
            /* More computations using all register types */
            int mix1 = int_acc * 2;
            float mix2 = float_acc * 1.5f;
            v4si mix3 = vec_acc + (v4si){1, 2, 3, 4};
            
            /* This asm simulates a function call that clobbers registers */
            /* It's placed where a basic block might end */
            asm volatile (
                "# Simulated function call\n\t"
                "call dummy_label%=\n\t"
                "dummy_label%=:\n\t"
                "add $1, %%rax\n\t"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11",
                  "xmm0", "xmm1", "xmm2", "xmm3",
                  "mm0", "mm1", "mm2", "memory"
            );
            
            /* Label/jump to create control flow edge after the "call" */
            /* This creates a basic block ending with the asm above */
            if (mix1 > 1000) {
                /* This jump creates the control flow edge needed for 
                   the BB_END update logic */
                goto skip_point;
            }
            
            /* Use mixed values after the "call" */
            int_acc = mix1 / 3;
            float_acc = mix2 * 0.8f;
            vec_acc = mix3 - (v4si){1, 1, 1, 1};
            
            skip_point:;
        }
        
        /* ========== ANOTHER CALL-LIKE CLOBBER ========== */
        /* Another asm that looks like a call, placed before a jump */
        if (i % 5 == 0) {
            asm volatile (
                "# Another simulated call\n\t"
                "push %%rax\n\t"
                "pop %%rax\n\t"
                :
                :
                : "rax", "rbx", "memory"
            );
            
            /* Conditional that creates another basic block boundary */
            if (int_acc & 1) {
                vi1++;
            } else {
                vi2--;
            }
        }
    }
    
    /* Final computation using all accumulators */
    int final_result = int_acc;
    for (int i = 0; i < 4; i++) {
        final_result += vec_acc[i];
    }
    final_result += (int)float_acc;
    
    return final_result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int result = x * y + 12345;
    /* Clobber registers to force caller-save around this call */
    asm volatile (
        "# Helper function clobber\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory"
    );
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Multiple calls to test_caller_save with different seeds */
    for (int i = 0; i < 10; i++) {
        int result = test_caller_save(iterations, i * 100);
        total += result;
        
        /* Insert actual function calls between test runs */
        /* This creates more opportunities for caller-save insertion */
        int helper_result = helper_func(i, iterations);
        total += helper_result % 1000;
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total == 0) {
        printf("This should never happen\n");
    }
    
    return total != 0 ? 0 : 1;
}
