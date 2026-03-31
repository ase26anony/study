/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, target("no-sse")))
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
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across a conditional call */
__attribute__((noipa, noinline, optimize("no-inline")))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a asm("") = param1 * 3L;
    register long b asm("") = param2 * 5L;
    register long c asm("") = a + b + 7L;
    register long d asm("") = a * b - 11L;
    register long e asm("") = (a << 3) | (b & 0xFF);
    register long f asm("") = d ^ e;
    register long g asm("") = c * d / (e + 1);
    register long h asm("") = f + g * 2;
    register long i asm("") = (a + b + c + d) & 0xFFFF;
    register long j asm("") = (e ^ f ^ g ^ h) | i;
    
    /* Create data dependencies before the call */
    a = b + c * 2;
    b = d - e / 3;
    c = f ^ g;
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int flag = global_volatile_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (flag & 0x1) {
        /* Additional computation in the conditional path */
        d = e * 2 + 1;
        e = f - g;
        
        /* The critical call that will trigger caller-save */
        callee_function();
        
        /* More computation after the call in same basic block */
        f = g + h * 3;
    } else {
        /* Alternative path without call */
        d = e * 3 - 1;
        e = f + g;
        f = g - h / 2;
    }
    
    /* Additional memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computation using all variables */
    g = h * i - j;
    h = (a & b) | (c & d);
    i = (e ^ f) + (g ^ h);
    j = a * b * c * d / (e + f + g + h + i + 1);
    
    /* Mix all variables to ensure they're all live */
    long result = a + b * 2 + c * 3 + d * 4 + e * 5 + 
                  f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
    
    /* Another volatile operation */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
long helper_computation(long x, long y) {
    return (x * y) + (x ^ y) - (x & y);
}

int main(void) {
    long total = 0;
    
    /* Loop to create multiple call sites with different conditions */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the global flag to affect conditional paths */
        global_volatile_flag = iter & 0x3;
        
        /* Call with different parameters to prevent constant propagation */
        long result = caller_function(iter, iter * 2 + 1);
        
        /* Additional computation to use result */
        total += helper_computation(result, iter);
        
        /* Occasionally modify the flag mid-loop */
        if (iter % 7 == 0) {
            global_volatile_flag ^= 1;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    /* Additional test with different parameter patterns */
    for (int pattern = 0; pattern < 10; pattern++) {
        global_volatile_flag = pattern;
        long r1 = caller_function(pattern * 10, pattern * 20 + 5);
        long r2 = caller_function(pattern * 30 + 1, pattern * 40 + 3);
        printf("Pattern %d: %ld, %ld\n", pattern, r1, r2);
    }
    
    return 0;
}
