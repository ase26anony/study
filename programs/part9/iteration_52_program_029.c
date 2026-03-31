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

/* Another helper with mixed types to cause mode conversions */
__attribute__((noinline))
static double mixed_helper(int a, double b, long c, float d) {
    return a * b + c - d;
}

int main(void) {
    /* Seed random for initialization */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
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
    
    /* Additional variables with mismatched types for mode conversions */
    volatile char c1 = 'A';
    volatile short s1 = 1000;
    volatile int i1 = 50000;
    volatile float f1 = 3.14159f;
    volatile double d1 = 2.71828;
    
    /* Explicit register variables to pin to specific registers */
    register long reg_var1 asm ("r12") = v1 + v2;
    register long reg_var2 asm ("r13") = v3 * v4;
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 * v10 +
        v11 - v12 * v13 + v14 / (v15 + 1) -
        v16 + v17 * v18 - v19 / (v20 + 1);
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces the compiler to work around specific register usage */
    asm volatile (
        /* Dummy operation that uses specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (complex_result)   /* Output operand */
        : [in1] "r" (v1),               /* Input operand 1 */
          [in2] "r" (v2)                /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx",   /* Clobbered registers */
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"                /* Clobber flags and memory */
    );
    
    /* Another inline asm with different constraints */
    long asm_result;
    asm volatile (
        "imulq %1, %0\n\t"
        "addq %2, %0"
        : "+r" (asm_result)
        : "r" (v3), "r" (v4)
        : "cc"
    );
    
    /* Force address reloads with complex memory addressing */
    long array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i;
    }
    
    /* Complex array indexing that may require address reloads */
    long array_access = 
        array[v1 + v2 * 3 - v3] +
        array[v4 * v5 + v6] +
        array[v7 - v8 * v9 + v10] +
        array[v11 * v12 - v13 + v14] +
        array[v15 + v16 - v17 * v18 + v19];
    
    /* Mixed type operations causing mode conversions */
    double mixed_result = mixed_helper(
        c1 + s1,    /* char + short -> int promotion */
        d1,         /* double */
        i1,         /* int -> long conversion */
        f1          /* float */
    );
    
    /* Complex expression with mixed types */
    double another_mixed = 
        (c1 * d1) +          /* char * double -> double */
        (s1 / f1) -          /* short / float -> float -> double */
        (i1 * 2.5) +         /* int * double -> double */
        (reg_var1 / 3.0);    /* long / double -> double */
    
    /* Call helper function - forces parameter passing in registers */
    long helper_result = helper_func(
        v1, v2, v3, v4, v5, v6
    );
    
    /* Use volatile condition to prevent dead code elimination */
    volatile int condition = (complex_result > 1000);
    if (condition) {
        helper_result += array_access;
    } else {
        helper_result -= asm_result;
    }
    
    /* Final computation using all results */
    long final_result = 
        complex_result + 
        helper_result + 
        (long)mixed_result + 
        (long)another_mixed + 
        array_access;
    
    /* Print to prevent optimization */
    printf("Final result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
