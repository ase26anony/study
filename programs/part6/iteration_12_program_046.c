/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 * 100.0) + (int)(f2 * 50.0);
    result += (intptr_t)p1 % 1000 + (intptr_t)p2 % 500;
    return result;
}

/* Test 1: Extreme register pressure with many live variables */
int __attribute__((noinline)) test_register_pressure(int seed) {
    volatile int a = seed + 1;
    volatile int b = seed + 2;
    volatile int c = seed + 3;
    volatile int d = seed + 4;
    volatile int e = seed + 5;
    volatile int f = seed + 6;
    volatile int g = seed + 7;
    volatile int h = seed + 8;
    volatile int i = seed + 9;
    volatile int j = seed + 10;
    volatile int k = seed + 11;
    volatile int l = seed + 12;
    volatile int m = seed + 13;
    volatile int n = seed + 14;
    volatile int o = seed + 15;
    volatile int p = seed + 16;
    volatile int q = seed + 17;
    volatile int r = seed + 18;
    volatile int s = seed + 19;
    volatile int t = seed + 20;
    
    /* Many independent computations creating register pressure */
    int t1 = a + b * c;
    int t2 = d - e / (f ? f : 1);
    int t3 = g ^ h | i & j;
    int t4 = k << 2 + l >> 1;
    int t5 = m * n + o - p;
    int t6 = q % (r ? r : 1) + s * t;
    int t7 = t1 + t2 - t3;
    int t8 = t4 * t5 + t6;
    int t9 = t7 ^ t8 | t1 & t2;
    int t10 = t3 << t4 >> t5;
    int t11 = t6 + t7 - t8 * t9;
    int t12 = t10 % (t11 ? t11 : 1);
    int t13 = t1 * t2 + t3 - t4;
    int t14 = t5 ^ t6 | t7 & t8;
    int t15 = t9 << t10 >> t11;
    int t16 = t12 + t13 - t14 * t15;
    int t17 = t16 % (t1 ? t1 : 1);
    int t18 = t2 + t3 - t4 * t5;
    int t19 = t6 ^ t7 | t8 & t9;
    int t20 = t10 << t11 >> t12;
    
    /* Force all results to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int idx = index;
    
    /* Large immediate offset requiring reload */
    int val1 = global_array[4096 + idx];
    
    /* Complex index calculation */
    int val2 = global_array[(idx * 37 + 123) % 1000];
    
    /* Multi-level addressing */
    int val3 = global_array[global_array[idx % 100] + idx];
    
    /* 64-bit access on potentially misaligned boundary */
    long long val4 = global_big_array[(idx * 2 + 1) % 1000];
    
    /* Double precision floating with potential alignment issues */
    double val5 = global_double_array[(idx * 3) % 500];
    
    /* Combined complex addressing */
    int val6 = global_array[
        global_array[idx % 100] + 
        global_array[(idx + 100) % 100] + 
        2048  /* Large immediate */
    ];
    
    return val1 + val2 + val3 + (int)val4 + (int)val5 + val6;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 2;
    int b = y + 100;
    int c = a ^ b;
    int d = b - a;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber commonly used registers */
    asm volatile(
        "# Clobber many registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computations after clobber - forces reloads */
    int e = c * d + a;
    int f = b ^ d | c & a;
    int g = e << 2 + f >> 1;
    
    /* Another assembly block */
    asm volatile(
        "# Clobber more registers\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    return e + f + g;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Create many values that need to be in registers for function call */
    int a1 = base + 1;
    int a2 = base + 2;
    int a3 = base + 3;
    int a4 = base + 4;
    int a5 = base + 5;
    int a6 = base + 6;
    int a7 = base + 7;
    int a8 = base + 8;
    int a9 = base + 9;
    int a10 = base + 10;
    double f1 = base * 1.5;
    double f2 = base * 2.5;
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register allocation */
    int result1 = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                     f1, f2, p1, p2);
    
    /* Call again with different values */
    int result2 = many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                     f2, f1, p2, p1);
    
    return result1 + result2;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mixed size types */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed * 3;
    long long ll1 = (long long)seed * 1000;
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    
    /* Operations mixing types - may require conversions and reloads */
    int t1 = c1 + s1;
    long long t2 = ll1 + i1;
    double t3 = d1 + f1 + i1;
    
    /* Access with large immediate offset */
    int t4 = global_array[3000 + seed];
    
    /* Complex expression with mixed types */
    int result = t1 + (int)t2 + (int)t3 + t4 + r1 + r2;
    
    /* Force use of register variables */
    asm volatile("# Use register vars\n\t"
                 : "+r"(r1), "+r"(r2));
    
    return result;
}

/* Main orchestrator */
int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int result = 0;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", result);
    
    result += test_complex_addressing(seed);
    printf("Test 2 complete: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: %d\n", result);
    
    result += test_many_args(seed);
    printf("Test 4 complete: %d\n", result);
    
    result += test_mixed_types(seed);
    printf("Test 5 complete: %d\n", result);
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
