/* test_caller_save.c - Program to trigger specific RTL list manipulation in GCC's caller-save pass */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Inline assembly to clobber caller-saved registers */
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
        :
        : 
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
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure across call */
__attribute__((noipa, noinline, noclone))
int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a = seed * 1;
    register int64_t b = seed * 2 + 1;
    register int64_t c = seed * 3 + 2;
    register int64_t d = seed * 4 + 3;
    register int64_t e = seed * 5 + 4;
    register int64_t f = seed * 6 + 5;
    register int64_t g = seed * 7 + 6;
    register int64_t h = seed * 8 + 7;
    register int64_t i = seed * 9 + 8;
    register int64_t j = seed * 10 + 9;
    
    /* Complex computation before call to ensure values are live */
    a = b * c + d - e;
    b = c * d / (e + 1) + f;
    c = d * e - f * g;
    d = e * f + g * h;
    e = f * g - h * i;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag > 0) {
        /* This call creates the need for caller-save */
        callee_function();
    }
    
    /* More computations after call - variables still live */
    f = g * h + i * j;
    g = h * i - j * a;
    h = i * j + a * b;
    i = j * a - b * c;
    j = a * b + c * d;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + 
           (a * b) - (c * d) + (e * f) - (g * h) + (i * j);
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int seed) {
    register int64_t v1 = seed * 11;
    register int64_t v2 = seed * 12;
    register int64_t v3 = seed * 13;
    register int64_t v4 = seed * 14;
    register int64_t v5 = seed * 15;
    register int64_t v6 = seed * 16;
    register int64_t v7 = seed * 17;
    register int64_t v8 = seed * 18;
    
    v1 = v2 * v3 + v4;
    v2 = v3 * v4 - v5;
    
    /* Nested conditional to create more complex CFG */
    if (global_flag & 1) {
        if (global_flag & 2) {
            callee_function();
        } else {
            v3 = v4 * v5 + v6;
            asm volatile("" : : : "memory");
        }
    }
    
    v4 = v5 * v6 - v7;
    v5 = v6 * v7 + v8;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the global flag to prevent constant propagation */
    global_flag = 1;
    result1 = caller_function(42);
    
    global_flag = 2;
    result2 = caller_function2(24);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %lld, %lld\n", (long long)result1, (long long)result2);
    
    /* Additional test with different seed */
    global_flag = 3;
    result1 = caller_function(100);
    result2 = caller_function2(200);
    printf("Results2: %lld, %lld\n", (long long)result1, (long long)result2);
    
    return 0;
}
