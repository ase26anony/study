/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to make conditional unpredictable */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__) || defined(__i386__)
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

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone, optimize("no-inline")))
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
    
    /* Complex computation before call to make variables live */
    a = b * c + d;
    b = c * d - e;
    c = d * e / (f + 1);
    d = e * f + g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_volatile_flag) {
        /* This creates a basic block boundary */
        callee_function();
    } else {
        /* Alternative path to create control flow complexity */
        asm volatile("" : : : "memory");
    }
    
    /* More computations after call, keeping variables live */
    e = f * g + h;
    f = g * h - i;
    g = h * i / (j + 1);
    h = i * j + a;
    
    /* Additional memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + 
           (a * b) - (c * d) + (e * f) - (g * h) + (i * j);
}

/* Another caller to create different call patterns */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int64_t seed) {
    register int64_t v1 asm("") = seed * 11;
    register int64_t v2 asm("") = seed + 12;
    register int64_t v3 asm("") = seed / 13;
    register int64_t v4 asm("") = seed - 14;
    register int64_t v5 asm("") = seed + 15;
    
    /* Nested conditionals to create complex CFG */
    if (global_volatile_flag & 1) {
        v1 = v2 * v3;
        if (global_volatile_flag & 2) {
            callee_function();
            v2 = v3 + v4;
        } else {
            v2 = v3 - v4;
        }
        callee_function();
    } else {
        v1 = v2 / v3;
    }
    
    v3 = v4 * v5;
    v4 = v5 + v1;
    
    return v1 + v2 + v3 + v4 + v5;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to create different execution paths */
    for (int iter = 0; iter < 100; iter++) {
        global_volatile_flag = iter % 3;
        
        /* Call both functions to increase coverage */
        result1 = caller_function(iter);
        result2 = caller_function2(iter + 1000);
        
        /* Use results to prevent elimination */
        if (result1 > result2) {
            printf("Iteration %d: %ld > %ld\n", iter, result1, result2);
        }
    }
    
    return 0;
}
