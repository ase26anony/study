/* reload_coverage.c - Test program to cover push_reload initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This forces reloads due to mismatched constraints */
    asm volatile (
        /* Move int to output with 'r' constraint, but input might need secondary reload */
        "movl %1, %0\n\t"
        /* Use the value in a computation to prevent optimization */
        "addl $100, %0"
        : "=r" (out1)          /* Output in general reg */
        : "i" (g_int)          /* Input as immediate (may need secondary reload) */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operations with memory constraints */
    /* Forces memory address reloads */
    asm volatile (
        "movq %1, %%rax\n\t"   /* Load from memory */
        "addq %2, %%rax\n\t"   /* Add immediate */
        "movq %%rax, %0"
        : "=r" (out3)          /* Output in register */
        : "m" (g_llong),       /* Memory input */
          "i" (1000LL)         /* Large immediate (may need secondary reload) */
        : "rax", "cc"          /* Clobber rax and flags */
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with mixed modes */
    /* Different in/out modes trigger specific reload initialization */
    asm volatile (
        "movss %1, %%xmm0\n\t"     /* Load float */
        "cvtss2sd %%xmm0, %%xmm1\n\t" /* Convert to double */
        "movsd %%xmm1, %0"
        : "=m" (out5)           /* Output to memory */
        : "r" (c),              /* Input in register (float in int reg - needs reload) */
          "m" (g_double)        /* Memory operand for address reload */
        : "xmm0", "xmm1", "cc"  /* Clobber */
    );
    accumulator += (long long)out5;
    
    /* ASM 4: Vector operations with specific constraints */
    /* Vector moves often need secondary reloads */
    asm volatile (
        "movdqa %1, %0\n\t"     /* Aligned vector move */
        "paddd %2, %0"          /* Vector add */
        : "=x" (out_vec_int)    /* Output in xmm register */
        : "xm" (vec_int),       /* Input: register or memory */
          "xm" (g_array[0])     /* Memory with possible complex address */
        : "cc"
    );
    
    /* ASM 5: Multiple outputs with different classes */
    /* Forces multiple reload entries */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %2, %1\n\t"
        "addl $42, %0\n\t"
        "subl $42, %1"
        : "=&r" (out1), "=&r" (out2)  /* Two earlyclobber outputs */
        : "mr" (a)                     /* Memory or register input */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 6: Complex addressing mode that needs secondary reload */
    /* Force address computation into register */
    asm volatile (
        "leaq g_array(%%rip), %%rax\n\t"
        "movl (%%rax,%1,4), %0"
        : "=r" (out1)
        : "r" (a & 7)           /* Scaled index - needs register */
        : "rax", "cc"
    );
    accumulator += out1;
    
    /* ASM 7: In/out operand with different modes */
    /* Tests inmode != outmode initialization */
    asm volatile (
        "mov %1, %k0\n\t"       /* Move 32-bit to 64-bit register */
        "shl $32, %0"
        : "=r" (out3)           /* 64-bit output */
        : "r" (out1)            /* 32-bit input */
        : "cc"
    );
    accumulator += out3;
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable values to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize test variables */
    int a = base + 1;
    long long b = (long long)base * 1000;
    float c = (float)base / 3.0f;
    double d = (double)base / 7.0;
    
    /* Initialize vectors */
    v4si vec_int = {base, base+1, base+2, base+3};
    v4sf vec_float = {c, c+1.0f, c+2.0f, c+3.0f};
    
    /* Call function that triggers reloads */
    long long result = trigger_reloads(a, b, c, d, vec_int, vec_float);
    
    /* Use result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional volatile asm to ensure reload pass runs */
    asm volatile ("" : : : "memory");
    
    return (result > 0) ? 0 : 1;
}
