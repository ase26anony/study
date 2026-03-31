/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create conditional call */
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
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across a call */
__attribute__((noipa, noinline))
int64_t caller_function(int64_t seed) {
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
    
    /* Pre-call computations - create data dependencies */
    a = b * c + d;
    b = c + d * e;
    c = d - e / (f + 1);
    d = e ^ f | g;
    
    /* Memory barrier to force values to be live in registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates a basic block boundary opportunity */
    if (global_volatile_flag) {
        /* The call instruction will be 'insn' in the uncovered code */
        callee_function();
    }
    
    /* Post-call computations using the same variables */
    /* Ensures variables are live across the call */
    e = f * g + h;
    f = g + h * i;
    g = h - i / (j + 1);
    h = i ^ j | a;
    
    /* More computations to increase register pressure */
    i = j * a + b;
    j = a + b * c;
    
    /* Complex return value using all variables */
    /* Prevents dead code elimination */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + 
           f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Another caller function with different pattern to increase chances */
__attribute__((noipa, noinline))
int64_t caller_function2(int64_t seed) {
    volatile int local_flag = global_volatile_flag;
    int64_t vars[12];
    
    /* Initialize array elements with computations */
    for (int k = 0; k < 12; k++) {
        vars[k] = seed * k + k * k;
    }
    
    /* Pre-call computation chain */
    vars[0] = vars[1] * vars[2] + vars[3];
    vars[1] = vars[2] + vars[3] * vars[4];
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Nested conditional to create more complex CFG */
    if (local_flag) {
        if (seed > 0) {
            callee_function();
        } else {
            vars[2] = vars[3] - vars[4];
        }
    }
    
    /* Post-call computation chain */
    vars[3] = vars[4] * vars[5] + vars[6];
    vars[4] = vars[5] + vars[6] * vars[7];
    
    /* Use all variables in return */
    int64_t sum = 0;
    for (int k = 0; k < 12; k++) {
        sum += vars[k] * (k + 1);
    }
    return sum;
}

int main(void) {
    int64_t result1, result2;
    
    /* Call first function multiple times with different seeds */
    for (int iter = 0; iter < 100; iter++) {
        global_volatile_flag = iter % 3;  /* Change flag periodically */
        result1 = caller_function(iter * 17 + 123);
        result2 = caller_function2(iter * 23 + 456);
        
        /* Use results to prevent optimization */
        if (result1 > 1000000 || result2 > 1000000) {
            printf("Iteration %d: %lld %lld\n", iter, 
                   (long long)result1, (long long)result2);
        }
    }
    
    return 0;
}
