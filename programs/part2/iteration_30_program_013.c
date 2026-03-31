/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create conditional call */
volatile int global_volatile_flag = 1;

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

/* Caller function with high register pressure across a call */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2)
{
    /* Declare many local variables to create register pressure */
    register long a = param1 * 3;
    register long b = param2 * 7;
    register long c = a + b * 2;
    register long d = c - param1 * 5;
    register long e = d * 3 + 11;
    register long f = e - b / 2;
    register long g = f * a + 17;
    register long h = g - c * 2;
    register long i = h + d * 3;
    register long j = i - e * 4;
    
    /* First computation phase - creates data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates the basic block structure needed for the uncovered code */
    if (global_volatile_flag) {
        /* Call that clobbers caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase - variables still live after call */
    /* Complex data dependencies to prevent easy optimization */
    d = e * f - g + a;
    e = f * g + h - b;
    f = g * h + i * c;
    g = h * i - j * d;
    h = i * j + a * e;
    
    /* More memory barriers to constrain optimization */
    asm volatile("" : : : "memory");
    
    /* Final computation using all variables */
    long result = a + b * 2 + c * 3 + d * 4 + e * 5 + 
                  f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile long sink = result;
    asm volatile("" : : "r"(sink) : "memory");
    
    return result;
}

/* Another caller with different pattern to increase coverage */
__attribute__((noipa, noinline))
long alternate_caller(int x, int y)
{
    /* Different register usage pattern */
    register long v1 = x * 2;
    register long v2 = y * 3;
    register long v3 = v1 + v2;
    register long v4 = v2 - v1;
    register long v5 = v3 * v4;
    register long v6 = v5 / 7;
    register long v7 = v6 + 19;
    register long v8 = v7 - 23;
    
    /* Create data flow that encourages save/restore reordering */
    for (int k = 0; k < 3; k++) {
        v1 = v2 + v3 * k;
        v2 = v3 - v4 / (k + 1);
        
        /* Conditional call inside loop */
        if (global_volatile_flag & (1 << k)) {
            callee_function();
        }
        
        v3 = v4 + v5 * (k + 2);
        v4 = v5 - v6 / (k + 3);
    }
    
    /* Complex return expression */
    return v1 * v2 + v3 * v4 - v5 * v6 + v7 * v8;
}

int main(void)
{
    /* Initialize with non-trivial values */
    int seed = 42;
    
    /* Call the first function */
    long result1 = caller_function(seed, seed * 2);
    
    /* Modify volatile flag */
    global_volatile_flag = 0;
    
    /* Call the second function */
    long result2 = alternate_caller(seed, seed * 3);
    
    /* Use results to prevent optimization */
    printf("Results: %ld, %ld\n", result1, result2);
    
    /* Additional test with different parameters */
    for (int i = 0; i < 5; i++) {
        global_volatile_flag = i;
        long r = caller_function(i, i * 2);
        printf("Iteration %d: %ld\n", i, r);
    }
    
    return 0;
}
