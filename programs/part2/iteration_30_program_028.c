/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers to force save/restore */
#if defined(__x86_64__)
    asm volatile(
        "movq $0x12345678, %%rax\n\t"
        "movq $0x87654321, %%rcx\n\t"
        "movq $0x11111111, %%rdx\n\t"
        "movq $0x22222222, %%rsi\n\t"
        "movq $0x33333333, %%rdi\n\t"
        "movq $0x44444444, %%r8\n\t"
        "movq $0x55555555, %%r9\n\t"
        "movq $0x66666666, %%r10\n\t"
        "movq $0x77777777, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    asm volatile(
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
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-inline")))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a asm("") = param1 * 2;
    register long b asm("") = param2 * 3;
    register long c asm("") = a + b;
    register long d asm("") = c * 7;
    register long e asm("") = d - a;
    register long f asm("") = e + b;
    register long g asm("") = f * 2;
    register long h asm("") = g / 3;
    register long i asm("") = h << 2;
    register long j asm("") = i >> 1;
    register long k asm("") = j | 0xFF;
    register long l asm("") = k & 0xAA;
    
    /* Create data dependencies to prevent reordering */
    volatile long dep1 = a + b;
    volatile long dep2 = c + d;
    
    /* Memory barrier to ensure values are in registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call - creates basic block boundaries */
    if (global_flag) {
        /* Additional computation before call */
        a = b * c + 1;
        b = c * d - 2;
        c = d * e + 3;
        
        /* Memory barrier before call */
        asm volatile("" : : : "memory");
        
        /* The call that will trigger caller-save logic */
        callee_function();
        
        /* Memory barrier after call */
        asm volatile("" : : : "memory");
        
        /* Additional computation after call */
        d = e * f + 4;
        e = f * g - 5;
        f = g * h + 6;
    } else {
        /* Alternative path without call */
        a = b * c - 1;
        b = c * d + 2;
    }
    
    /* More computations ensuring all variables are live */
    g = h * i + 7;
    h = i * j - 8;
    i = j * k + 9;
    j = k * l - 10;
    k = l * a + 11;
    l = a * b - 12;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l + dep1 + dep2;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline))
long secondary_caller(int x, int y) {
    register long v1 = x * 11;
    register long v2 = y * 13;
    register long v3 = v1 + v2;
    register long v4 = v2 - v1;
    register long v5 = v3 * v4;
    
    if (global_flag > 0) {
        v1 = v2 * v3;
        v2 = v3 * v4;
        asm volatile("" : : : "memory");
        callee_function();
        asm volatile("" : : : "memory");
        v3 = v4 * v5;
        v4 = v5 * v1;
    }
    
    return v1 + v2 + v3 + v4 + v5;
}

int main(void) {
    volatile int flag = 1;
    long result1, result2;
    
    /* Vary the flag to affect branch prediction */
    global_flag = flag;
    
    /* Call the function multiple times with different parameters */
    result1 = caller_function(10, 20);
    result2 = secondary_caller(30, 40);
    
    /* Change flag and call again */
    global_flag = 0;
    result1 += caller_function(50, 60);
    result2 += secondary_caller(70, 80);
    
    /* Use results to prevent optimization */
    printf("Results: %ld %ld\n", result1, result2);
    
    return 0;
}
