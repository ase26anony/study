/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to make conditional unpredictable */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Use inline assembly to clobber caller-saved registers */
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
        "mov x16, #0xdddd\n\t"
        "mov x17, #0xeeee\n\t"
        "add x0, x0, x1\n\t"
        "add x0, x0, x2\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
          "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
int caller_function(int x, int y) {
    /* Declare many local variables to create register pressure */
    register int a = x + 1;
    register int b = y + 2;
    register int c = x * y;
    register int d = x - y;
    register int e = x ^ y;
    register int f = x | y;
    register int g = x & y;
    register int h = x << 2;
    register int i = y >> 1;
    register int j = x + y;
    register int k = x * 3;
    register int l = y * 7;
    
    /* First computation phase - creates dependencies */
    a = b + c;
    d = e * f;
    g = h ^ i;
    j = k - l;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int flag_copy = global_flag;
    
    /* Conditional call - creates basic block structure */
    if (flag_copy) {
        /* This call will need caller-saved registers preserved */
        callee_function();
    }
    
    /* Second computation phase using same variables */
    /* Create complex dependencies to keep variables live */
    a = a + d + 1;
    b = b + g + 2;
    c = c + j + 3;
    d = d + a + 4;
    e = e + b + 5;
    f = f + c + 6;
    g = g + d + 7;
    h = h + e + 8;
    i = i + f + 9;
    j = j + g + 10;
    k = k + h + 11;
    l = l + i + 12;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables */
    /* This ensures all variables remain live until after the call */
    return a + b + c + d + e + f + g + h + i + j + k + l + x + y;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline))
int secondary_caller(int seed) {
    int result = 0;
    for (int i = 0; i < 3; i++) {
        result += caller_function(seed + i, seed - i);
        
        /* Volatile operation to prevent loop unrolling from
           eliminating the call entirely */
        asm volatile("" : : : "memory");
    }
    return result;
}

int main(void) {
    int total = 0;
    
    /* Initialize global flag with unpredictable value */
    global_flag = (rand() & 1);
    
    /* Call multiple times to give optimizer more chances */
    for (int i = 0; i < 5; i++) {
        total += secondary_caller(i * 10);
        
        /* Modify global flag between iterations */
        global_flag ^= 1;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Also use result in a way that can't be optimized away */
    if (total > 1000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
