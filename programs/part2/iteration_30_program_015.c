/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_volatile_flag = 1;

/* Function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber many caller-saved registers */
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
    asm volatile(
        "# Clobber caller-saved registers on ARM64\n\t"
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

/* Complex caller function with many live registers across call */
__attribute__((noipa, noinline, noclone))
long caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register long a asm("") = seed + 1;
    register long b asm("") = seed * 2;
    register long c asm("") = seed / 3;
    register long d asm("") = seed - 4;
    register long e asm("") = seed ^ 0x55;
    register long f asm("") = seed | 0xAA;
    register long g asm("") = seed << 2;
    register long h asm("") = seed >> 1;
    register long i asm("") = ~seed;
    register long j asm("") = seed * seed;
    
    /* Force values to be computed and in registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                     "r"(f), "r"(g), "r"(h), "r"(i), "r"(j) : "memory");
    
    /* Pre-call computation using all variables */
    a = b * c + d;
    b = c ^ d ^ e;
    c = d + e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_volatile_flag) {
        /* This call will clobber caller-saved registers */
        callee_function();
    }
    
    /* Post-call computation - variables must be restored */
    e = f + g + h;
    f = g * h / (i + 1);
    g = h ^ i ^ j;
    h = i * j - a;
    i = j + a + b;
    j = a * b + c * d;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Another caller with different pattern to increase optimization opportunities */
__attribute__((noipa, noinline, noclone))
long caller_function2(int seed) {
    volatile int local_flag = global_volatile_flag;
    register long v1 = seed * 3;
    register long v2 = seed + 7;
    register long v3 = seed - 5;
    register long v4 = seed ^ 0xFF;
    register long v5 = seed | 0x11;
    register long v6 = seed & 0x22;
    register long v7 = seed << 3;
    register long v8 = seed >> 2;
    
    /* Create data dependencies */
    v1 = v2 + v3;
    v2 = v3 * v4;
    
    /* Memory clobber */
    asm volatile("" : : : "memory");
    
    /* Nested conditional to create interesting CFG */
    if (local_flag) {
        if (seed > 0) {
            callee_function();
            v3 = v4 + v5;
        } else {
            v3 = v4 - v5;
        }
    }
    
    /* More computations */
    v4 = v5 * v6;
    v5 = v6 + v7 + v8;
    v6 = v7 ^ v8 ^ v1;
    
    /* Another conditional call */
    if (v6 > 0) {
        asm volatile("" : : : "memory");
        callee_function();
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    long total = 0;
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += caller_function(i);
        total += caller_function2(i);
        
        /* Modify volatile flag to change execution path */
        global_volatile_flag = (i % 3) ? 1 : 0;
        
        /* Memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Use result to prevent optimization */
    printf("Result: %ld\n", total);
    
    /* Additional test with array to create spill pressure */
    {
        register long r1 = total;
        register long r2 = total * 2;
        register long r3 = total / 3;
        register long r4 = total - 100;
        register long r5 = total ^ 0x1234;
        register long r6 = total | 0xABCD;
        
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6));
        
        if (global_volatile_flag) {
            callee_function();
        }
        
        r1 = r2 + r3;
        r2 = r3 * r4;
        
        printf("Additional: %ld\n", r1 + r2);
    }
    
    return 0;
}
