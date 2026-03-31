/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Use inline assembly to clobber caller-saved registers */
    /* This forces the caller to save/restore these registers */
#if defined(__x86_64__)
    asm volatile(
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x55555555, %%rdx\n\t"
        "mov $0xAAAAAAAA, %%rsi\n\t"
        "mov $0x11111111, %%rdi\n\t"
        "mov $0x22222222, %%r8\n\t"
        "mov $0x33333333, %%r9\n\t"
        "mov $0x44444444, %%r10\n\t"
        "mov $0x66666666, %%r11\n\t"
        "add %%rcx, %%rax\n\t"
        "sub %%rdx, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    asm volatile(
        "mov x0, #0x1234\n\t"
        "mov x1, #0x5678\n\t"
        "mov x2, #0x9ABC\n\t"
        "mov x3, #0xDEF0\n\t"
        "mov x4, #0x1111\n\t"
        "mov x5, #0x2222\n\t"
        "mov x6, #0x3333\n\t"
        "mov x7, #0x4444\n\t"
        "mov x8, #0x5555\n\t"
        "mov x9, #0x6666\n\t"
        "mov x10, #0x7777\n\t"
        "mov x11, #0x8888\n\t"
        "mov x12, #0x9999\n\t"
        "mov x13, #0xAAAA\n\t"
        "mov x14, #0xBBBB\n\t"
        "mov x15, #0xCCCC\n\t"
        "mov x16, #0xDDDD\n\t"
        "mov x17, #0xEEEE\n\t"
        "add x0, x0, x1\n\t"
        "sub x2, x2, x3\n\t"
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

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2, int param3) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 2;
    register long b = param2 + 100;
    register long c = param3 - 50;
    register long d = a * b;
    register long e = b + c;
    register long f = c * d;
    register long g = d + e;
    register long h = e * f;
    register long i = f + g;
    register long j = g * h;
    register long k = h + i;
    register long l = i * j;
    
    /* First computation phase - creates data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates a basic block boundary */
    if (global_flag) {
        /* The call instruction will be 'insn' in the uncovered code */
        callee_function();
    }
    
    /* Second computation phase using same variables */
    /* Ensures variables are live across the call */
    d = e * f - g;
    e = f * g + h;
    f = g * h - i;
    g = h * i + j;
    h = i * j - k;
    i = j * k + l;
    
    /* More complex computations to prevent dead code elimination */
    j = (a + b) * (c - d);
    k = (e + f) * (g - h);
    l = (i + j) * (k - a);
    
    /* Final result using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Another caller to create more complex CFG */
__attribute__((noipa, noinline))
long caller_function2(int x) {
    register long v1 = x * 3;
    register long v2 = x + 7;
    register long v3 = x - 5;
    register long v4 = v1 * v2;
    register long v5 = v2 + v3;
    
    /* Create a loop to generate more complex control flow */
    for (int i = 0; i < 3; i++) {
        v1 = v2 * v3 + v4;
        v2 = v3 * v4 - v5;
        
        /* Conditional call inside loop */
        if (global_flag & (1 << i)) {
            callee_function();
        }
        
        v3 = v4 * v5 + v1;
        v4 = v5 * v1 - v2;
        v5 = v1 * v2 + v3;
    }
    
    return v1 + v2 + v3 + v4 + v5;
}

int main(void) {
    long result1, result2;
    
    /* Initialize with non-trivial values */
    int base = 42;
    
    /* Call first function */
    result1 = caller_function(base, base + 1, base - 1);
    
    /* Modify global flag */
    global_flag = 2;
    
    /* Call second function */
    result2 = caller_function2(base);
    
    /* Use results to prevent optimization */
    printf("Results: %ld, %ld\n", result1, result2);
    
    /* Force compiler to keep both functions */
    if (result1 > 1000000) {
        caller_function(1, 2, 3);
    }
    
    return 0;
}
