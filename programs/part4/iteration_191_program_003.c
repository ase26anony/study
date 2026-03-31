/* reload_test.c - Test program to trigger push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 123456789012345LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This forces reloads due to register class mismatches */
    asm volatile (
        /* Move int to output with 'r' constraint, but input might need secondary reload */
        "movl %1, %0\n\t"
        : "=r" (out1)          /* Output in general register */
        : "irm" (a)            /* Input: immediate, register, or memory - may need secondary */
        : /* no clobbers */
    );
    result += out1;
    
    /* ASM 2: 64-bit operations with specific constraints */
    /* May require secondary reloads for 64-bit constants */
    asm volatile (
        "addq %2, %1, %0\n\t"
        : "=r" (out3)          /* Output in register */
        : "r" (b),             /* b in register */
          "irm" (g_llong)      /* Global may need secondary reload */
        : "cc"                 /* Clobbers condition codes */
    );
    result += out3;
    
    /* ASM 3: Floating point with memory constraints */
    /* Forces memory reloads and potential secondary reloads */
    asm volatile (
        "addsd %1, %2, %0\n\t"
        : "=x" (out5)          /* Output in SSE register */
        : "x" (d),             /* d in SSE register */
          "m" (g_double)       /* Memory operand - may need address reload */
        : /* no clobbers */
    );
    result += (long long)out5;
    
    /* ASM 4: Mixed float/int with specific constraints */
    /* Creates complex reload scenario */
    asm volatile (
        "cvtsi2sd %1, %0\n\t"  /* Convert int to double */
        : "=x" (out5)          /* Output in SSE register */
        : "r" (out1)           /* Input in general register */
        : /* no clobbers */
    );
    result += (long long)out5;
    
    /* ASM 5: Vector operations with memory constraints */
    /* May trigger vector register reloads */
    asm volatile (
        "paddd %1, %2, %0\n\t"
        : "=x" (out_vec_int)   /* Output in vector register */
        : "x" (vec_int),       /* Input in vector register */
          "m" (vec_int)        /* Same value also from memory */
        : /* no clobbers */
    );
    result += out_vec_int[0];
    
    /* ASM 6: Complex addressing mode with multiple constraints */
    /* Forces address computation reloads */
    asm volatile (
        "mov (%[addr]), %0\n\t"
        : "=r" (out2)
        : [addr] "r" (&g_int)  /* Address in register */
        : "memory"
    );
    result += out2;
    
    /* ASM 7: Immediate with range limitations */
    /* May require secondary reload if immediate doesn't fit */
    asm volatile (
        "addl $0x7FFFFFFF, %0\n\t"  /* Large immediate */
        : "+r" (out1)
        : /* no inputs */
        : "cc"
    );
    result += out1;
    
    /* ASM 8: Output in specific register with input in memory */
    /* Forces output reload to specific register */
    register int reg_out asm("eax") = 0;
    asm volatile (
        "movl %1, %%eax\n\t"
        : "=a" (reg_out)
        : "m" (g_int)
        : /* eax already specified */
    );
    result += reg_out;
    
    /* ASM 9: Multiple outputs with different constraints */
    int out6, out7;
    asm volatile (
        "movl %2, %0\n\t"
        "movl %2, %1\n\t"
        : "=r" (out6), "=m" (out7)  /* One in reg, one in memory */
        : "r" (a)
        : "memory"
    );
    result += out6 + out7;
    
    /* ASM 10: Clobber many registers to force spills and reloads */
    asm volatile (
        "mov %1, %0\n\t"
        : "=r" (out1)
        : "r" (a)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory"
    );
    result += out1;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable inputs to prevent constant propagation */
    int int_val = argc > 1 ? atoi(argv[1]) : 100;
    long long llong_val = argc > 2 ? atoll(argv[2]) : 200;
    float float_val = argc > 3 ? atof(argv[3]) : 3.14f;
    double double_val = argc > 4 ? atof(argv[4]) : 2.71;
    
    /* Initialize vectors */
    v4si vec_int = {int_val, int_val + 1, int_val + 2, int_val + 3};
    v4sf vec_float = {float_val, float_val + 1.0f, float_val + 2.0f, float_val + 3.0f};
    
    /* Call function multiple times with different arguments */
    long long total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(int_val + i, llong_val + i, 
                                float_val + i, double_val + i,
                                vec_int, vec_float);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 256);
}
