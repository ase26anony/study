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
    
    /* Use explicit register variables to create conflicts */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    
    /* Complex expression using most variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 + v8 - v9 / (v10 + 1) +
        v11 * v12 - v13 + v14 / (v15 + 1) +
        v16 - v17 * v18 + v19 / (v20 + 1) +
        r12_var * 2 - r13_var / 3;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Perform some dummy operations */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        /* Clobber multiple registers to force reloads */
        : [out] "=r" (v1)
        : [in1] "r" (v2), [in2] "r" (v3)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "r14", "r15", "cc", "memory"
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (temp)
        : "r" (complex_expr)
        : "eax"
    );
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 20; i++) {
        /* Complex array indexing that may require address reloads */
        arr[(i * v1 + v2) % 100] = i + v3;
        arr[(i * v4 - v5) % 100] = arr[(i * v6 + v7) % 100] + v8;
    }
    
    /* Mix different data types to cause mode conversions */
    char char_var = v9 & 0xFF;
    short short_var = v10 & 0xFFFF;
    int int_var = v11;
    long long_var = v12;
    
    /* Operations requiring mode conversions */
    long mixed_expr = char_var + short_var * int_var - long_var / (v13 + 1);
    
    /* Use volatile to prevent optimization */
    volatile long volatile_sum = 0;
    for (int i = 0; i < 10; i++) {
        volatile_sum += arr[i] + mixed_expr;
    }
    
    /* Force conditional with complex expression to prevent dead code elimination */
    if (volatile_sum > 1000) {
        /* Call helper function - forces parameter passing in registers */
        long result = helper_func(v1, v2, v3, v4, v5, v6);
        
        /* More complex operations */
        result += helper_func(v7, v8, v9, v10, v11, v12);
        result += helper_func(v13, v14, v15, v16, v17, v18);
        
        /* Final output to prevent optimization */
        printf("Final result: %ld\n", result + volatile_sum + v19 + v20);
    } else {
        printf("Alternative path: %ld\n", volatile_sum);
    }
    
    return 0;
}
