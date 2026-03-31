/* test_caller_save.c - Program to trigger specific RTL instruction chain manipulation
   in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc) */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Force clobbering of caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    /* x86/x86_64 caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "memory");
#elif defined(__aarch64__)
    /* ARM64 caller-saved registers */
    asm volatile("" : : : 
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18",
        "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
        "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
        "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
        "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
        "memory");
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure across call */
__attribute__((noipa, noinline, noclone, optimize("no-inline")))
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
    
    /* Create data dependencies before the call */
    a = b * c + d;
    b = c + d * e;
    c = d - e * f;
    d = e + f / g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile store to force register spilling */
    volatile int64_t temp = a + b + c + d;
    
    /* Conditional call based on volatile flag - creates basic block boundary */
    if (global_volatile_flag) {
        /* This call should trigger caller-save register preservation */
        callee_function();
        
        /* Additional computation in the same basic block after the call */
        temp = temp + 1;
    } else {
        /* Alternative path without call */
        temp = temp - 1;
    }
    
    /* More computations using all variables after the call */
    e = f * g + h;
    f = g + h * i;
    g = h - i * j;
    h = i + j / a;
    i = j * a + b;
    j = a + b * c;
    
    /* Complex expression using all variables to keep them live */
    int64_t result = a + b * 2 + c * 3 + d * 4 + e * 5 + 
                     f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use volatile variable to prevent dead code elimination */
    result += temp;
    
    return result;
}

/* Another caller to create different calling context */
__attribute__((noipa, noinline))
int64_t caller_function2(int64_t seed) {
    register int64_t v1 = seed + 100;
    register int64_t v2 = seed * 200;
    register int64_t v3 = seed / 300;
    register int64_t v4 = seed - 400;
    register int64_t v5 = seed + 500;
    
    /* Different computation pattern */
    v1 = v2 * v3;
    v2 = v4 + v5;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Nested conditional with call */
    if (global_volatile_flag > 0) {
        if (seed % 2) {
            callee_function();
            v3 = v4 * v5;
        } else {
            v3 = v4 / v5;
        }
    }
    
    v4 = v5 + v1;
    v5 = v2 * v3;
    
    return v1 + v2 + v3 + v4 + v5;
}

int main(void) {
    int64_t total = 0;
    
    /* Call multiple times with different seeds to explore different paths */
    for (int64_t seed = 0; seed < 100; seed++) {
        /* Toggle the volatile flag to affect conditional paths */
        global_volatile_flag = (seed % 3) ? 1 : 0;
        
        /* Call both functions to create different register pressure scenarios */
        total += caller_function(seed);
        total += caller_function2(seed + 1000);
        
        /* Prevent loop unrolling for more predictable basic block structure */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", (long)total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
