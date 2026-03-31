/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void)
{
    /* Use inline assembly to clobber caller-saved registers */
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
__attribute__((noipa, noinline))
long caller_function(int x, int y)
{
    /* Declare many local variables to create register pressure */
    register long a = x + 1;
    register long b = y * 2;
    register long c = a * b + 3;
    register long d = c - a + b;
    register long e = d * 3 - c;
    register long f = e + a * 2;
    register long g = f - b + d;
    register long h = g * a - f;
    register long i = h + c * 2;
    register long j = i - d + e;
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Complex computation before call */
    a = b * c + d - e;
    b = c * d - e + f;
    c = d * e - f + g;
    d = e * f - g + h;
    
    /* Use volatile to prevent reordering */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag) {
        /* Additional computation in the conditional block */
        e = f * g - h + i;
        f = g * h - i + j;
        
        /* The critical call */
        callee_function();
        
        /* More computation after call in same basic block */
        g = h * i - j + a;
        h = i * j - a + b;
    } else {
        /* Alternative path without call */
        e = f + g + h;
        f = g + h + i;
        g = h + i + j;
        h = i + j + a;
    }
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex computation using all variables after call */
    i = j * a - b + c;
    j = a * b - c + d;
    a = b * c - d + e;
    b = c * d - e + f;
    c = d * e - f + g;
    d = e * f - g + h;
    e = f * g - h + i;
    f = g * h - i + j;
    
    /* Ensure all variables are live and used */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline))
long caller_function2(int x, int y, int z)
{
    register long v1 = x * y + z;
    register long v2 = y * z + x;
    register long v3 = z * x + y;
    register long v4 = v1 + v2 - v3;
    register long v5 = v2 + v3 - v1;
    register long v6 = v3 + v1 - v2;
    register long v7 = v4 * v5 - v6;
    register long v8 = v5 * v6 - v7;
    register long v9 = v6 * v7 - v8;
    register long v10 = v7 * v8 - v9;
    
    /* Varying the condition */
    volatile int cond = global_flag & 1;
    
    if (cond) {
        v1 = v2 * v3 + v4;
        v2 = v3 * v4 + v5;
        callee_function();
        v3 = v4 * v5 + v6;
        v4 = v5 * v6 + v7;
    } else {
        v1 = v10 - v9;
        v2 = v9 - v8;
        callee_function();
        v3 = v8 - v7;
        v4 = v7 - v6;
    }
    
    /* Cross-dependent computations */
    v5 = v1 * v2 + v3 * v4;
    v6 = v2 * v3 + v4 * v5;
    v7 = v3 * v4 + v5 * v6;
    v8 = v4 * v5 + v6 * v7;
    v9 = v5 * v6 + v7 * v8;
    v10 = v6 * v7 + v8 * v9;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

int main(void)
{
    long result1, result2;
    int i;
    
    /* Vary the global flag to affect code paths */
    for (i = 0; i < 10; i++) {
        global_flag = i & 3;  /* Cycle through 0,1,2,3 */
        
        /* Call both functions to create different patterns */
        result1 = caller_function(i, i + 1);
        result2 = caller_function2(i, i + 2, i + 3);
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: %ld, %ld\n", i, result1, result2);
    }
    
    return 0;
}
