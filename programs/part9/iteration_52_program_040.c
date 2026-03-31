/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_address(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing that may need address reloads */
    return base[idx1 + idx2 * idx3 - (idx1 & idx2) | idx3];
}

int main(void) {
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Create massive register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile long v1 = rand() % 100 + 1;
    volatile long v2 = rand() % 100 + 1;
    volatile long v3 = rand() % 100 + 1;
    volatile long v4 = rand() % 100 + 1;
    volatile long v5 = rand() % 100 + 1;
    volatile long v6 = rand() % 100 + 1;
    volatile long v7 = rand() % 100 + 1;
    volatile long v8 = rand() % 100 + 1;
    volatile long v9 = rand() % 100 + 1;
    volatile long v10 = rand() % 100 + 1;
    volatile long v11 = rand() % 100 + 1;
    volatile long v12 = rand() % 100 + 1;
    volatile long v13 = rand() % 100 + 1;
    volatile long v14 = rand() % 100 + 1;
    volatile long v15 = rand() % 100 + 1;
    volatile long v16 = rand() % 100 + 1;
    volatile long v17 = rand() % 100 + 1;
    volatile long v18 = rand() % 100 + 1;
    volatile long v19 = rand() % 100 + 1;
    volatile long v20 = rand() % 100 + 1;
    
    /* Additional non-volatile variables for more pressure */
    long nv1 = v1 + 1, nv2 = v2 + 2, nv3 = v3 + 3, nv4 = v4 + 4, nv5 = v5 + 5;
    long nv6 = v6 + 6, nv7 = v7 + 7, nv8 = v8 + 8, nv9 = v9 + 9, nv10 = v10 + 10;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    long asm_result;
    asm volatile (
        /* Clobber multiple registers to force spills/reloads */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 + v9 / (v10 + 2) +
        v11 - v12 * v13 + v14 / (v15 + 3) -
        v16 + v17 * v18 - v19 / (v20 + 4);
    
    /* More arithmetic mixing different variable types */
    /* Mode mismatches can trigger reloads */
    int int_var = (int)v1;
    short short_var = (short)v2;
    char char_var = (char)v3;
    
    /* Operations with mode conversions */
    long mixed_expr = int_var + short_var * char_var - 
                     ((int)char_var << 2) + (short_var & 0xFF);
    
    /* Force address reloads with complex array access */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Complex addressing with multiple index variables */
    int idx_result = complex_address(array, 
                                     (int)v1, (int)v2, (int)v3);
    
    /* Use explicit register variables to pin to specific registers */
    /* This creates conflicts with compiler's register allocation */
    register long reg_var1 asm ("r12") = v1 + v2;
    register long reg_var2 asm ("r13") = v3 + v4;
    register long reg_var3 asm ("r14") = v5 + v6;
    
    /* Operations with register-pinned variables */
    long reg_expr = reg_var1 * reg_var2 - reg_var3;
    
    /* Another inline asm that uses specific output registers */
    long fixed_reg_out;
    asm volatile (
        "movq $100, %%r15\n\t"
        "imulq %[in], %%r15\n\t"
        "movq %%r15, %[out]\n\t"
        : [out] "=r" (fixed_reg_out)
        : [in] "r" (v7)
        : "r15"
    );
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v8, v9, v10, v11, v12, v13);
    
    /* Use volatile condition to prevent dead code elimination */
    volatile int condition = (complex_expr > 1000);
    if (condition) {
        /* More operations that keep variables live */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
    }
    
    /* Final computation using all variables to ensure they're not optimized away */
    long final_result = 
        asm_result + complex_expr + mixed_expr + idx_result + 
        reg_expr + fixed_reg_out + func_result + nv1 + nv2 + nv3 +
        nv4 + nv5 + nv6 + nv7 + nv8 + nv9 + nv10;
    
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
