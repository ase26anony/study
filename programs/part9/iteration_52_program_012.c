/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d,
                             int64_t e, int64_t f, int64_t g, int64_t h) {
    volatile int64_t result = 0;
    /* Complex expression to create register pressure */
    result = a + b * c - d / (e + 1) + (f << 2) | (g & h);
    return result;
}

/* Another helper with mismatched types */
__attribute__((noinline))
static double mixed_mode_op(int a, double b, float c, long d) {
    /* Mixing different types forces mode conversions */
    return (double)a + b * (double)c + (double)d;
}

int main(void) {
    /* Create massive register pressure with many live variables */
    volatile int64_t v1 = 1;
    register int64_t v2 asm ("r12") = 2;  /* Pin to specific register */
    int64_t v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int64_t v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14;
    int64_t v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Additional variables with different types */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    char c1 = 'A', c2 = 'B';
    short s1 = 100, s2 = 200;
    
    /* Complex array indexing - forces address reloads */
    int64_t arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Inline assembly with fixed register constraints */
    int64_t asm_result;
    asm volatile (
        /* Clobber multiple specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* More inline assembly with mismatched constraints */
    double d_result;
    asm volatile (
        "cvtsi2sd %[int_in], %%xmm0\n\t"
        "addsd %[double_in], %%xmm0\n\t"
        "movsd %%xmm0, %[out]\n\t"
        : [out] "=m" (d_result)
        : [int_in] "r" (v3), [double_in] "x" (d1)
        : "xmm0", "xmm1"
    );
    
    /* Complex expression using most variables - creates register pressure */
    int64_t complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        (v6 << (v7 % 8)) | (v8 & v9) ^ 
        (v10 + v11 - v12 * v13 / (v14 | 1)) +
        (v15 << 3) - (v16 >> 2) +
        (v17 & 0xFF) | (v18 << 8) +
        v19 * v20;
    
    /* Mix types to force mode conversions */
    double mixed = mixed_mode_op(v1, d1, f1, v2);
    
    /* Force address computation reloads with complex indexing */
    int64_t addr_reload = 
        arr[v1 + v2 * 2 - v3] +
        arr[v4 % 50 + v5] +
        arr[(v6 * v7) % 100] +
        arr[v8 & 0x7F] +
        arr[v9 | 0x80];
    
    /* Nested struct with address taken */
    struct nested {
        int64_t a;
        struct {
            int64_t b;
            int64_t c;
        } inner;
        int64_t d;
    } s = {1, {2, 3}, 4};
    
    int64_t* ptr1 = &s.a;
    int64_t* ptr2 = &s.inner.b;
    int64_t* ptr3 = &s.inner.c;
    int64_t* ptr4 = &s.d;
    
    /* Use all pointers to prevent optimization */
    int64_t struct_sum = *ptr1 + *ptr2 + *ptr3 + *ptr4;
    
    /* Function call with many parameters - forces register allocation */
    int64_t func_result = use_registers(
        v1, v2, v3, v4, v5, v6, v7, v8
    );
    
    /* Use volatile to prevent dead code elimination */
    volatile int64_t final_check = 0;
    if (complex_expr > 1000) {
        final_check = asm_result + func_result + addr_reload + struct_sum;
    } else {
        final_check = mixed + d_result;
    }
    
    /* Use all variables in output to prevent optimization */
    printf("Results: %ld %ld %f %ld %ld\n", 
           complex_expr, func_result, mixed, addr_reload, final_check);
    
    return (int)(final_check % 256);
}
