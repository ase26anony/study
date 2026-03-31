/* reload_test.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b - c * d / e + f;
    return result;
}

int main(void) {
    /* Seed random for variable initialization */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile long v1 = rand() % 100;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    long v5 = v4 / 3;
    long v6 = v5 * 7;
    long v7 = v6 + 11;
    long v8 = v7 - 13;
    long v9 = v8 * 17;
    long v10 = v9 / 19;
    long v11 = v10 + 23;
    long v12 = v11 - 29;
    long v13 = v12 * 31;
    long v14 = v13 / 37;
    long v15 = v14 + 41;
    long v16 = v15 - 43;
    long v17 = v16 * 47;
    long v18 = v17 / 53;
    long v19 = v18 + 59;
    long v20 = v19 - 61;
    
    /* Force mode mismatches */
    char c1 = v1 & 0xFF;      /* Narrow mode */
    short s1 = v2 & 0xFFFF;   /* Intermediate mode */
    int i1 = v3;              /* 32-bit mode */
    long l1 = v4;             /* 64-bit mode */
    
    /* Complex memory addressing - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    long complex_addr = arr[(v1 + v2 * v3 - v4 / (v5 + 1)) % 100];
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and potential conflicts */
    asm volatile (
        "movq %[in], %%rax\n\t"           /* Input to rax */
        "addq %%r12, %%rax\n\t"           /* Use pinned register r12 */
        "movq %%rax, %[out]\n\t"          /* Output from rax */
        : [out] "=r" (v20)                /* Output operand */
        : [in] "r" (v19), "r" (v2)        /* Input operands */
        : "rax", "rbx", "rcx", "rdx",     /* Clobber specific registers */
          "rsi", "rdi", "r8", "r9", "r10",
          "r11", "r13", "r14", "r15",
          "cc", "memory"                  /* Clobber condition codes and memory */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "imulq %1, %0\n\t"
        : "=a" (temp)                     /* Fixed output in rax */
        : "r" (v18), "0" (v17)            /* Input: v18 in any reg, v17 in rax */
        : "rdx"                           /* imul clobbers rdx */
    );
    
    /* Complex expression using most variables - maximizes live ranges */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 - v8 * v9 / (v10 + 1) +
         v11 + v12 - v13 * v14 / (v15 + 1) + v16 - v17 * v18 / (v19 + 1) +
         v20 + complex_addr;
    
    /* Mix different data types to force mode conversions */
    double d1 = v1 * 1.5;                 /* Integer to float conversion */
    float f1 = d1 / 2.0;                  /* Double to float conversion */
    int i2 = f1;                          /* Float to int conversion */
    
    /* Force conditional with volatile to prevent elimination */
    volatile int condition = (v1 > 1000);
    if (condition) {
        /* Use inline asm with specific register constraints */
        register long a asm ("rax") = v1;
        register long b asm ("rbx") = v2;
        asm volatile (
            "xchgq %%rbx, %%rax\n\t"
            : "+r" (a), "+r" (b)
            :
            : "cc"
        );
        v1 = a;
        v2 = b;
    }
    
    /* Call helper function - forces parameter passing in registers */
    long result = helper_func(v1, v2, v3, v4, v5, v6);
    result += helper_func(v7, v8, v9, v10, v11, v12);
    result += helper_func(v13, v14, v15, v16, v17, v18);
    
    /* More register pressure with pointer arithmetic */
    long *ptr1 = &v1;
    long *ptr2 = &v2;
    long *ptr3 = &v3;
    long *ptr4 = &v4;
    long *ptr5 = &v5;
    
    /* Complex pointer chain */
    long ptr_result = *ptr1 + *ptr2 * *ptr3 - *ptr4 / *ptr5;
    
    /* Final computation to ensure nothing is optimized away */
    long final_result = result + ptr_result + i2 + (long)f1 + (long)d1;
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
