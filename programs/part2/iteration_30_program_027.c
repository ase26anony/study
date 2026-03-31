/* test_caller_save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Function to clobber caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__) || defined(__i386__)
    asm volatile (
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
    asm volatile ("" : : : "memory");
#endif
}

/* Main caller function with high register pressure */
__attribute__((noipa, noinline))
int64_t caller_function(int64_t seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed - 3;
    register int64_t d asm("") = seed / 4;
    register int64_t e asm("") = seed % 5;
    register int64_t f asm("") = seed ^ 0x1234;
    register int64_t g asm("") = seed | 0x5678;
    register int64_t h asm("") = seed & 0x9ABC;
    register int64_t i asm("") = ~seed;
    register int64_t j asm("") = seed << 2;
    register int64_t k asm("") = seed >> 1;
    register int64_t l asm("") = seed * seed;
    
    /* Perform computations to make variables live */
    a = b * c + d;
    b = c ^ d ^ e;
    c = d + e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag) {
        /* Additional computation before call in same block */
        e = f + g + h;
        f = g * h / 2;
        
        /* The critical call */
        callee_function();
        
        /* More computations after call, keeping variables live */
        g = h ^ i ^ j;
        h = i + j + k;
    } else {
        /* Alternative path without call */
        e = f - g - h;
        f = g / h * 2;
        g = h | i | j;
        h = i - j - k;
    }
    
    /* Additional computations mixing all variables */
    i = j * k + l;
    j = k ^ l ^ a;
    k = l + a + b;
    l = a * b - c;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Another caller to create different patterns */
__attribute__((noipa, noinline))
int64_t caller_function2(int64_t seed) {
    register int64_t m asm("") = seed * 3;
    register int64_t n asm("") = seed + 7;
    register int64_t o asm("") = seed - 11;
    register int64_t p asm("") = seed / 13;
    
    /* Nested conditional with call */
    if (global_flag > 0) {
        m = n * o;
        if (global_flag < 10) {
            n = o + p;
            callee_function();
            o = p * m;
        } else {
            n = o - p;
        }
        p = m / n;
    }
    
    return m + n + o + p;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the flag to create different execution paths */
    global_flag = 1;
    result1 = caller_function(42);
    
    global_flag = 5;
    result2 = caller_function2(100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %lld, %lld\n", (long long)result1, (long long)result2);
    
    /* Try with different flag values */
    global_flag = 0;
    result1 = caller_function(99);
    
    global_flag = 15;
    result2 = caller_function2(200);
    
    printf("Results2: %lld, %lld\n", (long long)result1, (long long)result2);
    
    return 0;
}
