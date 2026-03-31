/* caller-save-test.c
 * Designed to trigger specific instruction chain manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-optimize-sibling-calls test.c -o test
 * For RTL dumps: add -fdump-rtl-caller-save -fdump-rtl-all
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile to force conditional call */
volatile int global_flag = 1;

/* Callee that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Force clobbering of caller-saved registers */
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
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller with high register pressure across conditional call */
__attribute__((noipa, noinline, noclone))
int64_t caller_function(int seed) {
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
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Read volatile to force spill/reload consideration */
    temp = global_flag;
    
    /* Conditional call - creates basic block boundary */
    if (temp & 1) {
        /* Additional computation in the conditional path */
        d = e * f + g;
        e = f ^ g ^ h;
        
        /* The call that clobbers caller-saved registers */
        callee_function();
        
        /* More computation after call, using pre-call values */
        f = g + h + i;
        g = h * i / j;
    } else {
        /* Alternative path without call */
        d = e + f + g;
        e = f * g - h;
    }
    
    /* Complex post-call computation ensuring all variables are live */
    h = i * j + a;
    i = j ^ a ^ b;
    j = (a + b) * (c - d);
    
    /* Final computation using all variables */
    int64_t result = a + b + c + d + e + f + g + h + i + j;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another caller to create more context for the optimization */
__attribute__((noinline))
int64_t caller_wrapper(int seed) {
    int64_t x = caller_function(seed);
    int64_t y = caller_function(seed + 100);
    
    /* Force computation to prevent optimization */
    volatile int64_t dummy;
    dummy = x;
    
    return x ^ y;
}

int main(void) {
    int64_t total = 0;
    
    /* Loop to create multiple call sites */
    for (int i = 0; i < 100; i++) {
        /* Vary the global flag to affect conditional */
        global_flag = i & 3;
        
        /* Call with different seeds to prevent constant propagation */
        int64_t result = caller_wrapper(i * 7 + 12345);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Occasionally modify global flag */
        if (i % 23 == 0) {
            global_flag = ~global_flag;
        }
    }
    
    printf("Result: %ld\n", (long)total);
    return 0;
}
