/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

int main(void) {
    /* Create register pressure with many live variables */
    register long v1 asm ("r12") = 1;
    register long v2 asm ("r13") = 2;
    volatile long v3 = 3;
    volatile long v4 = 4;
    long v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    long v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    long v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    long v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    /* Complex expression creating register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 / (v9 + 1) + 
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20 * v21 - v22 / (v23 + 1) +
        v24 * v25;
    
    /* Inline assembly with fixed register constraints */
    long asm_input = complex_result;
    long asm_output;
    
    /* Force reloads by clobbering specific registers */
    asm volatile (
        "movq %1, %%rax\n\t"           /* Input to rax */
        "addq $100, %%rax\n\t"         /* Modify value */
        "movq %%rax, %0\n\t"           /* Output from rax */
        : "=r" (asm_output)            /* Output operand */
        : "r" (asm_input)              /* Input operand */
        : "%rax", "%rbx", "%rcx", "%rdx"  /* Clobber multiple registers */
    );
    
    /* More inline assembly with mismatched constraints */
    char char_var = 42;
    int int_result;
    
    /* Mode mismatch: char (QImode) to int (SImode/DImode) */
    asm volatile (
        "movsbl %1, %%eax\n\t"         /* Sign extend byte to dword */
        "imull $2, %%eax\n\t"          /* Multiply */
        "movl %%eax, %0\n\t"           /* Store result */
        : "=r" (int_result)
        : "r" (char_var)
        : "%rax", "cc"
    );
    
    /* Complex memory addressing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    long idx = v1 + v2 * v3 - v4;
    long mem_access = arr[idx + v5 * v6 - v7 / (v8 + 1)];
    
    /* Force address computation reloads */
    struct nested {
        int a;
        struct {
            long x;
            long y;
        } inner;
        int b;
    } nested_struct;
    
    long *ptr = &nested_struct.inner.x;
    *ptr = mem_access + asm_output;
    
    /* Call helper function forcing parameter passing reloads */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile in condition to prevent elimination */
    if (v3 > 0) {
        func_result += helper_func(v7, v8, v9, v10, v11, v12);
    }
    
    /* Final computation using all variables */
    long final_result = 
        complex_result + asm_output + int_result + 
        mem_access + *ptr + func_result;
    
    printf("Result: %ld\n", final_result);
    
    /* Additional stress: loop with many live variables */
    for (int i = 0; i < 1000; i++) {
        /* Force spills/reloads in loop */
        v1 = v1 + v2 - v3 * v4 / (v5 + 1);
        v2 = v2 + v3 - v4 * v5 / (v6 + 1);
        v3 = v3 + v4 - v5 * v6 / (v7 + 1);
        
        /* Inline asm in loop to force more reloads */
        asm volatile (
            "addq $1, %0\n\t"
            "addq $1, %1\n\t"
            : "+r" (v8), "+r" (v9)
            :
            : "cc"
        );
    }
    
    return (int)(final_result % 256);
}
