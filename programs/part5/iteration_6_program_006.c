/* test_caller_save.c - Target GCC caller-save.cc lines 905-913 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations, int seed) {
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Volatile variables to extend liveness across asm clobbers */
    volatile int vi1 = seed;
    volatile int vi2 = seed * 2;
    volatile float vf1 = seed * 0.5f;
    volatile float vf2 = seed * 1.5f;
    
    /* Variables that will live across clobbering asm statements */
    int int_acc = 0;
    float float_acc = 0.0f;
    v4si vec_acc = {0, 0, 0, 0};
    
    /* Loop to prevent hoisting of save/restore code */
    for (int i = 0; i < iterations; i++) {
        /* ====== INTEGER REGISTER CLOBBER SEQUENCE ====== */
        /* Compute values in call-clobbered registers */
        int r1 = vi1 * 3 + i;
        int r2 = vi2 * 7 - i;
        
        /* Use the values before clobbering */
        int_acc += r1 + r2;
        
        /* Clobber multiple integer registers - simulating a call */
        /* This forces caller-save for rax, rbx, rcx, rdx */
        asm volatile (
            "# CLOBBER INTEGER REGS\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0x55555555, %%rcx\n\t"
            "mov $0xAAAAAAAA, %%rdx\n\t"
            "add $1, %%rax\n\t"
            "add $1, %%rbx\n\t"
            "add $1, %%rcx\n\t"
            "add $1, %%rdx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values again after clobber - forcing restore */
        int_acc += r1 - r2;
        
        /* ====== FLOATING POINT REGISTER CLOBBER SEQUENCE ====== */
        /* Compute in xmm registers */
        float f1 = vf1 * 2.0f + i;
        float f2 = vf2 * 3.0f - i;
        
        float_acc += f1 + f2;
        
        /* Clobber xmm registers */
        asm volatile (
            "# CLOBBER XMM REGS\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "addps %%xmm0, %%xmm0\n\t"
            "addps %%xmm1, %%xmm1\n\t"
            "addps %%xmm2, %%xmm2\n\t"
            "addps %%xmm3, %%xmm3\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use floating values after clobber */
        float_acc += f1 - f2;
        
        /* ====== VECTOR REGISTER CLOBBER SEQUENCE ====== */
        /* Vector computations */
        v4si v1 = {vi1 + i, vi1 - i, vi1 * i, vi1 / (i + 1)};
        v4si v2 = {vi2 + i, vi2 - i, vi2 * i, vi2 / (i + 1)};
        
        vec_acc += v1 + v2;
        
        /* Clobber more xmm registers used for vectors */
        asm volatile (
            "# CLOBBER MORE XMM REGS\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "addps %%xmm4, %%xmm4\n\t"
            "addps %%xmm5, %%xmm5\n\t"
            "addps %%xmm6, %%xmm6\n\t"
            "addps %%xmm7, %%xmm7\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm4", "xmm5", "xmm6", "xmm7", "memory"
        );
        
        /* Use vectors after clobber */
        vec_acc += v1 - v2;
        
        /* ====== CREATE BASIC BLOCK ENDING WITH CLOBBER ====== */
        /* Conditional to create control flow edge after asm */
        if (int_acc % 7 == 0) {
            /* Another clobber at potential block end */
            asm volatile (
                "# POTENTIAL BLOCK-ENDING CLOBBER\n\t"
                "mov $0x33333333, %%r8\n\t"
                "mov $0x44444444, %%r9\n\t"
                "mov $0x55555555, %%r10\n\t"
                "mov $0x66666666, %%r11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "r10", "r11", "memory"
            );
            /* Label or jump to create edge */
            vi1++;  /* This creates a use after the asm */
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# ALTERNATIVE CLOBBER\n\t"
                "pxor %%xmm8, %%xmm8\n\t"
                "pxor %%xmm9, %%xmm9\n\t"
                "pxor %%xmm10, %%xmm10\n\t"
                "pxor %%xmm11, %%xmm11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "xmm8", "xmm9", "xmm10", "xmm11", "memory"
            );
            vi2++;
        }
        
        /* Force another computation using clobbered values */
        int_acc += vi1 * vi2;
    }
    
    /* Final computation to use all accumulated values */
    int result = int_acc + (int)float_acc;
    for (int i = 0; i < 4; i++) {
        result += vec_acc[i];
    }
    
    return result;
}

/* External function call to force additional caller-save */
static __attribute__((noinline))
int external_helper(int x) {
    return x * 3 + 7;
}

/* Another function with calls inside */
static __attribute__((noinline))
int nested_calls(int x) {
    int a = external_helper(x);
    
    /* Call with live values in registers */
    int b = external_helper(a);
    
    /* Mix with asm clobber */
    asm volatile (
        "# NESTED CLOBBER\n\t"
        "mov $0x77777777, %%r12\n\t"
        "mov $0x88888888, %%r13\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "memory"
    );
    
    int c = external_helper(b);
    
    return a + b + c;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        int result = test_caller_save(iterations, i * 17);
        total += result;
        
        /* Also test nested calls */
        total += nested_calls(i * 23);
    }
    
    printf("Total checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
