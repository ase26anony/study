/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__)
    asm volatile(
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
    /* Clobber caller-saved registers for ARM64 */
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
        "add x0, x0, x1\n\t"
        "add x0, x0, x2\n\t"
        : /* no outputs */
        : /* no inputs */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
int caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int a asm("") = seed + 1;
    register int b asm("") = seed * 2;
    register int c asm("") = seed / 3;
    register int d asm("") = seed - 4;
    register int e asm("") = seed + 5;
    register int f asm("") = seed * 6;
    register int g asm("") = seed / 7;
    register int h asm("") = seed - 8;
    register int i asm("") = seed + 9;
    register int j asm("") = seed * 10;
    
    /* Create data dependencies between variables */
    a = b * c + d;
    b = c + d * e;
    c = d - e * f;
    d = e + f / g;
    
    /* Memory barrier to force values to be live in registers */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int flag = global_flag;
    
    /* Conditional call - creates basic block boundary */
    if (flag) {
        /* Additional computation before call to create more live values */
        e = f * g + h;
        f = g + h * i;
        
        /* Call that clobbers caller-saved registers */
        callee_function();
        
        /* More computations after call - variables still live */
        g = h * i + j;
        h = i + j * a;
    } else {
        /* Alternative path with different computations */
        e = f - g + h;
        f = g - h + i;
        g = h - i + j;
        h = i - j + a;
    }
    
    /* Complex computation using all variables */
    i = j * a + b;
    j = a * b + c;
    
    /* Create cross-dependencies to prevent reordering */
    a = b + c * d;
    b = c + d * e;
    c = d + e * f;
    d = e + f * g;
    
    /* Final result using all variables */
    int result = a + b + c + d + e + f + g + h + i + j;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional call site with different pattern */
__attribute__((noipa, noinline))
int helper_function(int x, int y) {
    register int r1 = x + y;
    register int r2 = x * y;
    register int r3 = x - y;
    register int r4 = x ^ y;
    register int r5 = x | y;
    register int r6 = x & y;
    
    /* Create data dependencies */
    r1 = r2 + r3 * r4;
    r2 = r3 + r4 / (r5 ? r5 : 1);
    
    /* Force conditional call */
    if (global_flag > 0) {
        callee_function();
    }
    
    /* Continue using registers */
    r3 = r4 * r5 + r6;
    r4 = r5 + r6 * r1;
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

int main(void) {
    int total = 0;
    
    /* Call multiple times with different seeds */
    for (int k = 0; k < 100; k++) {
        /* Toggle global flag to affect conditional paths */
        global_flag = (k % 3) > 0;
        
        /* Call main function */
        total += caller_function(k);
        
        /* Also call helper to create different call pattern */
        if (k % 5 == 0) {
            total += helper_function(k, k * 2);
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        return 0;
    } else {
        return 1;
    }
}
