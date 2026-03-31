/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile int global_array[10000];
volatile long long global_big_array[20000];
volatile double global_double_array[5000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation to prevent elimination */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double fsum = f1 + f2 + f3;
    return sum + (int)fsum + (int)((intptr_t)p1 ^ (intptr_t)p2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    volatile int a = seed;
    volatile int b = seed + 1;
    volatile int c = seed + 2;
    volatile int d = seed + 3;
    volatile int e = seed + 4;
    volatile int f = seed + 5;
    volatile int g = seed + 6;
    volatile int h = seed + 7;
    volatile int i = seed + 8;
    volatile int j = seed + 9;
    volatile int k = seed + 10;
    volatile int l = seed + 11;
    volatile int m = seed + 12;
    volatile int n = seed + 13;
    volatile int o = seed + 14;
    volatile int p = seed + 15;
    volatile int q = seed + 16;
    volatile int r = seed + 17;
    volatile int s = seed + 18;
    volatile int t = seed + 19;
    
    /* Many independent computations creating register pressure */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = k + l;
    int t7 = m + n;
    int t8 = o + p;
    int t9 = q + r;
    int t10 = s + t;
    
    int t11 = t1 * t2;
    int t12 = t3 * t4;
    int t13 = t5 * t6;
    int t14 = t7 * t8;
    int t15 = t9 * t10;
    
    int t16 = t11 + t12;
    int t17 = t13 + t14;
    int t18 = t15 + t16;
    int t19 = t17 + t18;
    int t20 = t19 * 2;
    
    /* More computations to ensure values stay live */
    int t21 = t20 + a - b;
    int t22 = t21 + c - d;
    int t23 = t22 + e - f;
    int t24 = t23 + g - h;
    int t25 = t24 + i - j;
    int t26 = t25 + k - l;
    int t27 = t26 + m - n;
    int t28 = t27 + o - p;
    int t29 = t28 + q - r;
    int t30 = t29 + s - t;
    
    return t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096] + global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int val2 = global_array[idx * 3 + 100];
    
    /* Complex addressing with multiple computations */
    int val3 = global_array[(idx * idx) / 2 + 5000];
    
    /* Misaligned access simulation with long long */
    long long ll1 = global_big_array[idx];
    long long ll2 = global_big_array[idx + 1000];
    
    /* Double type with potential alignment issues */
    double d1 = global_double_array[idx % 1000];
    double d2 = global_double_array[(idx + 500) % 1000];
    
    /* Combined addressing with large offset */
    int val4 = global_array[global_array[idx] + 2000];
    
    return val1 + val2 + val3 + (int)ll1 + (int)ll2 + (int)d1 + (int)d2 + val4;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 2;
    int b = y * 3;
    int c = a + b;
    int d = c * 5;
    int e = d - x;
    int f = e + y;
    int g = f * 2;
    int h = g - a;
    
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
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More computations after clobber - forces reloads */
    int i = h + 100;
    int j = i * b;
    int k = j - c;
    int l = k + d;
    int m = l * e;
    int n = m - f;
    int o = n + g;
    
    return o;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
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
    
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    double f3 = base * 3.3;
    
    void *p1 = &global_array[0];
    void *p2 = &global_big_array[1000];
    
    /* Call with many args - forces register pressure for argument passing */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, f3, p1, p2);
    
    /* More computations to keep values live */
    int t1 = result + a1;
    int t2 = t1 * a2;
    int t3 = t2 - a3;
    int t4 = t3 + a4;
    int t5 = t4 * a5;
    int t6 = t5 - a6;
    int t7 = t6 + a7;
    int t8 = t7 * a8;
    int t9 = t8 - a9;
    int t10 = t9 + a10;
    
    return t10;
}

/* Test 5: Mixed types and explicit register variables */
#ifdef __x86_64__
int __attribute__((noinline)) test_mixed_types(int seed)
{
    /* Explicit register variables - compete for specific registers */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Floating point computations - use different register class */
    double d1 = seed * 1.5;
    double d2 = seed * 2.5;
    double d3 = d1 + d2;
    double d4 = d3 * 1.1;
    
    /* Long long computations - may require multiple registers */
    long long ll1 = seed * 1000LL;
    long long ll2 = seed * 2000LL;
    long long ll3 = ll1 + ll2;
    long long ll4 = ll3 * 3LL;
    
    /* Mix all types together */
    int i1 = r1 + r2;
    int i2 = (int)d3 + (int)d4;
    int i3 = (int)(ll3 >> 32) + (int)ll4;
    
    /* Complex addressing with mixed types */
    int idx = (i1 + i2 + i3) % 1000;
    double d5 = global_double_array[idx] + d1;
    long long ll5 = global_big_array[idx] + ll1;
    
    return i1 + i2 + i3 + (int)d5 + (int)ll5;
}
#endif

int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int result = 0;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("test_register_pressure: %d\n", result);
    
    result += test_complex_addressing(seed % 1000);
    printf("test_complex_addressing: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("test_asm_clobber: %d\n", result);
    
    result += test_many_args(seed);
    printf("test_many_args: %d\n", result);
    
#ifdef __x86_64__
    result += test_mixed_types(seed);
    printf("test_mixed_types: %d\n", result);
#endif
    
    /* Final complex computation to ensure all values are used */
    result = result * 2 - seed;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
