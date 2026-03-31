/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) int test_caller_save(int iterations);

/* Non-inlineable helper to prevent optimization */
static __attribute__((noinline, noipa)) void clobber_all(void) {
    /* Empty function that compiler can't analyze */
}

/* Vector types to use SSE registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

int main(void) {
    int sum = 0;
    
    /* Call multiple times with different iteration counts */
    sum += test_caller_save(10);
    sum += test_caller_save(15);
    sum += test_caller_save(20);
    
    printf("Checksum: %d\n", sum);
    return 0;
}

static __attribute__((noinline)) int test_caller_save(int iterations) {
    volatile int vi = 7;          /* Force memory operations */
    volatile float vf = 3.14f;
    volatile double vd = 2.71828;
    
    /* Use various call-clobbered registers */
    register int64_t r1 asm("rax") = vi * 2;
    register int64_t r2 asm("rbx") = vi * 3;  /* rbx is callee-saved on x86-64 */
    register int64_t r3 asm("rcx") = vi * 4;
    register int64_t r4 asm("rdx") = vi * 5;
    register int64_t r5 asm("rsi") = vi * 6;
    register int64_t r6 asm("rdi") = vi * 7;
    
    /* Floating point/vector registers */
    register double fd1 asm("xmm0") = vd;
    register double fd2 asm("xmm1") = vd * 2.0;
    register v4si vec1 asm("xmm2");
    register v4sf vec2 asm("xmm3");
    
    /* Initialize vectors */
    vec1 = (v4si){vi, vi+1, vi+2, vi+3};
    vec2 = (v4sf){vf, vf*2, vf*3, vf*4};
    
    int result = 0;
    
    /* Loop to prevent hoisting of save/restore */
    for (volatile int i = 0; i < iterations; i++) {
        /* SECTION 1: Integer register pressure */
        int64_t temp1 = r1 + r2;
        int64_t temp2 = r3 + r4;
        
        /* Clobber integer registers - simulating a function call */
        asm volatile(
            "# Clobber integer regs\n\t"
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rcx\n\t"
            "mov $0x11111111, %%rdx\n\t"
            "mov $0x22222222, %%rsi\n\t"
            "mov $0x33333333, %%rdi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rcx", "rdx", "rsi", "rdi", "memory"
        );
        
        /* Use the original values after clobber - forces save/restore */
        result += (int)(temp1 + r5 + r6);
        
        /* SECTION 2: Floating point/vector pressure */
        double ftemp = fd1 * fd2;
        v4si vtemp1 = vec1 + (v4si){1, 2, 3, 4};
        v4sf vtemp2 = vec2 * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
        
        /* Clobber floating point/vector registers */
        asm volatile(
            "# Clobber FP/vector regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use original FP/vector values */
        result += (int)ftemp;
        result += vtemp1[0] + vtemp1[2];
        
        /* SECTION 3: Mixed usage with actual function call */
        /* Create a basic block ending with a call */
        if (i & 1) {
            /* This call should end a basic block */
            clobber_all();
            
            /* Label to create control flow edge */
            asm volatile("# LABEL_AFTER_CALL %0" : "+r"(result));
            
            /* Use values across the call */
            result += (int)(r1 + r3 + r5);
            fd1 = fd1 * 1.1;
        } else {
            /* Alternative path without call */
            result += (int)(r2 + r4 + r6);
            fd2 = fd2 * 0.9;
        }
        
        /* SECTION 4: More register pressure with MMX (if available) */
        register long long mmx_val asm("mm0") = 0x1122334455667788ULL;
        
        asm volatile(
            "# Use MMX\n\t"
            "movq %0, %%mm0\n\t"
            "psllq $4, %%mm0\n\t"
            : /* no outputs */
            : "r"(mmx_val)
            : "mm0"
        );
        
        /* Force MMX clobber */
        asm volatile(
            "# Clobber MMX\n\t"
            "pxor %%mm0, %%mm0\n\t"
            : /* no outputs */
            : /* no inputs */
            : "mm0", "memory"
        );
        
        /* Update values for next iteration */
        r1 += vi;
        r2 += vi * 2;
        r3 += vi * 3;
        fd1 += 0.1;
        fd2 -= 0.1;
        vec1 += (v4si){1, 1, 1, 1};
        vec2 *= (v4sf){0.99f, 0.99f, 0.99f, 0.99f};
    }
    
    return result;
}

/* Additional non-inlineable functions to create more call sites */
static __attribute__((noinline, noipa)) void dummy_call_1(int x) {
    asm volatile("# dummy1 %0" : : "r"(x) : "memory");
}

static __attribute__((noinline, noipa)) void dummy_call_2(float x) {
    asm volatile("# dummy2 %x0" : : "x"(x) : "memory");
}

/* Function that uses the helper to create more complex CFG */
void __attribute__((noinline)) extra_calls(int n) {
    for (int i = 0; i < n; i++) {
        /* These calls create basic blocks ending with calls */
        dummy_call_1(i);
        if (i % 3 == 0) {
            dummy_call_2(i * 1.5f);
        }
    }
}
