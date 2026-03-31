/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int test_caller_save(int iterations) {
    /* Mixed register types that are call-clobbered on x86-64 */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3;
    volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
    volatile double vd1 = 1.25, vd2 = 2.25, vd3 = 3.25;
    
    /* Vector types to use SSE registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    volatile v4si vec_int = {1, 2, 3, 4};
    volatile v4sf vec_float = {1.1f, 2.2f, 3.3f, 4.4f};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (int i = 0; i < iterations; i++) {
        /* ========== INTEGER REGISTERS ========== */
        /* Use rax, rbx, rcx, rdx (call-clobbered on x86-64) */
        int a = vi1 + i;
        int b = vi2 * a;
        int c = vi3 ^ b;
        
        /* Clobber integer registers - simulating a function call */
        asm volatile (
            "# Clobber integer registers\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "mov $0x55555555, %%rcx\n\t"
            "mov $0xAAAAAAAA, %%rdx\n\t"
            "add $1, %%rax\n\t"
            "sub $1, %%rbx\n\t"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += a + b + c;
        
        /* ========== FLOATING POINT REGISTERS ========== */
        /* Use xmm0-xmm5 (call-clobbered on x86-64) */
        float f1 = vf1 * i;
        float f2 = vf2 / (f1 + 1.0f);
        double d1 = vd1 * i;
        double d2 = vd2 / (d1 + 1.0);
        
        /* Clobber floating point registers */
        asm volatile (
            "# Clobber floating point registers\n\t"
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
        
        /* Use floating point values after clobber */
        result += (int)(f1 + f2 + d1 + d2);
        
        /* ========== VECTOR REGISTERS ========== */
        /* Use vector registers */
        v4si v1 = vec_int + i;
        v4si v2 = v1 * (v4si){2, 2, 2, 2};
        v4sf vf = vec_float * (float)i;
        
        /* Clobber more registers including MMX if available */
        asm volatile (
            "# Clobber mixed registers\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "#ifdef __MMX__\n\t"
            "  emms\n\t"
            "#endif\n\t"
            :
            :
            : "xmm6", "xmm7"
            #ifdef __MMX__
            , "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
            #endif
        );
        
        /* Use vector values after clobber */
        for (int j = 0; j < 4; j++) {
            result += v1[j] + v2[j] + (int)vf[j];
        }
        
        /* ========== CREATE BASIC BLOCK ENDING WITH CLOBBER ========== */
        /* This creates a control flow edge right after a clobbering asm */
        if (i % 3 == 0) {
            /* Additional clobber that might end a basic block */
            asm volatile (
                "# Potential block-ending clobber\n\t"
                "mov $0x11111111, %%r8\n\t"
                "mov $0x22222222, %%r9\n\t"
                "mov $0x33333333, %%r10\n\t"
                "mov $0x44444444, %%r11\n\t"
                :
                :
                : "r8", "r9", "r10", "r11", "memory"
            );
            /* Label/jump creates edge - might make previous asm a block end */
            result += 1000;
        } else if (i % 3 == 1) {
            /* Different path with different clobbers */
            asm volatile (
                "# Alternative clobber\n\t"
                "pxor %%xmm8, %%xmm8\n\t"
                "pxor %%xmm9, %%xmm9\n\t"
                :
                :
                : "xmm8", "xmm9", "memory"
            );
            result += 2000;
        }
        
        /* Force spill by using all volatile variables */
        vi1 = result % 100;
        vf1 = (float)(result % 100) / 10.0f;
        vd1 = (double)(result % 100) / 10.0;
    }
    
    return result;
}

/* Another function to create actual call instructions */
static __attribute__((noinline))
int helper_func(int x, int y) {
    volatile int r = x * y + 12345;
    /* Clobber more registers */
    asm volatile (
        "# Helper function clobber\n\t"
        "mov $0xDEADBEEF, %%r12\n\t"
        "mov $0xBEEFDEAD, %%r13\n\t"
        :
        :
        : "r12", "r13", "memory"
    );
    return r;
}

int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    int total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 5; i++) {
        int r = test_caller_save(iterations + i);
        total += r;
        
        /* Call helper between test calls to create more caller-save contexts */
        int h = helper_func(i, r % 1000);
        total += h;
        
        printf("Iteration %d: test=%d, helper=%d, total=%d\n", 
               i, r, h, total);
    }
    
    /* Final computation using all results */
    volatile int final = total;
    for (int i = 0; i < 3; i++) {
        /* More clobbering to force final spills */
        asm volatile (
            "# Final clobbering\n\t"
            "mov $0x99999999, %%r14\n\t"
            "mov $0x88888888, %%r15\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            :
            :
            : "r14", "r15", "xmm10", "xmm11", "memory"
        );
        final = final * 31 + 17;
    }
    
    printf("Final result: %d (0x%08x)\n", final, final);
    return final != 0 ? 0 : 1;
}
