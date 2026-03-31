/* test-caller-save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create conditional call path */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Use inline assembly to clobber caller-saved registers */
    /* This forces the compiler to save/restore these registers around the call */
    
#if defined(__x86_64__)
    /* x86_64 caller-saved registers */
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
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across a conditional call */
__attribute__((noipa, noinline, optimize("no-inline")))
int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    /* Use 'register' keyword to encourage register allocation */
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
    register int64_t k = seed + 11;
    register int64_t l = seed * 12;
    
    /* Perform computations before the call to make variables live */
    a = b * c + d;
    e = f - g * h;
    i = j / (k + 1) + l;
    
    /* Memory barrier to prevent reordering and ensure values are in registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag - creates basic block boundary */
    if (global_volatile_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
        
        /* The caller-save pass may need to insert save/restore around this call */
        /* and potentially reorder instructions */
    }
    
    /* More computations after the call using the same variables */
    /* This ensures variables are live across the call */
    b = a * e + i;
    c = d - f * g;
    h = j * k - l;
    
    /* Complex computation to prevent dead code elimination */
    int64_t result = a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + 
                     g * 7 + h * 8 + i * 9 + j * 10 + k * 11 + l * 12;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional control flow complexity */
__attribute__((noinline))
int64_t helper_function(int seed) {
    /* Different computation to create different register usage pattern */
    return seed * 123456789;
}

int main(void) {
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different conditions */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the global flag to create different execution paths */
        global_volatile_flag = (iter % 3) != 0;
        
        /* Call the function with different seeds to create varying register usage */
        int64_t result = caller_function(iter);
        
        /* Mix in helper function calls to create more register pressure */
        if (iter % 2 == 0) {
            result += helper_function(iter);
        }
        
        total += result;
        
        /* Use result to prevent dead code elimination */
        if (result > 1000000) {
            global_volatile_flag = 0;  /* Modify volatile variable */
        }
    }
    
    printf("Total: %ld\n", (long)total);
    
    /* Additional test with inline assembly to force specific register usage */
    {
        int64_t x = 42;
        int64_t y = 123;
        
        /* Force specific values into registers */
        asm volatile (
            "# Force register usage\n\t"
            : "+r" (x), "+r" (y)
            :
            : "memory"
        );
        
        /* Call with forced register values */
        total += caller_function(x + y);
    }
    
    return (int)(total % 1000);
}
