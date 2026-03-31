/* reload_coverage.c
 * Designed to trigger GCC's reload pass initialization in reload.cc lines 1381-1399
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_coverage.c -o reload_test
 * For detailed reload analysis: gcc -O2 -fomit-frame-pointer -fdump-rtl-reload -march=x86-64 reload_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d, 
                            int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to create register pressure */
    return a * b + c * d - e * f + g * h - a + b - c + d - e + f - g + h;
}

/* Another helper with mixed types to force mode conversions */
__attribute__((noinline))
static double mixed_mode_op(int a, float b, double c, long d) {
    return (double)a + (double)b * c - (double)d;
}

int main(void) {
    /* Create massive register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile int64_t v1 = rand() % 100;
    int64_t v2 = rand() % 100 + 1;  /* +1 to avoid division by zero */
    register int64_t v3 asm ("r12") = rand() % 100;  /* Pin to specific register */
    int64_t v4 = rand() % 100;
    int64_t v5 = rand() % 100 + 1;
    int64_t v6 = rand() % 100;
    int64_t v7 = rand() % 100;
    int64_t v8 = rand() % 100;
    int64_t v9 = rand() % 100;
    int64_t v10 = rand() % 100;
    int64_t v11 = rand() % 100;
    int64_t v12 = rand() % 100;
    int64_t v13 = rand() % 100;
    int64_t v14 = rand() % 100;
    int64_t v15 = rand() % 100;
    int64_t v16 = rand() % 100;
    int64_t v17 = rand() % 100;
    int64_t v18 = rand() % 100;
    int64_t v19 = rand() % 100;
    int64_t v20 = rand() % 100;
    
    /* Force mode mismatches with different types */
    char c1 = rand() % 100;
    short s1 = rand() % 100;
    float f1 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    
    /* Complex array indexing to force address reloads */
    int64_t arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Move values using specific registers */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v1)      /* Output operand */
        : [input1] "r" (v2),      /* Input operand 1 */
          [input2] "r" (v3)       /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobber specific registers */
    );
    
    /* Another asm with different constraints */
    int64_t asm_result;
    asm volatile (
        "imulq %[in1], %[in2]\n\t"
        "movq %[in2], %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v4), [in2] "r" (v5)
        : "cc"  /* Clobber condition codes */
    );
    
    /* Complex expression using many variables - creates register pressure */
    /* Mix operations to force different register classes */
    int64_t complex_result = 
        v1 + v2 * v3 - v4 / v5 + 
        v6 * v7 - v8 + v9 * v10 +
        v11 - v12 * v13 + v14 / v15 -
        v16 + v17 * v18 - v19 + v20;
    
    /* Force mode conversion reloads */
    double mixed_result = mixed_mode_op(
        (int)c1,           /* char to int conversion */
        f1 + (float)s1,    /* short to float conversion */
        d1 * 2.0,          /* double operation */
        v1 + v2            /* int64_t to long conversion */
    );
    
    /* Complex array access with multiple index variables */
    /* This often requires address reloads on x86 */
    int64_t array_access = 
        arr[(v1 + v2 * v3 - v4) % 100] +
        arr[(v5 + v6 * v7) % 100] -
        arr[(v8 * v9 + v10) % 100] +
        arr[(v11 - v12 + v13) % 100];
    
    /* Use volatile variable in condition to prevent elimination */
    volatile int trigger = rand() % 2;
    if (trigger) {
        /* Force function call with many arguments - uses calling convention registers */
        complex_result += use_registers(v1, v2, v3, v4, v5, v6, v7, v8);
    } else {
        complex_result += use_registers(v9, v10, v11, v12, v13, v14, v15, v16);
    }
    
    /* More inline assembly with explicit register variables */
    register int64_t pinned1 asm ("r13") = v17;
    register int64_t pinned2 asm ("r14") = v18;
    
    asm volatile (
        "addq %[p2], %[p1]\n\t"
        : [p1] "+r" (pinned1)
        : [p2] "r" (pinned2)
        : "cc"
    );
    
    /* Structure with nested addressing */
    struct nested {
        int64_t a;
        struct {
            int64_t x;
            int64_t y;
        } inner;
        int64_t b;
    };
    
    struct nested nst;
    nst.a = v1;
    nst.inner.x = v2;
    nst.inner.y = v3;
    nst.b = v4;
    
    /* Taking address of nested member - can force address reloads */
    int64_t* ptr = &nst.inner.x;
    *ptr += complex_result;
    
    /* Final computation using all results to prevent dead code elimination */
    int64_t final_result = 
        complex_result + 
        asm_result + 
        (int64_t)mixed_result + 
        array_access + 
        pinned1 + 
        *ptr;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 100);
}
