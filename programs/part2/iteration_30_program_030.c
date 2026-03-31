/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
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
    /* Clobber caller-saved registers for ARM64 */
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
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 2;
    register long b = param2 * 3;
    register long c = a + b;
    register long d = param1 - param2;
    register long e = param1 * param2;
    register long f = param1 + 100;
    register long g = param2 - 50;
    register long h = a * b;
    register long i = c + d;
    register long j = e - f;
    
    /* Additional variables for more pressure */
    register long k = g * 2;
    register long l = h / 3;
    register long m = i << 2;
    register long n = j >> 1;
    
    /* Force values to be live in registers */
    asm volatile("" : : : "memory");
    
    /* Complex computation before the call */
    a = b * c + d;
    b = c - d * e;
    c = d + e / f;
    d = e * f - g;
    e = f + g * h;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* More computations after the call - variables still live */
    f = g + h / i;
    g = h * i - j;
    h = i + j * k;
    i = j - k / l;
    j = k * l + m;
    
    /* Additional post-call computations */
    k = l + m * n;
    l = m - n / a;
    m = n * a + b;
    n = a - b * c;
    
    /* Create data dependencies to prevent dead code elimination */
    a = b + c;
    b = c + d;
    c = d + e;
    d = e + f;
    e = f + g;
    f = g + h;
    g = h + i;
    h = i + j;
    i = j + k;
    j = k + l;
    k = l + m;
    l = m + n;
    
    /* Final computation using all variables */
    long result = a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another caller to create different call pattern */
__attribute__((noipa, noinline, noclone))
long alternate_caller(int x, int y) {
    register long v1 = x * 7;
    register long v2 = y * 11;
    register long v3 = v1 + v2;
    register long v4 = v1 - v2;
    register long v5 = v3 * v4;
    register long v6 = v5 / 2;
    register long v7 = v6 << 3;
    register long v8 = v7 >> 1;
    
    /* Create a basic block boundary before the call */
    if (x > y) {
        v1 = v2 * 3;
        v2 = v3 + 5;
    } else {
        v1 = v2 / 2;
        v2 = v3 - 7;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call with different condition */
    if (global_flag & 1) {
        callee_function();
        
        /* Additional instruction that might be moved */
        v3 = v4 * v5;
    }
    
    /* Post-call computations in same basic block */
    v4 = v5 + v6;
    v5 = v6 * v7;
    v6 = v7 - v8;
    
    /* Force register usage */
    asm volatile("" : "=r"(v7) : "0"(v8));
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    long total = 0;
    
    /* Call multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        global_flag = i & 1;  /* Alternate flag value */
        total += caller_function(i, i * 2);
        total += alternate_caller(i, i + 1);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    /* Additional test with different register usage patterns */
    volatile int seed = 42;
    long sum = 0;
    
    for (int i = 0; i < 50; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        global_flag = seed & 3;
        
        /* Create varying call patterns */
        if (seed & 1) {
            sum += caller_function(seed, seed >> 1);
        } else {
            sum += alternate_caller(seed >> 2, seed >> 3);
        }
    }
    
    printf("Final sum: %ld\n", sum);
    
    return 0;
}
