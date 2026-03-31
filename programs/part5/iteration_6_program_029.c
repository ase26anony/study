/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int helper(int seed) {
    return seed * 1103515245 + 12345;
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Non-inlineable function with complex caller-save requirements */
static __attribute__((noinline,noipa))
int test_caller_save(int iterations, int init_val) {
    volatile int vol_counter = iterations;  /* Prevent optimizations */
    int result = init_val;
    
    /* Use various register types that are call-clobbered */
    register long rax_val asm("rax") = result + 1;
    register long rbx_val asm("rbx") = result + 2;
    register long rcx_val asm("rcx") = result + 3;
    v4si vec_int = {result, result + 1, result + 2, result + 3};
    v4sf vec_float = {result * 1.0f, result * 2.0f, 
                      result * 3.0f, result * 4.0f};
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < vol_counter; i = i + 1) {
        /* Phase 1: Integer computations in call-clobbered registers */
        rax_val = rax_val * 6364136223846793005ULL + 1;
        rbx_val = rbx_val * 6364136223846793005ULL + 2;
        rcx_val = rcx_val * 6364136223846793005ULL + 3;
        
        /* Force spill point with asm that clobbers registers */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0xDEADBEEF, %%rax\n\t"
            "mov $0xCAFEBABE, %%rbx\n\t"
            "mov $0xBAADF00D, %%rcx"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result ^= (rax_val & 0xFFFF);
        result ^= (rbx_val & 0xFFFF) << 8;
        result ^= (rcx_val & 0xFFFF) << 16;
        
        /* Phase 2: Vector computations */
        vec_int = vec_int + (v4si){1, 2, 3, 4};
        vec_float = vec_float * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        
        /* Another asm that clobbers vector registers */
        asm volatile (
            "# Clobber vector regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%mm0, %%mm0"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "mm0", "memory"
        );
        
        /* Use vector values after clobber */
        result += vec_int[0] + vec_int[2];
        result += (int)vec_float[1] + (int)vec_float[3];
        
        /* Function call that creates basic block boundary */
        if (i & 1) {
            /* Call that ends a basic block, followed by label */
            result = helper(result);
            /* Label creates control flow edge */
            __label__ after_call;
            after_call:
            result ^= 0x12345678;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0xAAAAAAAA, %%r8\n\t"
                "mov $0xBBBBBBBB, %%r9"
                : 
                : 
                : "r8", "r9", "memory"
            );
            result = result * 3 + 1;
        }
        
        /* Phase 3: Mixed computations */
        double fp_val = result * 3.14159;
        long int_val = result * 0x9E3779B97F4A7C15ULL;
        
        /* Final asm clobbering multiple register classes */
        asm volatile (
            "# Final massive clobber\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%mm1, %%mm1"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
              "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
              "memory"
        );
        
        /* Use values after final clobber */
        result += (int)(fp_val / 2.71828);
        result ^= (int_val & 0xFFFFFFFF);
        
        /* Conditional that might make the call the end of BB */
        if (result & 0x100) {
            /* Another call at potential BB end */
            result = helper(result);
            /* Immediate jump target */
            goto update_loop;
        }
        
        update_loop:
        /* Empty statement for label */
        ;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    /* Multiple calls with different arguments */
    for (int i = 0; i < 5; i++) {
        int val = test_caller_save(iterations + i, i * 1000 + 123);
        total += val;
        printf("Iteration %d: result = %d (total = %d)\n", i, val, total);
    }
    
    /* Final checksum */
    int checksum = total ^ 0x55AA55AA;
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (checksum != 0) ? 0 : 1;
}
