/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create conditional call */
volatile int global_volatile_flag = 1;

/* Prevent interprocedural optimizations */
#ifdef __GNUC__
#define NOIPA __attribute__((noipa, noinline, noclone))
#else
#define NOIPA
#endif

/* Callee function that clobbers caller-saved registers */
NOIPA void callee_function(void) {
    /* Force register clobbering with inline assembly */
#if defined(__x86_64__) || defined(__i386__)
    /* Clobber common caller-saved registers for x86/x86_64 */
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
    /* Clobber caller-saved registers for ARM64 */
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
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across conditional call */
NOIPA int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a = seed + 1;
    register int64_t b = seed * 2;
    register int64_t c = seed + 3;
    register int64_t d = seed * 4;
    register int64_t e = seed + 5;
    register int64_t f = seed * 6;
    register int64_t g = seed + 7;
    register int64_t h = seed * 8;
    register int64_t i = seed + 9;
    register int64_t j = seed * 10;
    
    /* Complex computation before call to ensure variables are live in registers */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering but keep values in registers */
    asm volatile("" : : : "memory");
    
    /* Volatile read to create side effect */
    volatile int local_flag = global_volatile_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag) {
        /* Additional computation in the conditional path */
        e = f * g + h;
        f = g * h - i;
        
        /* The call that will need caller-saved register preservation */
        callee_function();
        
        /* More computation after call, using same variables */
        g = h * i + j;
        h = i * j - a;
    } else {
        /* Alternative path without call */
        e = f * g - h;
        f = g * h + i;
        g = h * i - j;
        h = i * j + a;
    }
    
    /* Complex post-call computation ensuring all variables remain live */
    i = j * a + b;
    j = a * b - c;
    
    /* Additional arithmetic mixing all variables */
    a = a + b + c + d;
    b = b - c + d - e;
    c = c * d * e * f;
    d = d / (e > 0 ? e : 1) + f;
    
    /* Final computation using all variables */
    int64_t result = a + b + c + d + e + f + g + h + i + j;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
NOIPA int64_t helper_computation(int64_t x, int64_t y) {
    register int64_t t1 = x * y;
    register int64_t t2 = x + y;
    register int64_t t3 = x - y;
    register int64_t t4 = x ^ y;
    register int64_t t5 = x | y;
    
    asm volatile("" : : : "memory");
    
    return t1 + t2 + t3 + t4 + t5;
}

int main(void) {
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different conditions */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Vary the global flag to create different execution paths */
        global_volatile_flag = iteration % 3;
        
        /* Call the function with different seeds */
        int64_t result = caller_function(iteration);
        
        /* Additional computation to prevent dead code elimination */
        result = helper_computation(result, iteration);
        
        total += result;
        
        /* Print periodically to create observable side effect */
        if (iteration % 25 == 0) {
            printf("Iteration %d: result = %ld, total = %ld\n", 
                   iteration, result, total);
        }
    }
    
    printf("Final total: %ld\n", total);
    
    /* Use the result to prevent optimization */
    if (total > 1000) {
        return 0;
    } else {
        return 1;
    }
}
