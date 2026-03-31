/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b - c * d / (e + f + 1);
    return result;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_calc(int x, int y, int z, int w, int v) {
    return (x * y) + (z / w) - (v << 2);
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int v5 = rand() % 100 + 1;
    volatile int v6 = rand() % 100 + 1;
    volatile int v7 = rand() % 100 + 1;
    volatile int v8 = rand() % 100 + 1;
    volatile int v9 = rand() % 100 + 1;
    volatile int v10 = rand() % 100 + 1;
    
    /* More variables to increase pressure */
    long l1 = v1 * 2L;
    long l2 = v2 * 3L;
    long l3 = v3 * 4L;
    long l4 = v4 * 5L;
    long l5 = v5 * 6L;
    long l6 = v6 * 7L;
    long l7 = v7 * 8L;
    long l8 = v8 * 9L;
    long l9 = v9 * 10L;
    long l10 = v10 * 11L;
    
    /* Additional variables with different types */
    unsigned long ul1 = l1 + 1000;
    unsigned long ul2 = l2 + 2000;
    unsigned long ul3 = l3 + 3000;
    unsigned long ul4 = l4 + 4000;
    
    /* Force mode mismatches with char/int mixing */
    char c1 = v1 & 0xFF;
    char c2 = v2 & 0xFF;
    short s1 = v3 & 0xFFFF;
    short s2 = v4 & 0xFFFF;
    
    /* Complex array indexing to force address reloads */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing with multiple variables */
    int idx1 = arr[v1 + v2 * v3 - v4];
    int idx2 = arr[v5 + v6 * v7 / (v8 + 1)];
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    long asm_result;
    asm volatile (
        /* Clobber multiple registers to force reloads */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input1] "r" (l1), [input2] "r" (l2)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* More inline assembly with different constraints */
    int asm_result2;
    asm volatile (
        "movl %[in1], %%ebx\n\t"
        "imull %[in2], %%ebx\n\t"
        "movl %%ebx, %[out]\n\t"
        : [out] "=r" (asm_result2)
        : [in1] "r" (v3), [in2] "r" (v4)
        : "rbx", "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        l1 + l2 * l3 - l4 / (l5 + 1) +
        ul1 - ul2 + ul3 * ul4 +
        (v1 * v2) + (v3 / v4) - (v5 << v6) +
        (c1 * s1) + (c2 / s2) +  /* Mode mixing */
        asm_result + asm_result2 +
        idx1 - idx2;
    
    /* Force more register pressure with function calls */
    long func_result = helper_func(l1, l2, l3, l4, l5, l6);
    int func_result2 = complex_calc(v7, v8, v9, v10, v1);
    
    /* Use volatile condition to prevent dead code elimination */
    volatile int condition = complex_result > 1000;
    if (condition) {
        /* More complex operations in conditional block */
        complex_result += arr[func_result2 % 100] * 2;
        
        /* Additional inline assembly in conditional path */
        asm volatile (
            "movq %[addr], %%rsi\n\t"
            "movl (%%rsi), %%eax\n\t"
            : 
            : [addr] "r" (&arr[0])
            : "rax", "rsi", "memory"
        );
    }
    
    /* Final computation using all variables */
    long final_result = 
        complex_result + 
        func_result + 
        func_result2 + 
        l7 + l8 + l9 + l10 +
        arr[v9 % 100] - arr[v10 % 100];
    
    /* Use the result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    /* Additional register pressure with loop */
    long accumulator = 0;
    for (int i = 0; i < 1000; i++) {
        /* Mix different types and operations */
        accumulator += arr[i % 100] * (i & 0xF);
        accumulator -= (i * v1) / (v2 + 1);
        
        /* Occasional inline assembly */
        if (i % 100 == 0) {
            asm volatile (
                "cpuid\n\t"
                : 
                : "a" (0)
                : "rbx", "rcx", "rdx"
            );
        }
    }
    
    printf("Accumulator: %ld\n", accumulator);
    
    return (final_result > 0) ? 0 : 1;
}
