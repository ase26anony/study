/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
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
#elif defined(__aarch64__)
    /* Clobber caller-saved registers for ARM64 */
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

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone))
long caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register long a = seed * 2;
    register long b = seed + 100;
    register long c = seed - 50;
    register long d = seed * 3 + 7;
    register long e = seed / 2;
    register long f = seed * seed;
    register long g = seed + 777;
    register long h = seed - 123;
    register long i = seed * 5 - 99;
    register long j = seed + 456;
    
    /* Complex computation before call to make values live in registers */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    e = f * g + h;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to force conditional evaluation */
    volatile int flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (flag) {
        /* Additional computation in the conditional path */
        f = g * h + i;
        g = h * i - j;
        
        /* The critical call that needs save/restore */
        callee_function();
        
        /* More computation after call, keeping variables live */
        h = i * j + a;
        i = j * a - b;
    } else {
        /* Alternative path without call */
        f = seed * 2;
        g = seed * 3;
    }
    
    /* Complex post-call computation using all variables */
    j = a + b + c + d + e;
    a = b + c + d + e + f;
    b = c + d + e + f + g;
    c = d + e + f + g + h;
    d = e + f + g + h + i;
    
    /* Final computation that uses all variables */
    long result = a * b + c * d - e * f + g * h - i * j;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result + seed;
}

/* Helper function to create additional register pressure */
__attribute__((noipa, noinline, noclone))
int helper(int x, int y) {
    return x * y + (x ^ y) - (x & y);
}

int main(void) {
    long total = 0;
    
    /* Loop to create multiple call sites with different contexts */
    for (int i = 0; i < 100; i++) {
        /* Vary the global flag to affect branch prediction */
        global_flag = i % 3;
        
        /* Call with different seeds to create varying register usage */
        long result = caller_function(i);
        
        /* Use helper to create additional register pressure in main */
        int temp = helper(i, result & 0xFF);
        
        /* Accumulate result to prevent elimination */
        total += result + temp;
        
        /* Occasionally call callee directly from main too */
        if (i % 7 == 0) {
            callee_function();
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    return 0;
}
