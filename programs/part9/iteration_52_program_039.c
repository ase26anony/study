/* reload_trigger.c - Forces GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d,
                             int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to create register pressure */
    return a + b * c - d / (e + 1) + (f << 2) - (g >> 3) + h * h;
}

/* Another helper with mixed types to force mode conversions */
__attribute__((noinline))
static double mixed_mode_op(int a, float b, double c, char d) {
    return a + b + c + d;
}

int main(void) {
    /* Create massive register pressure with many live variables */
    volatile int64_t v1 = rand() % 100;
    int64_t v2 = v1 + 1;
    int64_t v3 = v2 * 2;
    int64_t v4 = v3 - 5;
    int64_t v5 = v4 / 2;
    int64_t v6 = v5 << 1;
    int64_t v7 = v6 >> 1;
    int64_t v8 = v7 | 0xFF;
    int64_t v9 = v8 & 0x0F;
    int64_t v10 = v9 ^ 0x55;
    int64_t v11 = v10 + 100;
    int64_t v12 = v11 - 50;
    int64_t v13 = v12 * 3;
    int64_t v14 = v13 / 7;
    int64_t v15 = v14 % 11;
    int64_t v16 = v15 << 2;
    int64_t v17 = v16 >> 1;
    int64_t v18 = v17 + 999;
    int64_t v19 = v18 - 888;
    int64_t v20 = v19 * 2;
    
    /* Pin specific variables to registers to create conflicts */
    register int64_t r12_var asm ("r12") = v1 + v2;
    register int64_t r13_var asm ("r13") = v3 + v4;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates reloads */
    asm volatile (
        /* Clobber multiple registers to force spills/reloads */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v20)          /* Output operand */
        : [in1] "r" (v19),          /* Input operand in register */
          [in2] "r" (r12_var)       /* Another input */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Complex addressing mode to force address reloads */
    int64_t* dynamic_array = (int64_t*)malloc(100 * sizeof(int64_t));
    for (int i = 0; i < 20; i++) {
        /* Complex index calculation - may need address reload */
        dynamic_array[(i + v1) * (v2 + 3) / (v3 + 1)] = v4 + i;
    }
    
    /* Mixed mode operations to force mode conversions */
    float f1 = v5 * 1.5f;
    double d1 = v6 * 2.5;
    char c1 = v7 & 0xFF;
    
    /* This forces mode mismatches and potential reloads */
    double mixed_result = mixed_mode_op(v8, f1, d1, c1);
    
    /* Very long expression chain - maximizes live ranges */
    int64_t result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        (v6 << 2) - (v7 >> 3) + v8 * v9 -
        v10 + v11 * v12 - v13 / (v14 + 1) +
        (v15 << 1) - (v16 >> 2) + v17 * v18 -
        v19 + v20 * r12_var - r13_var / (v1 + 1);
    
    /* Another inline asm with explicit register variables */
    int64_t final_result;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "movq %[b], %[res]\n\t"
        : [res] "=r" (final_result)
        : [a] "r" (result),
          [b] "r" (r12_var),
          [c] "r" (r13_var)
        : "cc"
    );
    
    /* Use volatile to prevent dead code elimination */
    volatile int64_t output = final_result + (int64_t)mixed_result;
    
    /* Force register pressure around function call */
    int64_t func_result = use_registers(
        v1, v2, v3, v4, v5, v6, v7, v8
    );
    
    /* Complex condition to keep everything alive */
    if (output > 1000 || func_result < 500) {
        printf("Result: %ld (mixed: %.2f)\n", 
               final_result + func_result, mixed_result);
    }
    
    free(dynamic_array);
    return (final_result > 0) ? 0 : 1;
}
