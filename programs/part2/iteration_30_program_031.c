/* test_caller_save.c - Program to trigger specific RTL list surgery in GCC's caller-save pass */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to create conditional call */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Use inline assembly to clobber caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
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

/* Caller function with high register pressure across call */
__attribute__((noipa, noinline))
int caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register int a = param1 * 2;
    register int b = param2 + 3;
    register int c = a - b;
    register int d = param1 * param2;
    register int e = a + b + c;
    register int f = d - e;
    register int g = param1 ^ param2;
    register int h = a * b - c;
    register int i = d | e;
    register int j = f & g;
    
    /* First computation phase - all variables live */
    a = b * c + d;
    b = c - d * e;
    c = d + e / (f + 1);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_volatile_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase - variables still live after call */
    d = e + f * g;
    e = f - g / (h + 1);
    f = g * h + i;
    g = h - i * j;
    h = i + j / (a + 1);
    i = j * a - b;
    j = a + b * c;
    
    /* Additional memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables to keep them live */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Another caller with different pattern to increase chances */
__attribute__((noipa, noinline))
int caller_function2(int param) {
    register int v1 = param;
    register int v2 = param * 2;
    register int v3 = param + 3;
    register int v4 = param - 4;
    register int v5 = param * param;
    register int v6 = v1 + v2;
    register int v7 = v3 - v4;
    register int v8 = v5 ^ v6;
    
    /* Create data dependencies */
    v1 = v2 + v3;
    v2 = v4 * v5;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Nested conditional to create interesting CFG */
    if (global_volatile_flag > 0) {
        if (param % 2) {
            callee_function();
        } else {
            v3 = v6 * v7;
            asm volatile("" : : : "memory");
        }
    }
    
    /* More computations */
    v4 = v7 + v8;
    v5 = v1 - v2;
    v6 = v3 * v4;
    v7 = v5 / (v6 + 1);
    v8 = v1 ^ v2 ^ v3;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    int result1, result2;
    
    /* Vary the flag to prevent optimization */
    global_volatile_flag = rand() % 3;
    
    /* Call first function */
    result1 = caller_function(rand() % 100, rand() % 100);
    
    /* Change flag again */
    global_volatile_flag = rand() % 3;
    
    /* Call second function */
    result2 = caller_function2(rand() % 100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional test with loop to create more optimization opportunities */
    for (int i = 0; i < 10; i++) {
        global_volatile_flag = i % 3;
        int temp = caller_function(i, i * 2);
        result1 += temp;
    }
    
    printf("Final result: %d\n", result1);
    
    return 0;
}
