/* reload_test.c - Test program to trigger push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 0x123456789ABCDEF0LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Vector types for testing different machine modes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with multiple inline asm statements requiring reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    int out1, out2;
    long long out3, out4;
    float out5;
    double out6;
    v4si out_vec_int;
    v4sf out_vec_float;
    long long accumulator = 0;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This forces reloads due to register class mismatches */
    asm volatile (
        /* Template uses different size modifiers for different modes */
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (out1)                /* Output in general reg */
        : "r" (a),                   /* Input in general reg */
          "i" (0x7FFF)               /* Immediate that might need reload */
        : "cc"                       /* Clobbers condition codes */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operations with memory constraints */
    /* May require secondary reloads for address computation */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (out3)                /* Output */
        : "m" (g_llong),             /* Memory operand - may need address reload */
          "r" (b)                    /* Register operand */
        : "rax", "cc"                /* Clobber rax and condition codes */
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with mixed constraints */
    /* Forces different inmode/outmode initialization */
    asm volatile (
        "movss %1, %0\n\t"
        "addss %2, %0\n\t"
        : "=x" (out5)                /* Output in SSE register */
        : "x" (c),                   /* Input in SSE register */
          "m" (g_float)              /* Memory operand */
        : /* No clobbers */
    );
    /* Use result to prevent elimination */
    accumulator += (int)out5;
    
    /* ASM 4: Double precision with specific constraints */
    asm volatile (
        "movsd %1, %0\n\t"
        : "=f" (out6)                /* Output in floating reg */
        : "fm" (d)                   /* Input: floating or memory */
        : /* No clobbers */
    );
    accumulator += (long long)out6;
    
    /* ASM 5: Vector operations - different machine modes */
    /* Likely to trigger complex reload scenarios */
    asm volatile (
        "movdqa %1, %0\n\t"
        "paddd %2, %0\n\t"
        : "=x" (out_vec_int)         /* Output in XMM register */
        : "x" (vec_int),             /* Input in XMM register */
          "m" (g_array)              /* Memory operand - address may need reload */
        : /* No clobbers */
    );
    /* Sum vector elements */
    for (int i = 0; i < 4; i++) {
        accumulator += out_vec_int[i];
    }
    
    /* ASM 6: Complex constraints requiring secondary reloads */
    /* Using 'r' constraint with constant pool address */
    asm volatile (
        "lea %1, %%rax\n\t"
        "mov (%%rax), %%rbx\n\t"
        "mov %%rbx, %0\n\t"
        : "=r" (out2)                /* Output */
        : "r" (&g_int)               /* Input: address - may need secondary reload */
        : "rax", "rbx", "memory"     /* Clobber multiple regs and memory */
    );
    accumulator += out2;
    
    /* ASM 7: Multiple outputs with different constraints */
    asm volatile (
        "mov %2, %0\n\t"
        "mov %3, %1\n\t"
        : "=r" (out1), "=r" (out2)   /* Two outputs */
        : "r" (a), "m" (g_int)       /* Mixed input constraints */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 8: Inline asm with 'X' constraint (any operand) */
    /* Can trigger various reload paths */
    {
        long long complex_imm = 0x12345678;
        asm volatile (
            "mov %1, %0\n\t"
            "add $0x1234, %0\n\t"
            : "=r" (out4)
            : "X" (complex_imm)      /* 'X' - any operand */
            : "cc"
        );
        accumulator += out4;
    }
    
    return accumulator;
}

/* Main function that sets up test data and calls trigger function */
int main(int argc, char *argv[]) {
    /* Use argv to create variable inputs preventing constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Initialize test variables with non-constant values */
    int a = rand() % 100;
    long long b = g_llong + rand() % 1000;
    float c = g_float + (rand() % 100) * 0.01f;
    double d = g_double + (rand() % 100) * 0.01;
    
    /* Initialize vector types */
    v4si vec_int = {a, a+1, a+2, a+3};
    v4sf vec_float = {c, c+1.0f, c+2.0f, c+3.0f};
    
    /* Call function with all test variables */
    long long result = trigger_reloads(a, b, c, d, vec_int, vec_float);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional volatile operations to ensure asm isn't optimized away */
    asm volatile ("" : : "r" (result) : "memory");
    
    return (result > 0) ? 0 : 1;
}
