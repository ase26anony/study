/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__) || defined(__i386__)
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
    /* Clobber caller-saved registers for ARM64 */
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
int64_t caller_function(int64_t seed) {
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
    
    /* Perform computations before the call */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag) {
        /* Additional computation in the conditional path */
        e = f * g + h;
        f = g * h - i;
        
        /* The critical call that triggers caller-save */
        callee_function();
        
        /* More computations after the call */
        g = h * i + j;
        h = i * j - k;
    } else {
        /* Alternative path without the call */
        e = f * g - h;
        f = g * h + i;
        g = h * i - j;
        h = i * j + k;
    }
    
    /* Additional computations that use all variables */
    i = j * k + l;
    j = k * l - a;
    k = l * a + b;
    l = a * b - c;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int64_t seed) {
    register int64_t m = seed * 13;
    register int64_t n = seed + 14;
    register int64_t o = seed * 15;
    register int64_t p = seed + 16;
    register int64_t q = seed * 17;
    register int64_t r = seed + 18;
    
    m = n * o + p;
    n = o * p - q;
    
    asm volatile("" : : : "memory");
    
    if (global_flag > 0) {
        callee_function();
        o = p * q + r;
        p = q * r - m;
    }
    
    q = r * m + n;
    r = m * n - o;
    
    return m + n + o + p + q + r;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to affect branch prediction */
    for (int iter = 0; iter < 100; iter++) {
        global_flag = (iter % 3) != 0;  /* Mix of true/false */
        
        /* Call both functions to increase optimization scope */
        result1 = caller_function(iter);
        result2 = caller_function2(iter + 100);
        
        /* Use results to prevent dead code elimination */
        asm volatile("" : "+r"(result1), "+r"(result2));
    }
    
    /* Final output to prevent optimization */
    printf("Results: %lld %lld\n", (long long)result1, (long long)result2);
    
    return 0;
}
