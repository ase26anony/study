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
    
    /* Force values to be computed and live in registers */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates opportunity for save/restore reordering */
    if (local_flag) {
        /* Additional computation before call to create more live values */
        e = f * g + h;
        f = g * h - i;
        
        /* Call that clobbers registers */
        callee_function();
        
        /* More computation after call, using pre-call values */
        g = h * i + j;
        h = i * j - k;
    } else {
        /* Alternative path with different computation */
        e = j * k + l;
        f = k * l - a;
        g = l * a + b;
        h = a * b - c;
    }
    
    /* Complex computation using all variables to keep them live */
    i = j * k + l;
    j = k * l - a;
    k = l * a + b;
    l = a * b - c;
    
    /* Final computation that uses all variables */
    int64_t result = a + b + c + d + e + f + g + h + i + j + k + l;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noinline))
int64_t helper(int64_t x, int64_t y) {
    return x * y + (x ^ y);
}

int main(void) {
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different contexts */
    for (int64_t i = 0; i < 100; i++) {
        /* Vary the global flag to affect branch prediction */
        global_flag = (i % 3) != 0;
        
        /* Call with different seeds to create varying register usage patterns */
        int64_t result = caller_function(i);
        
        /* Mix in another function call to create additional save/restore points */
        result += helper(i, result);
        
        total += result;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    printf("Result: %ld\n", (long)total);
    return (int)total;
}
