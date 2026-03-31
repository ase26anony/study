/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void)
{
    /* Use inline assembly to clobber caller-saved registers */
    /* This forces the caller to save/restore these registers */
    
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
long caller_function(int param1, int param2, int param3)
{
    /* Declare many local variables to create register pressure */
    register long a __asm__("") = param1 * 3L;
    register long b __asm__("") = param2 * 5L;
    register long c __asm__("") = param3 * 7L;
    register long d __asm__("") = a + b + c;
    register long e __asm__("") = a * b - c;
    register long f __asm__("") = b * c + d;
    register long g __asm__("") = c * d - e;
    register long h __asm__("") = d * e + f;
    register long i __asm__("") = e * f - g;
    register long j __asm__("") = f * g + h;
    register long k __asm__("") = g * h - i;
    register long l __asm__("") = h * i + j;
    
    /* Force values to be computed and live in registers */
    volatile long temp __attribute__((unused));
    
    /* First computation phase - creates live values in registers */
    a = b * c + 12345;
    b = c * d - 67890;
    c = d * e + a;
    d = e * f - b;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates a basic block boundary and conditional path */
    if (global_volatile_flag) {
        /* Additional computation before call to create more register pressure */
        e = f * g + c;
        f = g * h - d;
        
        /* Call that clobbers caller-saved registers */
        callee_function();
        
        /* More computation after call - registers need to be restored */
        g = h * i + e;
        h = i * j - f;
    } else {
        /* Alternative path - different computation */
        e = f * g - c;
        f = g * h + d;
        g = h * i - e;
        h = i * j + f;
    }
    
    /* Second computation phase using all variables */
    /* Ensures all variables remain live across different paths */
    i = j * k + g;
    j = k * l - h;
    k = l * a + i;
    l = a * b - j;
    
    /* Complex final computation using all variables */
    /* Prevents dead code elimination */
    long result = (a + b) * (c - d) + (e * f) - (g / (h + 1)) 
                  + (i << 2) - (j >> 3) + (k & 0xFF) | (l ^ 0xFFFF);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use volatile store to force side effect */
    temp = result;
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
void create_register_pressure(void)
{
    /* Function with its own register usage */
    register long x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    register long x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    /* Force computation */
    x1 = x2 * x3 + x4;
    x2 = x3 * x4 - x5;
    x3 = x4 * x5 + x6;
    x4 = x5 * x6 - x7;
    x5 = x6 * x7 + x8;
    
    /* Volatile to prevent optimization */
    volatile long dummy = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    (void)dummy;
}

int main(void)
{
    long total = 0;
    
    /* Call multiple times with different parameters */
    /* This creates different register allocation patterns */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the volatile flag occasionally */
        if (iter % 7 == 0) {
            global_volatile_flag = !global_volatile_flag;
        }
        
        /* Create additional register pressure before call */
        create_register_pressure();
        
        /* Call the target function with varying parameters */
        long result = caller_function(iter, iter + 1, iter + 2);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Occasionally call callee directly to affect optimization decisions */
        if (iter % 13 == 0) {
            callee_function();
        }
    }
    
    printf("Total result: %ld\n", total);
    
    /* Dump RTL if compiled with -fdump-rtl-caller-save */
    #ifdef __GNUC__
    /* Force a compiler barrier */
    asm volatile("" : : : "memory");
    #endif
    
    return total != 0 ? 0 : 1;
}
