/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
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
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed / 7;
    register int64_t h asm("") = seed - 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    register int64_t k asm("") = seed / 11;
    register int64_t l asm("") = seed - 12;
    
    /* Complex computation before call to keep variables live */
    a = b * c + d - e;
    b = c * d / (e + 1);
    c = d ^ e ^ f;
    d = (g << 2) | (h >> 3);
    e = f * g - h;
    f = g + h * i;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call - creates basic block boundaries */
    if (global_flag) {
        /* Additional computation in the conditional path */
        g = h * i + j;
        h = i ^ j ^ k;
        
        /* The call that needs caller-save protection */
        callee_function();
        
        /* More computation after call, keeping variables live */
        i = j * k - l;
        j = k + l * a;
    } else {
        /* Alternative path to create control flow complexity */
        g = h - i + j;
        h = i & j & k;
        i = j | k | l;
        j = k ^ l ^ a;
    }
    
    /* Complex post-call computation mixing all variables */
    k = l * a + b - c;
    l = a * b / (c + 1);
    a = b ^ c ^ d;
    b = (e << 3) | (f >> 2);
    c = d * e - f;
    d = e + f * g;
    e = f ^ g ^ h;
    f = (i << 1) | (j >> 4);
    g = h * i + j;
    h = i ^ j ^ k;
    i = j * k - l;
    j = k + l * a;
    
    /* Final computation that uses all variables */
    int64_t result = a + b - c + d - e + f - g + h - i + j - k + l;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional call site with different pattern */
__attribute__((noipa, noinline, noclone))
int64_t another_caller(int64_t x, int64_t y) {
    register int64_t v1 asm("") = x + y;
    register int64_t v2 asm("") = x * y;
    register int64_t v3 asm("") = x - y;
    register int64_t v4 asm("") = x ^ y;
    register int64_t v5 asm("") = x & y;
    register int64_t v6 asm("") = x | y;
    register int64_t v7 asm("") = x << (y & 3);
    register int64_t v8 asm("") = x >> (y & 3);
    
    /* Create data dependencies */
    v1 = v2 + v3;
    v2 = v3 * v4;
    v3 = v4 ^ v5;
    
    asm volatile("" : : : "memory");
    
    if (global_flag > 0) {
        v4 = v5 & v6;
        v5 = v6 | v7;
        
        callee_function();
        
        v6 = v7 << 2;
        v7 = v8 >> 1;
    }
    
    v8 = v1 + v2 - v3 + v4 - v5 + v6 - v7;
    
    return v8;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the global flag to affect code paths */
    global_flag = 1;
    result1 = caller_function(42);
    
    global_flag = 0;
    result2 = another_caller(100, 200);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %lld, %lld\n", 
           (long long)result1, 
           (long long)result2);
    
    /* Mix calls with different flags */
    global_flag = 1;
    result1 = caller_function(123);
    result2 = another_caller(456, 789);
    
    printf("More results: %lld, %lld\n", 
           (long long)result1, 
           (long long)result2);
    
    return 0;
}
