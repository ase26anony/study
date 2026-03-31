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
    /* This forces the compiler to save/restore these registers around the call */
    
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
    /* Generic memory clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2, int param3)
{
    /* Declare many local variables to create register pressure */
    /* Use 'register' keyword to encourage register allocation */
    register long a = param1 * 3;
    register long b = param2 * 5;
    register long c = param3 * 7;
    register long d = a + b + c;
    register long e = a * b - c;
    register long f = b * c + d;
    register long g = c * d - e;
    register long h = d * e + f;
    register long i = e * f - g;
    register long j = f * g + h;
    
    /* Perform computations before the call */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates a basic block boundary and conditional path */
    if (global_volatile_flag) {
        /* Additional computation right before call to create
           save/restore placement opportunities */
        d = e * f - g;
        e = f * g + h;
        
        /* The call that will trigger caller-save optimization */
        callee_function();
        
        /* Additional computation right after call */
        f = g * h - i;
    } else {
        /* Alternative path without call */
        d = g * h + i;
        e = h * i - j;
    }
    
    /* More computations using all variables after the call */
    /* This ensures variables stay live across the call */
    g = h * i + j;
    h = i * j - a;
    i = j * a + b;
    j = a * b - c;
    
    /* Complex expression using all variables to prevent dead code elimination */
    long result = (a * b) + (c * d) - (e * f) + (g * h) - (i * j);
    result += (a + b + c + d + e + f + g + h + i + j);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure in caller */
__attribute__((noinline))
int helper_computation(int x, int y)
{
    return x * y + (x ^ y) - (x & y);
}

int main(void)
{
    int i;
    long total = 0;
    
    /* Vary the volatile flag to create different execution paths */
    for (i = 0; i < 100; i++) {
        global_volatile_flag = (i % 3) != 0;  /* Mix of true/false */
        
        /* Call with different parameters to prevent constant propagation */
        long result = caller_function(
            helper_computation(i, 1),
            helper_computation(i, 2),
            helper_computation(i, 3)
        );
        
        total += result;
        
        /* Use result to prevent dead code elimination */
        if (result > 1000000) {
            printf("Large result: %ld\n", result);
        }
    }
    
    printf("Total: %ld\n", total);
    return total > 0 ? 0 : 1;
}
