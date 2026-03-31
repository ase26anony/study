/* test-caller-save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
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
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", 
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure across a call */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2, int param3) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 2;
    register long b = param2 + 117;
    register long c = param3 - 42;
    register long d = a * b + c;
    register long e = b * c - a;
    register long f = c * a + b;
    register long g = d + e + f;
    register long h = a * b * c;
    register long i = (param1 << 3) | (param2 & 0xFF);
    register long j = param3 * 7 + 13;
    
    /* First computation phase - creates dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    
    /* Memory barrier to prevent reordering but keep values in registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_volatile_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase - uses same variables, keeping them live */
    d = e * f - g;
    e = f * g + h;
    f = g * h - i;
    g = h * i + j;
    
    /* More computations to ensure variables stay live */
    h = i * j - a;
    i = j * a + b;
    j = a * b - c;
    
    /* Complex return value using all variables */
    return (a + b) * (c - d) + (e * f) - (g / 2) + (h << 1) + (i >> 2) + (j * 3);
}

/* Another caller function to create different control flow pattern */
__attribute__((noipa, noinline))
long alternate_caller(int x, int y) {
    register long v1 = x * 3;
    register long v2 = y * 7;
    register long v3 = v1 + v2;
    register long v4 = v1 * v2;
    register long v5 = v2 - v1;
    register long v6 = v3 * v4;
    register long v7 = v4 / 3;
    register long v8 = v5 + 19;
    
    /* Create data dependencies */
    v1 = v2 * v3 + v4;
    v2 = v3 * v4 - v5;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call with different condition */
    if (global_volatile_flag & 1) {
        callee_function();
    } else {
        /* Alternative path that also uses registers */
        v3 = v4 * v5 + v6;
    }
    
    /* Continue computations */
    v4 = v5 * v6 - v7;
    v5 = v6 * v7 + v8;
    v6 = v7 * v8 - v1;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    long result1, result2;
    int i;
    
    /* Vary the global flag to affect conditional paths */
    for (i = 0; i < 10; i++) {
        global_volatile_flag = i & 1;
        
        /* Call both functions to exercise different paths */
        result1 = caller_function(i, i*2, i*3);
        result2 = alternate_caller(i+1, i*2+1);
        
        /* Use results to prevent dead code elimination */
        printf("Iteration %d: results = %ld, %ld\n", i, result1, result2);
        
        /* Occasionally change flag within loop */
        if (i == 5) {
            global_volatile_flag = 0;
        }
    }
    
    return 0;
}
