/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__)
    asm volatile(
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
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
int caller_function(int x, int y) {
    /* Declare many local variables to create register pressure */
    register int a = x + 1;
    register int b = y * 2;
    register int c = x * y + 3;
    register int d = x - y + 4;
    register int e = x * x + y;
    register int f = y * y + x;
    register int g = (x << 2) | y;
    register int h = (y << 3) & x;
    register int i = x ^ y + 7;
    register int j = ~x + y * 8;
    
    /* Force values to be in registers before computation */
    asm volatile("" : : : "memory");
    
    /* First computation phase - all variables used */
    a = b + c * 2;
    b = d - e / 3;
    c = f | g & 0xFF;
    d = h ^ i;
    e = j + a * b;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase - reuse all variables */
    f = a + b + c;
    g = d * e - f;
    h = (g >> 2) + i;
    i = j * h / 3;
    j = (a ^ b) | (c & d);
    
    /* Complex final computation using all variables */
    int result = a + b * 2 + c * 3 + d * 4 + e * 5 +
                 f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
int helper(int x) {
    return x * 3 + 7;
}

int main(void) {
    int result = 0;
    
    /* Loop to potentially create different optimization paths */
    for (int i = 0; i < 100; i++) {
        /* Vary the flag to create conditional paths */
        global_flag = i & 1;
        
        /* Call with different arguments to prevent constant propagation */
        result += caller_function(i, i + 1);
        
        /* Call helper to create additional register pressure in caller */
        result += helper(i);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with different patterns */
    global_flag = 1;
    int r1 = caller_function(100, 200);
    global_flag = 0;
    int r2 = caller_function(200, 100);
    
    printf("Results: %d, %d\n", r1, r2);
    
    return 0;
}
