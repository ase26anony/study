/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber caller-saved registers on x86_64 */
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
#elif defined(__aarch64__)
    /* Clobber caller-saved registers on ARM64 */
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
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone))
int caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Initialize variables with complex expressions */
    a = seed * 1;
    b = seed * 2 + 1;
    c = seed * 3 + 2;
    d = seed * 4 + 3;
    e = seed * 5 + 4;
    f = seed * 6 + 5;
    g = seed * 7 + 6;
    h = seed * 8 + 7;
    i = seed * 9 + 8;
    j = seed * 10 + 9;
    k = seed * 11 + 10;
    l = seed * 12 + 11;
    m = seed * 13 + 12;
    n = seed * 14 + 13;
    o = seed * 15 + 14;
    p = seed * 16 + 15;
    
    /* First computation phase - creates data dependencies */
    a = b + c * d;
    e = f - g / (h + 1);
    i = j * k + l;
    m = n ^ o | p;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase using same variables */
    b = a * 2 + e;
    c = i / 3 + m;
    d = b ^ c;
    f = e + g * h;
    
    /* More computations to keep variables live */
    g = i + j - k;
    h = l * m - n;
    j = o ^ p + a;
    k = b * c - d;
    
    /* Final computation that uses all variables */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another caller function to create more complex CFG */
__attribute__((noipa, noinline, noclone))
int complex_caller(int x, int y) {
    int r1, r2, r3;
    
    /* Create control flow divergence */
    if (x > 0) {
        r1 = caller_function(x);
        r2 = caller_function(y);
    } else {
        r1 = caller_function(y);
        r2 = caller_function(x);
    }
    
    /* Loop to create more basic blocks */
    for (int z = 0; z < 3; z++) {
        r3 = caller_function(r1 + r2 + z);
        /* Conditional inside loop */
        if (z % 2) {
            asm volatile("" : : : "memory");
            callee_function();
        }
    }
    
    return r1 + r2 + r3;
}

int main(void) {
    int result = 0;
    
    /* Vary the global flag to affect code paths */
    for (int iter = 0; iter < 10; iter++) {
        global_flag = iter % 3;
        
        /* Call with different seeds to create varying register usage */
        int seed1 = iter * 17 + 123;
        int seed2 = iter * 23 + 456;
        
        result += complex_caller(seed1, seed2);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000000) {
        return 0;
    } else {
        return 1;
    }
}
