/* reload_test.c - Trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long barrier = a + b + c + d + e + f;
    return barrier * 2;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_calc(int x, int y, int z) {
    return (x * y) / (z + 1);
}

int main(void) {
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Create massive register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    volatile int v11 = rand() % 100;
    volatile int v12 = rand() % 100;
    volatile int v13 = rand() % 100;
    volatile int v14 = rand() % 100;
    volatile int v15 = rand() % 100;
    volatile int v16 = rand() % 100;
    volatile int v17 = rand() % 100;
    volatile int v18 = rand() % 100;
    volatile int v19 = rand() % 100;
    volatile int v20 = rand() % 100;
    
    /* Additional non-volatile variables for more pressure */
    int nv1 = v1 + 1, nv2 = v2 + 2, nv3 = v3 + 3, nv4 = v4 + 4;
    int nv5 = v5 + 5, nv6 = v6 + 6, nv7 = v7 + 7, nv8 = v8 + 8;
    
    /* Explicit register variables to force specific register allocation */
    /* This creates conflicts requiring reloads */
    register long reg_var1 asm ("r10") = v1 * 2;
    register long reg_var2 asm ("r11") = v2 * 3;
    register long reg_var3 asm ("r12") = v3 * 4;
    
    /* Complex expression using many variables - creates long live ranges */
    int result = v1 + v2 * v3 - v4 / (v5 + 1) + v6 % (v7 + 1);
    result += v8 * v9 - v10 + v11 * v12 - v13 / (v14 + 1);
    result += v15 % (v16 + 1) + v17 * v18 - v19 + v20;
    
    /* Mix operations with different types to force mode conversions */
    char c1 = v1 & 0xFF;
    short s1 = v2 & 0xFFFF;
    int i1 = v3;
    long l1 = v4;
    
    /* Operations requiring mode conversions */
    result += (int)c1 + (int)s1 + (int)l1;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    int asm_in = v5;
    int asm_out;
    
    asm volatile (
        "movl %1, %%eax\n\t"          /* Input to eax */
        "addl $100, %%eax\n\t"        /* Do some operation */
        "movl %%eax, %0\n\t"          /* Output from eax */
        : "=r" (asm_out)              /* Output constraint */
        : "r" (asm_in)                /* Input constraint */
        : "%eax", "%ebx", "%ecx"      /* Clobbered registers - forces reloads */
    );
    
    result += asm_out;
    
    /* Another asm with different constraints */
    long asm_in2 = reg_var1;
    long asm_out2;
    
    asm volatile (
        "addq %1, %%r10\n\t"          /* Use r10 explicitly */
        "movq %%r10, %0\n\t"
        : "=r" (asm_out2)
        : "r" (asm_in2)
        : "%r10", "%r11"              /* Clobber specific registers */
    );
    
    /* Complex memory addressing - forces address reloads */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Complex array indexing with multiple variables */
    int idx = arr[v1 + v2 * v3 - v4 / (v5 + 1)];
    result += idx;
    
    /* Nested structure with address taking */
    struct inner {
        int a;
        int b;
        int c[5];
    };
    
    struct outer {
        struct inner in1;
        struct inner in2;
        int x;
    } outer_var;
    
    /* Take address of nested member - complex addressing */
    int *ptr = &outer_var.in1.c[v1 % 5];
    *ptr = result;
    
    /* Function call with many arguments - forces register parameter passing */
    /* System V ABI uses rdi, rsi, rdx, rcx, r8, r9 for first 6 args */
    long func_result = helper_func(
        reg_var1, reg_var2, reg_var3, 
        v10, v11, v12
    );
    
    /* More complex expressions keeping variables live */
    result = complex_calc(result, v13, v14);
    result += complex_calc(v15, v16, v17);
    result += complex_calc(v18, v19, v20);
    
    /* Use all variables in final computation to prevent dead code elimination */
    volatile int final_check = 
        (v1 | v2 | v3 | v4 | v5 | v6 | v7 | v8 | v9 | v10 |
         v11 | v12 | v13 | v14 | v15 | v16 | v17 | v18 | v19 | v20);
    
    if (final_check > 0) {
        result += func_result + asm_out2 + nv1 + nv2 + nv3 + nv4 + nv5 + nv6 + nv7 + nv8;
    }
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}
