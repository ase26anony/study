/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__) || defined(__i386__)
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
    /* Clobber caller-saved registers for ARM64 */
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

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone))
int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed + 3;
    register int64_t d asm("") = seed * 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed + 7;
    register int64_t h asm("") = seed * 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    
    /* Complex computation before call - creates data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e / (f + 1);
    d = e * f + g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundary */
    if (local_flag) {
        /* Additional computation in the conditional path */
        e = f * g + h;
        f = g * h - i;
        
        /* The call that needs caller-save handling */
        callee_function();
        
        /* More computation after call - keeps variables live */
        g = h * i + j;
        h = i * j - a;
    } else {
        /* Alternative path without call */
        e = f * g - h;
        f = g * h + i;
        g = h * i - j;
        h = i * j + a;
    }
    
    /* Complex computation using all variables */
    i = j * a + b;
    j = a * b - c;
    
    /* Force all variables to be used in return value */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int seed) {
    register int64_t v1 asm("") = seed * 11;
    register int64_t v2 asm("") = seed + 12;
    register int64_t v3 asm("") = seed * 13;
    register int64_t v4 asm("") = seed + 14;
    register int64_t v5 asm("") = seed * 15;
    register int64_t v6 asm("") = seed + 16;
    register int64_t v7 asm("") = seed * 17;
    register int64_t v8 asm("") = seed + 18;
    
    /* Create data dependencies */
    v1 = v2 * v3;
    v2 = v3 + v4;
    
    /* Nested conditional to create complex CFG */
    if (global_flag > 0) {
        v3 = v4 * v5;
        if (global_flag < 10) {
            v4 = v5 + v6;
            callee_function();
            v5 = v6 * v7;
        } else {
            v4 = v5 - v6;
        }
        v6 = v7 + v8;
    }
    
    /* Use all variables */
    v7 = v8 * v1;
    v8 = v1 + v2;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to affect branch prediction */
    global_flag = 1;
    result1 = caller_function(42);
    
    global_flag = 0;
    result2 = caller_function(24);
    
    /* Call second function with different conditions */
    global_flag = 5;
    result1 += caller_function2(33);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %ld, %ld\n", (long)result1, (long)result2);
    
    return (result1 > result2) ? 0 : 1;
}
