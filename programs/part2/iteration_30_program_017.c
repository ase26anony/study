/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Use inline assembly to clobber caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
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
    /* ARM64 caller-saved registers */
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
    /* Generic memory clobber */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, noclone))
long caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register long a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Initialize with complex expressions to prevent constant folding */
    a = seed * 1 + 100;
    b = seed * 2 + 200;
    c = seed * 3 + 300;
    d = seed * 4 + 400;
    e = seed * 5 + 500;
    f = seed * 6 + 600;
    g = seed * 7 + 700;
    h = seed * 8 + 800;
    i = seed * 9 + 900;
    j = seed * 10 + 1000;
    k = seed * 11 + 1100;
    l = seed * 12 + 1200;
    m = seed * 13 + 1300;
    n = seed * 14 + 1400;
    o = seed * 15 + 1500;
    p = seed * 16 + 1600;
    
    /* First computation phase - creates data dependencies */
    a = b * c + d;
    e = f * g - h;
    i = j * k / (l + 1);
    m = n * o % (p + 1);
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Second computation phase using same variables */
    /* Create complex data dependencies to keep variables live */
    b = a * e + i;
    c = m * p - j;
    d = k * l / (n + 1);
    f = o * h % (g + 1);
    
    /* More computations to increase register pressure */
    g = (a + b) * (c - d);
    h = (e + f) * (i - j);
    i = (k + l) * (m - n);
    j = (o + p) * (a - b);
    
    /* Final computation that uses all variables */
    long result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to prevent optimization */
__attribute__((noipa, noinline, noclone))
void use_result(long result) {
    /* Use volatile to prevent dead store elimination */
    volatile long sink = result;
    (void)sink;
}

int main(void) {
    long total = 0;
    
    /* Call multiple times with different seeds */
    for (int seed = 0; seed < 100; seed++) {
        /* Toggle global flag to create conditional path */
        global_flag = seed % 2;
        
        /* Call the function with register pressure */
        long result = caller_function(seed);
        
        /* Use the result to prevent elimination */
        use_result(result);
        
        total += result;
    }
    
    printf("Total: %ld\n", total);
    
    /* Also test with different optimization hints */
    {
        /* Force spill/reload behavior */
        int x = 12345;
        asm volatile("" : "+r" (x));
        total += x;
    }
    
    return total > 0 ? 0 : 1;
}
