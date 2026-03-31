/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, target("no-sse")))
void callee_function(void) {
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__)
    asm volatile (
        "movq $0, %%rax\n\t"
        "movq $0, %%rcx\n\t"
        "movq $0, %%rdx\n\t"
        "movq $0, %%rsi\n\t"
        "movq $0, %%rdi\n\t"
        "movq $0, %%r8\n\t"
        "movq $0, %%r9\n\t"
        "movq $0, %%r10\n\t"
        "movq $0, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    /* Clobber caller-saved registers for ARM64 */
    asm volatile (
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
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-inline")))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a = param1 * 3;
    register long b = param2 * 7;
    register long c = a + b;
    register long d = param1 - param2;
    register long e = param1 * param2;
    register long f = param1 + param2 * 2;
    register long g = param2 - param1;
    register long h = param1 * param1;
    register long i = param2 * param2;
    register long j = a + b + c;
    
    /* Force values to be in registers before the call */
    asm volatile("" : : : "memory");
    
    /* Complex computation creating data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call should trigger caller-save register spills */
        callee_function();
    }
    
    /* More computations after the call, keeping variables live */
    e = f * g + h;
    f = g * h - i;
    g = h * i + j;
    h = i * j - a;
    
    /* Create complex data flow to prevent dead store elimination */
    long result = (a + b) * (c - d) + (e * f) - (g / (h != 0 ? h : 1));
    
    /* Additional volatile operations to force register usage */
    volatile long sink = result;
    asm volatile("" : "+r"(result) : : "memory");
    
    return result + sink;
}

/* Another caller to create different calling context */
__attribute__((noipa, noinline))
long alternate_caller(int x, int y) {
    register long v1 = x * 11;
    register long v2 = y * 13;
    register long v3 = v1 + v2;
    register long v4 = v1 - v2;
    register long v5 = x * y;
    register long v6 = x + y * 3;
    register long v7 = y - x;
    register long v8 = x * x;
    
    /* Different computation pattern */
    v1 = v2 * v3 + v4;
    v2 = v3 * v4 - v5;
    
    /* Force conditional with different pattern */
    if (global_flag & 1) {
        callee_function();
    } else {
        asm volatile("" : : : "memory");
    }
    
    v3 = v4 * v5 + v6;
    v4 = v5 * v6 - v7;
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    /* Initialize with non-zero values */
    int seed = 42;
    
    /* Call multiple times with different parameters */
    long total = 0;
    for (int i = 0; i < 10; i++) {
        global_flag = (i % 3) + 1;  /* Change flag value */
        
        /* Call both functions to create different patterns */
        total += caller_function(seed + i, seed - i);
        total += alternate_caller(seed + i * 2, seed - i * 2);
        
        /* Memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %ld\n", total);
    
    /* Additional test with different optimization barriers */
    {
        volatile int flag = 1;
        long r1 = caller_function(100, 200);
        flag = 0;
        long r2 = alternate_caller(100, 200);
        printf("Additional: %ld\n", r1 + r2);
    }
    
    return 0;
}
