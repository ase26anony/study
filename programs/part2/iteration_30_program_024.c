/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers with inline assembly */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
    asm volatile(
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
    asm volatile(
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
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across a call */
__attribute__((noipa, noinline, optimize("no-inline")))
long caller_function(int param1, int param2, int param3) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 2;
    register long b = param2 + 100;
    register long c = param3 - 50;
    register long d = a * b;
    register long e = b + c;
    register long f = c * param1;
    register long g = param2 * param3;
    register long h = a + g;
    register long i = d - e;
    register long j = f * 3;
    
    /* Create data dependencies before the call */
    a = b * c + d;
    b = c - d + e;
    c = d * e - f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to create side effect */
    volatile int local_flag = global_volatile_flag;
    
    /* Conditional call with unpredictable branch */
    if (local_flag & 1) {
        /* Additional computation before call to create more register pressure */
        d = e * f + g;
        e = f - g + h;
        
        /* Call that clobbers caller-saved registers */
        callee_function();
        
        /* More computation after call, keeping variables live */
        f = g * h + i;
    } else {
        /* Alternative path without call */
        d = e * f - g;
        e = f + g - h;
        f = g * h - i;
    }
    
    /* Complex computation using all variables to ensure they stay live */
    g = h * i + j;
    h = i - j + a;
    i = j * a - b;
    j = a + b - c;
    
    /* Create cross-dependencies between all variables */
    a = b + c - d + e - f + g - h + i - j;
    b = c * d / (e + 1) + f - g * h;
    c = d + e + f + g + h + i + j;
    
    /* Final computation that uses all variables */
    long result = a + b * 2 - c * 3 + d * 4 - e * 5 + f * 6 - g * 7 + h * 8 - i * 9 + j * 10;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
int helper_computation(int x, int y) {
    return x * y + (x ^ y) - (x & y);
}

int main(void) {
    /* Initialize with non-trivial values */
    int base1 = 42;
    int base2 = 123;
    int base3 = 789;
    
    /* Create varying inputs to prevent constant propagation */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify global flag to affect branch prediction */
        global_volatile_flag = iter & 1;
        
        /* Call with different parameters each iteration */
        int p1 = base1 + iter;
        int p2 = base2 - iter;
        int p3 = base3 + (iter * 2);
        
        /* Additional computation to create register pressure in main too */
        int temp1 = helper_computation(p1, p2);
        int temp2 = helper_computation(p2, p3);
        int temp3 = helper_computation(p1, p3);
        
        /* Call the main function under test */
        long result = caller_function(temp1, temp2, temp3);
        
        /* Use the result to prevent dead code elimination */
        if (result > 1000000) {
            printf("Result[%d]: %ld\n", iter, result);
        }
        
        /* Modify bases to create different patterns */
        base1 = (base1 * 13 + 17) & 0xFF;
        base2 = (base2 * 29 + 31) & 0xFF;
        base3 = (base3 * 47 + 53) & 0xFF;
    }
    
    return 0;
}
