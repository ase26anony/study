/* test_caller_save.c - Program to trigger specific RTL instruction reordering */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create unpredictable condition */
volatile int global_flag = 1;

/* Function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers with inline assembly */
#if defined(__x86_64__)
    asm volatile (
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x11111111, %%rdx\n\t"
        "mov $0x22222222, %%rsi\n\t"
        "mov $0x33333333, %%rdi\n\t"
        "mov $0x44444444, %%r8\n\t"
        "mov $0x55555555, %%r9\n\t"
        "mov $0x66666666, %%r10\n\t"
        "mov $0x77777777, %%r11\n\t"
        "add %%rcx, %%rax\n\t"
        "add %%rdx, %%rax\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    asm volatile (
        "mov x0, #0x1234\n\t"
        "mov x1, #0x5678\n\t"
        "mov x2, #0x9abc\n\t"
        "mov x3, #0xdef0\n\t"
        "mov x4, #0x1111\n\t"
        "mov x5, #0x2222\n\t"
        "mov x6, #0x3333\n\t"
        "mov x7, #0x4444\n\t"
        "mov x8, #0x5555\n\t"
        "mov x9, #0x6666\n\t"
        "mov x10, #0x7777\n\t"
        "mov x11, #0x8888\n\t"
        "mov x12, #0x9999\n\t"
        "mov x13, #0xaaaa\n\t"
        "mov x14, #0xbbbb\n\t"
        "mov x15, #0xcccc\n\t"
        "add x0, x0, x1\n\t"
        "add x0, x0, x2\n\t"
        : /* no outputs */
        : /* no inputs */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", 
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory barrier for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
int64_t caller_function(int seed) {
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
    register int64_t k asm("") = seed / 11;
    register int64_t l asm("") = seed - 12;
    
    /* Force variables to be in registers with computations */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile store to create side effect */
    volatile int64_t temp = a + b;
    
    /* Conditional call - creates basic block boundary */
    if (global_flag) {
        /* Additional computation before call to create live values */
        e = f * g + h;
        f = g * h - i;
        
        /* Call that clobbers caller-saved registers */
        callee_function();
        
        /* More computations after call - variables still live */
        g = h * i + j;
        h = i * j - k;
    } else {
        /* Alternative path with different computations */
        e = seed * 100;
        f = seed * 200;
        g = seed * 300;
        h = seed * 400;
    }
    
    /* Complex computation using all variables */
    i = j * k + l;
    j = k * l - a;
    k = l * a + b;
    l = a * b - c;
    
    /* Final computation that uses all variables */
    int64_t result = a + b + c + d + e + f + g + h + i + j + k + l;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another caller to create more complex CFG */
__attribute__((noipa, noinline))
int64_t caller_function2(int seed) {
    register int64_t m = seed * 13;
    register int64_t n = seed / 14;
    register int64_t o = seed + 15;
    register int64_t p = seed - 16;
    
    m = n * o + p;
    n = o * p - m;
    
    if (global_flag > 0) {
        o = p * m + n;
        callee_function();
        p = m * n - o;
    }
    
    return m + n + o + p;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to affect branch prediction */
    global_flag = 1;
    
    /* Call with different seeds to create varying register usage */
    result1 = caller_function(42);
    result2 = caller_function2(24);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %ld, %ld\n", (long)result1, (long)result2);
    
    /* Change flag and call again */
    global_flag = 0;
    result1 = caller_function(100);
    result2 = caller_function2(200);
    
    printf("Results2: %ld, %ld\n", (long)result1, (long)result2);
    
    return 0;
}
