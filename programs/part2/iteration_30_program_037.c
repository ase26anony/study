/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create conditional call path */
volatile int global_volatile_flag = 1;

/* Prevent interprocedural analysis and inlining */
#ifdef __GNUC__
#define NOIPA __attribute__((noipa, noinline, noclone))
#else
#define NOIPA
#endif

/* Callee function that clobbers caller-saved registers */
NOIPA void callee_function(void) {
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__) || defined(__i386__)
    asm volatile(
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
/* Clobber caller-saved registers for ARM64 */
#elif defined(__aarch64__)
    asm volatile(
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

/* Caller function with high register pressure across a call */
NOIPA int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a = seed + 1;
    register int64_t b = seed * 2;
    register int64_t c = seed + 3;
    register int64_t d = seed * 4;
    register int64_t e = seed + 5;
    register int64_t f = seed * 6;
    register int64_t g = seed + 7;
    register int64_t h = seed * 8;
    register int64_t i = seed + 9;
    register int64_t j = seed * 10;
    register int64_t k = seed + 11;
    register int64_t l = seed * 12;
    
    /* Complex computation before call to ensure values are in registers */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    e = f * g + h;
    f = g * h - i;
    
    /* Memory barrier to ensure values are live in registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag - creates basic block boundary */
    if (global_volatile_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* More complex computations after call - variables still live */
    g = h * i + j;
    h = i * j - k;
    i = j * k + l;
    j = k * l - a;
    k = l * a + b;
    l = a * b - c;
    
    /* Additional memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables to prevent dead code elimination */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Another caller function with different pattern to increase optimization opportunities */
NOIPA int64_t caller_function2(int seed) {
    register int64_t m = seed * 13;
    register int64_t n = seed + 14;
    register int64_t o = seed * 15;
    register int64_t p = seed + 16;
    register int64_t q = seed * 17;
    register int64_t r = seed + 18;
    register int64_t s = seed * 19;
    register int64_t t = seed + 20;
    
    /* Different computation pattern */
    m = n * o - p;
    n = o * p + q;
    
    /* Volatile memory access to create additional basic blocks */
    volatile int temp = global_volatile_flag;
    
    if (temp > 0) {
        o = p * q - r;
        p = q * r + s;
        
        /* Nested conditional with call */
        if (global_volatile_flag & 1) {
            callee_function();
        }
        
        q = r * s - t;
    } else {
        o = p + q * r;
        p = q - r * s;
    }
    
    r = s * t + m;
    s = t * m - n;
    t = m * n + o;
    
    return m + n + o + p + q + r + s + t;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to create different execution paths */
    global_volatile_flag = 1;
    result1 = caller_function(42);
    
    global_volatile_flag = 0;
    result2 = caller_function2(24);
    
    /* Use results to prevent optimization */
    printf("Results: %lld, %lld\n", (long long)result1, (long long)result2);
    
    /* Additional test with loop to create more optimization context */
    for (int i = 0; i < 10; i++) {
        global_volatile_flag = i & 1;
        int64_t r = caller_function(i * 10);
        printf("Iteration %d: %lld\n", i, (long long)r);
    }
    
    return 0;
}
