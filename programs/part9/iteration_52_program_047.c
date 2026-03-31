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

int main(void) {
    /* Seed random number generator */
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
    
    /* Use explicit register variables to pin values */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    
    /* Complex expression using most variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 / (v10 + 2) +
        v11 - v12 * v13 + v14 / (v15 + 3) -
        v16 + v17 * v18 - v19 / (v20 + 4) +
        r12_var - r13_var;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces the compiler to work around specific register usage */
    asm volatile (
        /* Perform some dummy operations */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        /* Clobber specific registers to force reloads */
        : [out] "=r" (v1)          /* output operand */
        : [in1] "r" (v2),          /* input operand 1 */
          [in2] "r" (v3)           /* input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* More complex expressions with mismatched types */
    /* Mixing different sized integers creates mode conversion needs */
    int small_int = v1 & 0xFF;      /* char-sized */
    long large_int = v2 * 1000L;
    
    /* Force address reloads with complex array indexing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode - may require address reloads */
    long array_access = 
        arr[(v1 + v2 * v3 - v4) % 100] +
        arr[(v5 * v6 + v7 - v8) % 100] +
        arr[(v9 - v10 * v11 + v12) % 100];
    
    /* Use the helper function - forces parameter passing in registers */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Another inline asm with different constraints */
    long asm_result;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %%r12, %[b]\n\t"
        "movq %[b], %[res]\n\t"
        : [res] "=r" (asm_result)
        : [a] "r" (v13),
          [b] "r" (v14)
        : "r12", "cc"
    );
    
    /* Final computation using all results */
    volatile long final_result = 
        complex_result + 
        array_access + 
        func_result + 
        asm_result + 
        small_int + 
        large_int;
    
    /* Use result to prevent dead code elimination */
    if (final_result > 0) {
        printf("Result: %ld\n", final_result);
    } else {
        printf("Alternative: %ld\n", -final_result);
    }
    
    return 0;
}
