/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Function that clobbers caller-saved registers */
void __attribute__((noipa, noinline)) callee_function(void) {
    /* Clobber caller-saved registers to force save/restore */
#if defined(__x86_64__)
    asm volatile(
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x11111111, %%rdx\n\t"
        "mov $0x22222222, %%rsi\n\t"
        "mov $0x33333333, %%rdi\n\t"
        "mov $0x44444444, %%r8\n\t"
        "mov $0x55555555, %%r9\n\t"
        "mov $0x66666666, %%r10\n\t"
        "mov $0x77777777, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    asm volatile(
        "mov x0, #0x1234\n\t"
        "mov x1, #0x5678\n\t"
        "mov x2, #0x9abc\n\t"
        "mov x3, #0xdef0\n\t"
        "mov x4, #0x1111\n\t"
        "mov x5, #0x2222\n\t"
        "mov x6, #0x3333\n\t"
        "mov x7, #0x4444\n\t"
        "mov x8, #0x5555\n\t"
        "mov x9, #0x6666\n\t"
        "mov x10, #0x7777\n\t"
        "mov x11, #0x8888\n\t"
        "mov x12, #0x9999\n\t"
        "mov x13, #0xaaaa\n\t"
        "mov x14, #0xbbbb\n\t"
        "mov x15, #0xcccc\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
int __attribute__((noipa, noinline)) caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register int a = param1 * 2;
    register int b = param2 + 3;
    register int c = a * b - 7;
    register int d = param1 ^ param2;
    register int e = d << 2;
    register int f = c + e;
    register int g = f * 3;
    register int h = g - a;
    register int i = h / 2;
    register int j = i | 0xFF;
    
    /* Additional variables to increase pressure further */
    register int k = j + a;
    register int l = k * b;
    register int m = l - c;
    register int n = m ^ d;
    register int o = n << 1;
    
    /* First computation using variables */
    int result1 = a + b + c + d + e;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation using the same variables */
    int result2 = f + g + h + i + j + k + l + m + n + o;
    
    /* More computations to ensure variables stay live */
    a = a + result1;
    b = b * result2;
    c = c ^ a;
    d = d | b;
    
    /* Final result depends on all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + result1 + result2;
}

/* Another caller to create different calling context */
int __attribute__((noipa, noinline)) caller_function2(int x, int y) {
    register int v1 = x * y;
    register int v2 = x + y;
    register int v3 = v1 - v2;
    register int v4 = v1 ^ v2;
    register int v5 = v3 << 1;
    register int v6 = v4 >> 1;
    register int v7 = v5 * v6;
    register int v8 = v7 + x;
    
    /* Create data dependencies */
    v1 = v1 + v8;
    v2 = v2 * v7;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call in a loop to potentially trigger different save placement */
    for (int i = 0; i < 2; i++) {
        if (global_flag & (1 << i)) {
            callee_function();
            v3 = v3 + i;
        }
    }
    
    /* Complex data flow to prevent optimization */
    v4 = v4 + v1;
    v5 = v5 ^ v2;
    v6 = v6 | v3;
    v7 = v7 - v4;
    v8 = v8 * v5;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    int total = 0;
    
    /* Call multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        global_flag = i & 3;  /* Change flag periodically */
        total += caller_function(i, i * 2);
        total += caller_function2(i, i + 1);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Also test with different optimization hints */
    volatile int test_var = 100;
    if (test_var > 50) {
        total += caller_function(test_var, test_var / 2);
    }
    
    return total > 0 ? 0 : 1;
}
