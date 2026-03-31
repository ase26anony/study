/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization code in reload.cc
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_trigger
 */

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
static double fp_helper(double x, double y, int z) {
    /* Mix integer and floating point to create mode mismatches */
    return x * y + (double)z;
}

int main(void) {
    /* Seed random for varied initialization */
    srand(time(NULL));
    
    /* Create massive register pressure with many live variables */
    /* Use volatile on some to prevent optimization */
    volatile long v1 = rand() % 100;
    long v2 = rand() % 100 + 1;  /* +1 to avoid division by zero */
    register long v3 asm ("r12") = rand() % 100;  /* Pin to specific register */
    long v4 = rand() % 100;
    volatile long v5 = rand() % 100;
    long v6 = rand() % 100 + 1;
    long v7 = rand() % 100;
    long v8 = rand() % 100;
    long v9 = rand() % 100;
    long v10 = rand() % 100;
    long v11 = rand() % 100;
    long v12 = rand() % 100;
    long v13 = rand() % 100;
    long v14 = rand() % 100;
    long v15 = rand() % 100;
    long v16 = rand() % 100;
    long v17 = rand() % 100;
    long v18 = rand() % 100;
    long v19 = rand() % 100;
    long v20 = rand() % 100;
    
    /* Mix in some floating point for mode mismatches */
    double f1 = (double)v1;
    double f2 = (double)v2;
    float f3 = (float)v3;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Clobber multiple specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v4)      /* Output operand */
        : [in1] "r" (v1),      /* Input operand 1 */
          [in2] "r" (v2)       /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobber specific registers */
    );
    
    /* Another asm with different constraints */
    register long pinned asm ("r13") = v3;
    asm volatile (
        "imulq %[in], %[pinned]\n\t"
        : [pinned] "+r" (pinned)
        : [in] "r" (v5)
        : "cc"
    );
    v3 = pinned;
    
    /* Complex expression using many variables - creates register pressure */
    long result = v1 + v2 * v3 - v4 / v6 + v7 % v8;
    result += (v9 << 2) | (v10 >> 1);
    result ^= v11 & v12 | v13;
    result = (v14 * v15) + (v16 - v17) * (v18 + v19) / v20;
    
    /* Force address reloads with complex array indexing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Complex addressing mode that may require reloads */
    long idx = v1 + v2 * v3 - v4;
    volatile long array_access = arr[(idx + v5 * v6) % 100];
    
    /* Mix integer and floating point operations */
    double mixed = fp_helper(f1, f2, (int)v3);
    f3 = (float)mixed + f3 * 2.0f;
    
    /* Force function call with many parameters - uses register passing in SysV ABI */
    long call_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile condition to prevent dead code elimination */
    volatile int condition = (result > 1000);
    if (condition) {
        result += call_result;
    } else {
        result -= call_result;
    }
    
    /* More inline assembly with mismatched sizes */
    short s1 = v1 & 0xFFFF;
    int i1 = v2;
    asm volatile (
        "movw %w[short_in], %%ax\n\t"
        "movl %[int_in], %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %[int_out]\n\t"
        : [int_out] "=r" (i1)
        : [short_in] "r" (s1),
          [int_in] "r" (i1)
        : "rax", "rbx", "cc"
    );
    
    /* Final complex computation using most variables */
    result = result + array_access + (long)mixed + i1 + v7 + v8 + v9 + v10;
    
    printf("Result: %ld\n", result);
    return 0;
}
