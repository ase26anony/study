/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
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
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
/* Clobber caller-saved registers for ARM64 */
#elif defined(__aarch64__)
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
        "mov x16, #0xdddd\n\t"
        "mov x17, #0xeeee\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
/* Generic fallback */
#else
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone, optimize("no-inline")))
int64_t caller_function(int64_t seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 100;
    register int64_t e asm("") = seed ^ 0x5555;
    register int64_t f asm("") = seed | 0xAAAA;
    register int64_t g asm("") = seed & 0xFFFF;
    register int64_t h asm("") = seed << 2;
    register int64_t i asm("") = seed >> 1;
    register int64_t j asm("") = ~seed;
    
    /* Force values to be computed and live in registers */
    volatile int64_t temp;
    
    /* Complex computation before call to ensure values are live */
    a = b * c + d;
    b = c ^ d ^ e;
    c = (d + e) * (f - g);
    d = e | f | g;
    e = (h << 3) ^ (i >> 2);
    f = g * h + i - j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Store intermediate values to volatile to ensure they're computed */
    temp = a + b + c;
    
    /* Conditional call based on volatile global */
    if (global_flag) {
        /* This call will need to save many caller-saved registers */
        callee_function();
    }
    
    /* More computations after call, using the same variables */
    /* Create data dependencies that prevent reordering */
    g = a * b + c - d;
    h = (e ^ f) | (g & h);
    i = j + a - b * c;
    j = (d << 4) ^ (e >> 2) & f;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex return value using all variables */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + 
           f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Alternative caller with different control flow to encourage
   save/restore movement across basic blocks */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int64_t seed) {
    register int64_t v1 = seed * 3;
    register int64_t v2 = seed + 7;
    register int64_t v3 = seed ^ 0xFF;
    register int64_t v4 = seed | 0xAA;
    register int64_t v5 = seed & 0x55;
    register int64_t v6 = seed << 1;
    register int64_t v7 = seed >> 2;
    register int64_t v8 = ~seed;
    register int64_t v9 = seed * seed;
    register int64_t v10 = seed + seed;
    
    /* Create a loop to generate more complex CFG */
    for (int k = 0; k < 3; k++) {
        v1 = v2 + v3;
        v2 = v3 * v4;
        v3 = v4 ^ v5;
        
        /* Conditional call inside loop */
        if (global_flag & (1 << k)) {
            /* This creates a scenario where save/restore might be
               moved to less frequent paths */
            callee_function();
        }
        
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 + v8;
    }
    
    /* Force all variables to be used in return */
    return v1 + v2 - v3 * v4 + v5 ^ v6 | v7 & v8 + v9 - v10;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the global flag to affect control flow */
    global_flag = 1;
    result1 = caller_function(42);
    
    global_flag = 7; /* Different value for second call */
    result2 = caller_function2(123);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %lld, %lld\n", (long long)result1, (long long)result2);
    
    /* Also test with flag = 0 to exercise different path */
    global_flag = 0;
    result1 = caller_function(99);
    printf("Result with flag=0: %lld\n", (long long)result1);
    
    return 0;
}
