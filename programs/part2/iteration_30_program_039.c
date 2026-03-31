/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers on x86_64 */
#if defined(__x86_64__) || defined(__i386__)
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
    /* Clobber caller-saved registers on ARM64 */
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
        "mov x16, #0\n\t"
        "mov x17, #0\n\t"
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
__attribute__((noipa, noinline, noclone))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 3;
    register long b = param2 * 7;
    register long c = a + b * 2;
    register long d = c - param1 * 5;
    register long e = d * 3 + 11;
    register long f = e / 2 + param2;
    register long g = f * 7 - 13;
    register long h = g + a * 2;
    register long i = h - b * 3;
    register long j = i * 5 + 17;
    
    /* Force values to be in registers before the call */
    asm volatile("" : : : "memory");
    
    /* Complex computation before call to create live ranges */
    a = b * c + d - e;
    b = c * d - e + f;
    c = d * e + f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will need caller-saved registers preserved */
        callee_function();
    }
    
    /* More computations after call, using same variables */
    d = e * f - g + h;
    e = f * g + h - i;
    f = g * h + i - j;
    
    /* Additional computations to extend live ranges */
    g = h * i + j - a;
    h = i * j + a - b;
    i = j * a + b - c;
    j = a * b + c - d;
    
    /* Complex return value using all variables */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + 
           f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Another caller to create different calling context */
__attribute__((noipa, noinline, noclone))
long alternate_caller(int x, int y) {
    register long v1 = x * 2;
    register long v2 = y * 3;
    register long v3 = v1 + v2 * 4;
    register long v4 = v3 - x * 5;
    register long v5 = v4 * 6 + 7;
    register long v6 = v5 / 8 + y;
    register long v7 = v6 * 9 - 10;
    register long v8 = v7 + v1 * 11;
    
    /* Different computation pattern */
    v1 = v2 * v3 - v4;
    v2 = v3 + v4 * v5;
    
    /* Nested conditional to create interesting CFG */
    if (global_flag > 0) {
        if (x > y) {
            callee_function();
        } else {
            v3 = v4 * v5 - v6;
            asm volatile("" : : : "memory");
        }
    }
    
    v4 = v5 * v6 + v7;
    v5 = v6 - v7 * v8;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    long result1, result2;
    
    /* Vary the flag to affect code paths */
    global_flag = 1;
    result1 = caller_function(42, 17);
    
    global_flag = 0;
    result2 = alternate_caller(23, 89);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %ld, %ld\n", result1, result2);
    
    /* Also test with different values */
    global_flag = 1;
    result1 = caller_function(100, 200);
    result2 = alternate_caller(300, 400);
    printf("More results: %ld, %ld\n", result1, result2);
    
    return 0;
}
