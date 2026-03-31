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
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed / 7;
    register int64_t h asm("") = seed - 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    
    /* Complex computation before call to ensure values are live in registers */
    a = b * c + d;
    b = c * d - e;
    c = d * e / (f + 1);
    d = e * f + g;
    e = f * g - h;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block boundary */
    if (local_flag) {
        /* Additional computation in the conditional path */
        f = g * h + i;
        g = h * i - j;
        
        /* The call that needs caller-save handling */
        callee_function();
        
        /* More computation after call, keeping variables live */
        h = i * j + a;
        i = j * a - b;
    } else {
        /* Alternative path without call */
        f = g * h - i;
        g = h * i + j;
        h = i * j - a;
        i = j * a + b;
    }
    
    /* Complex post-call computation using all variables */
    j = a * b + c * d - e * f + g * h - i * j;
    a = b + c + d + e + f + g + h + i + j;
    b = a * 2 - c * 3 + d * 4 - e * 5;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Final computation ensuring all variables contribute to return value */
    int64_t result = a + b + c + d + e + f + g + h + i + j;
    
    /* Use result in a way that prevents dead code elimination */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Helper function to create additional register pressure */
__attribute__((noipa, noinline, noclone))
int64_t helper_function(int64_t x, int64_t y) {
    return x * y + (x >> 3) - (y << 2);
}

int main(void) {
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different conditions */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the global flag to affect branch prediction */
        global_flag = (iter % 3) != 0;
        
        /* Call with different seeds to create varying register usage patterns */
        int64_t seed = iter * 17 + 12345;
        
        /* Call helper to create additional register pressure context */
        int64_t temp = helper_function(seed, iter);
        
        /* Main call that should trigger caller-save optimization */
        int64_t result = caller_function(seed + temp);
        
        /* Use result to prevent optimization */
        total += result;
        
        /* Occasionally modify memory to affect optimization decisions */
        if (iter % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %ld\n", (long)total);
    
    return 0;
}
