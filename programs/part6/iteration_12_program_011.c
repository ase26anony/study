/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long big_global[8192];
volatile double fp_global[4096];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Force use of all arguments to prevent elimination */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Use volatile inputs to prevent constant propagation */
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
    
    /* Many independent computations to create live ranges */
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
    
    /* More computations to increase pressure */
    int u1 = a * c;
    int u2 = e * g;
    int u3 = i * k;
    int u4 = m * o;
    int u5 = q * s;
    
    int u6 = b * d;
    int u7 = f * h;
    int u8 = j * l;
    int u9 = n * p;
    int u10 = r * t;
    
    int u11 = u1 + u2 + u3 + u4 + u5;
    int u12 = u6 + u7 + u8 + u9 + u10;
    
    return t20 + u11 + u12;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int val3 = global_array[idx * 3 + 100];
    int val4 = global_array[idx * 7 + 2000];
    
    /* Complex base + index * scale */
    int val5 = global_array[(idx * idx) % 1000 + 3000];
    
    /* Multi-word types forcing piecewise moves */
    long long ll1 = big_global[idx];
    long long ll2 = big_global[idx + 100];
    long long ll3 = big_global[idx + 200];
    
    /* Floating point with alignment requirements */
    double d1 = fp_global[idx % 100];
    double d2 = fp_global[(idx + 50) % 100];
    
    /* Mixed addressing modes */
    int val6 = global_array[((int)ll1 + idx) % 5000];
    int val7 = global_array[((int)d1 + idx * 2) % 5000];
    
    return val1 + val2 + val3 + val4 + val5 + val6 + val7 + (int)ll1 + (int)ll2 + (int)ll3 + (int)d1 + (int)d2;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 2;
    int b = y * 3;
    int c = a + b;
    int d = a * b;
    
    /* Clobber many registers to force spills */
    __asm__ volatile (
        "# Clobber many registers\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    int e = c * d;
    int f = e + a;
    int g = f * b;
    
    /* Another asm with different clobbers */
    __asm__ volatile (
        "# More clobbering\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    return g + e + f;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many values to pass as arguments */
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
    
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many args - forces register/stack moves */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, f3, p1, p2);
    
    /* Call it again with different values */
    int result2 = many_args_func(a2, a3, a4, a5, a6, a7, a8, a9, a10, a1,
                                 f3, f1, f2, p2, p1);
    
    return result + result2;
}

/* Test 5: Explicit register variables and special types */
int __attribute__((noinline)) test_explicit_registers(int x)
{
    /* Try to use explicit registers (GCC extension) */
    register int r10_val asm("r10") = x * 2;
    register int r11_val asm("r11") = x * 3;
    
    /* Force use of these register variables in computations */
    int a = r10_val + 1;
    int b = r11_val + 2;
    
    /* Use them in complex expressions */
    for (int i = 0; i < 10; i++) {
        a = a * 2 + r10_val;
        b = b * 3 + r11_val;
        r10_val = r10_val + i;
        r11_val = r11_val - i;
    }
    
    /* Use 80-bit long double which may use x87 registers */
    volatile long double ld1 = x * 1.23456789L;
    volatile long double ld2 = x * 9.87654321L;
    volatile long double ld3 = ld1 * ld2;
    
    return a + b + (int)ld3;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Initialize globals */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 2;
        if (i < 8192) big_global[i] = i * 3LL;
        if (i < 4096) fp_global[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", total);
    
    total += test_complex_addressing(seed % 100);
    printf("Test 2 complete: %d\n", total);
    
    total += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: %d\n", total);
    
    total += test_many_args(seed);
    printf("Test 4 complete: %d\n", total);
    
    total += test_explicit_registers(seed);
    printf("Test 5 complete: %d\n", total);
    
    printf("Final result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    return total == 0 ? 1 : 0;
}
