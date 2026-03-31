/* test-caller-save.c */
#include <stdio.h>
#include <stdint.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers on x86_64 */
#if defined(__x86_64__) || defined(__i386__)
    asm volatile(
        "# Force clobber of caller-saved registers\n\t"
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
    asm volatile(
        "# Force clobber of caller-saved registers\n\t"
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
__attribute__((noipa, noinline, noclone, optimize("no-reorder-blocks")))
int64_t caller_function(int64_t seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed - 3;
    register int64_t d asm("") = seed / 4;
    register int64_t e asm("") = seed % 5;
    register int64_t f asm("") = seed ^ 0x1234;
    register int64_t g asm("") = seed | 0xABCD;
    register int64_t h asm("") = seed & 0xF0F0;
    register int64_t i asm("") = ~seed;
    register int64_t j asm("") = seed << 2;
    register int64_t k asm("") = seed >> 1;
    register int64_t l asm("") = seed * seed;
    
    /* Complex computation before call to ensure variables are live in registers */
    a = b * c + d;
    b = c ^ d ^ e;
    c = (d + e) * (f - g);
    d = (h & i) | (j ^ k);
    e = (l << 3) + (a >> 2);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile global */
    if (global_flag) {
        /* Additional computation right before call */
        f = g * h + i;
        g = j - k + l;
        
        /* The call that will trigger caller-save */
        callee_function();
        
        /* Additional computation right after call */
        h = i ^ j ^ k;
        i = l * a + b;
    } else {
        /* Alternative path without call */
        f = g * h - i;
        g = j + k - l;
        h = i & j & k;
        i = l * a - b;
    }
    
    /* More computations ensuring all variables remain live */
    j = (a + b) * (c - d);
    k = (e & f) | (g ^ h);
    l = (i << 4) + (j >> 3);
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Helper function to create additional register pressure */
__attribute__((noipa, noinline))
int64_t helper(int64_t x, int64_t y) {
    return (x * y) + (x ^ y) - (x & y);
}

int main(void) {
    int64_t result = 0;
    
    /* Create varying input to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        /* Vary the global flag occasionally */
        if (i % 7 == 0) {
            global_flag = !global_flag;
        }
        
        /* Call with different seeds to create different register usage patterns */
        result += caller_function(i);
        
        /* Call helper to create additional register pressure in main */
        result ^= helper(i, result);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", (long)result);
    
    /* Additional test with different optimization hints */
    {
        int64_t test1 = caller_function(0x12345678);
        int64_t test2 = caller_function(0x87654321);
        printf("Test1: %ld, Test2: %ld\n", (long)test1, (long)test2);
    }
    
    return (int)(result & 0x7FFFFFFF);
}
