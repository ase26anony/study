/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers to force save/restore */
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
        "mov x0, #0x1111\n\t"
        "mov x1, #0x2222\n\t"
        "mov x2, #0x3333\n\t"
        "mov x3, #0x4444\n\t"
        "mov x4, #0x5555\n\t"
        "mov x5, #0x6666\n\t"
        "mov x6, #0x7777\n\t"
        "mov x7, #0x8888\n\t"
        "mov x8, #0x9999\n\t"
        "mov x9, #0xAAAA\n\t"
        "mov x10, #0xBBBB\n\t"
        "mov x11, #0xCCCC\n\t"
        "mov x12, #0xDDDD\n\t"
        "mov x13, #0xEEEE\n\t"
        "mov x14, #0xFFFF\n\t"
        "mov x15, #0x1234\n\t"
        "mov x16, #0x5678\n\t"
        "mov x17, #0x9ABC\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 3;
    register long b = param2 * 7;
    register long c = a + b;
    register long d = a * b;
    register long e = c - d;
    register long f = param1 * param2;
    register long g = f * 13;
    register long h = g / 5;
    register long i = h + 42;
    register long j = i * 2;
    register long k = j - 17;
    register long l = k + param1;
    register long m = l * param2;
    register long n = m / 3;
    
    /* Create data dependencies before the call */
    a = b + c;
    b = c + d;
    c = d + e;
    d = e + f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* More computations after the call - variables still live */
    e = f + g + a;
    f = g + h + b;
    g = h + i + c;
    h = i + j + d;
    i = j + k + e;
    j = k + l + f;
    k = l + m + g;
    l = m + n + h;
    
    /* Create complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

/* Another caller to create different basic block structure */
__attribute__((noipa, noinline))
long alternate_caller(int x, int y) {
    register long v1 = x * 11;
    register long v2 = y * 13;
    register long v3 = v1 + v2;
    register long v4 = v1 * v2;
    register long v5 = v3 - v4;
    register long v6 = x * y;
    register long v7 = v6 * 17;
    register long v8 = v7 / 7;
    
    /* Different computation pattern */
    for (int i = 0; i < 3; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        
        /* Nested conditional call */
        if (global_flag & (1 << i)) {
            callee_function();
            v3 = v4 + v5;
        } else {
            v3 = v5 + v6;
        }
        
        v4 = v5 + v6;
        v5 = v6 + v7;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    long result1, result2;
    
    /* Vary the global flag to affect branch prediction */
    global_flag = 1;
    result1 = caller_function(42, 73);
    
    global_flag = 0;
    result2 = alternate_caller(42, 73);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %ld, %ld\n", result1, result2);
    
    /* Also test with different values */
    global_flag = 1;
    result1 = caller_function(100, 200);
    result2 = alternate_caller(100, 200);
    printf("Results2: %ld, %ld\n", result1, result2);
    
    return 0;
}
