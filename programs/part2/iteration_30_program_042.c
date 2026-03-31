/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Force clobbering of caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    /* ARM64 caller-saved registers */
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov x0, #0\n\t"
        "mov x1, #0\n\t"
        "mov x2, #0\n\t"
        "mov x3, #0\n\t"
        "mov x4, #0\n\t"
        "mov x5, #0\n\t"
        "mov x6, #0\n\t"
        "mov x7, #0\n\t"
        "mov x8, #0\n\t"
        "mov x9, #0\n\t"
        "mov x10, #0\n\t"
        "mov x11, #0\n\t"
        "mov x12, #0\n\t"
        "mov x13, #0\n\t"
        "mov x14, #0\n\t"
        "mov x15, #0\n\t"
        "mov x16, #0\n\t"
        "mov x17, #0\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-ipa-ra")))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a asm("") = param1 * 3L;
    register long b asm("") = param2 * 5L;
    register long c asm("") = a + b;
    register long d asm("") = param1 - param2;
    register long e asm("") = a * b;
    register long f asm("") = c + d;
    register long g asm("") = e - f;
    register long h asm("") = d * 7L;
    register long i asm("") = param1 * param2;
    register long j asm("") = h + i;
    
    /* Complex computation before call */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call - creates basic block boundaries */
    if (global_flag) {
        /* Additional computation in the conditional path */
        e = f * g + h;
        f = g * h - i;
        
        /* The call that needs caller-save handling */
        callee_function();
        
        /* More computation after call in same basic block */
        g = h * i + j;
        h = i * j - a;
    } else {
        /* Alternative path without call */
        e = f * g - h;
        f = g * h + i;
        g = h * i - j;
        h = i * j + a;
    }
    
    /* Complex post-call computation ensuring all vars are live */
    i = j * a + b;
    j = a * b - c;
    
    /* Mix all variables to ensure they're all used */
    long result = (a + b) * (c - d) + (e * f) - (g / (h != 0 ? h : 1)) + (i ^ j);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another caller with different pattern to increase chances */
__attribute__((noipa, noinline, optimize("no-ipa-ra")))
long caller_function2(int param) {
    register long v1 asm("") = param * 2L;
    register long v2 asm("") = param * 3L;
    register long v3 asm("") = v1 + v2;
    register long v4 asm("") = v1 - v2;
    register long v5 asm("") = v3 * v4;
    register long v6 asm("") = v5 + param;
    register long v7 asm("") = v6 * 11L;
    register long v8 asm("") = v7 - v5;
    register long v9 asm("") = v8 / 3L;
    register long v10 asm("") = v9 ^ v1;
    
    /* Create data dependencies */
    v1 = v2 + v3;
    v2 = v3 - v4;
    
    /* Nested conditional with call */
    if (global_flag > 0) {
        v3 = v4 * v5;
        if (param % 2) {
            v4 = v5 + v6;
            callee_function();
            v5 = v6 - v7;
        } else {
            v4 = v5 - v6;
        }
        v6 = v7 * v8;
    }
    
    /* Ensure all variables are live and used */
    v7 = v8 + v9;
    v8 = v9 - v10;
    v9 = v10 * v1;
    v10 = v1 / (v2 != 0 ? v2 : 1);
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

int main(void) {
    long total = 0;
    
    /* Call multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        global_flag = i % 3;  /* Change flag to create different paths */
        total += caller_function(i, i + 1);
        total += caller_function2(i);
        
        /* Prevent loop unrolling from simplifying register pressure */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        return 0;
    }
    return 1;
}
