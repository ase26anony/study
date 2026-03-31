/* test-caller-save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create unpredictable conditional */
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
    /* Generic memory clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure across call */
__attribute__((noipa, noinline))
int64_t caller_function(int64_t seed)
{
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed / 7;
    register int64_t h asm("") = seed - 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    
    /* Pre-call computations creating data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e / (f + 1);
    d = e * f + g;
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates the basic block structure needed */
    if (global_volatile_flag) {
        /* Additional computation before call in same block */
        e = f * g - h;
        f = g * h + i;
        
        /* The critical call instruction */
        callee_function();
        
        /* Post-call computations in same basic block */
        g = h * i / (j + 1);
        h = i * j - a;
    } else {
        /* Alternative path without call */
        e = f * g + h;
        f = g * h - i;
        g = h * i * j;
        h = i * j + a;
    }
    
    /* More computations ensuring variables stay live */
    i = j * a + b;
    j = a * b - c;
    
    /* Complex final computation using all variables */
    /* This prevents dead code elimination */
    int64_t result = (a * b) + (c * d) - (e * f) + (g * h) - (i * j);
    result = result ^ (a + b + c + d + e + f + g + h + i + j);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
int64_t helper_computation(int64_t x, int64_t y)
{
    volatile int64_t temp = x;
    return temp * y + (x ^ y);
}

int main(void)
{
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different contexts */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the seed to create different register usage patterns */
        int64_t seed = iter * 1234567;
        
        /* Call with different arguments to prevent constant propagation */
        int64_t result = caller_function(seed + helper_computation(iter, iter * 2));
        
        /* Use result to prevent elimination */
        total += result;
        
        /* Toggle flag occasionally to exercise both paths */
        if (iter % 7 == 0) {
            global_volatile_flag = !global_volatile_flag;
        }
    }
    
    /* Print result to ensure side effects are visible */
    printf("Total: %ld\n", (long)total);
    
    return 0;
}
