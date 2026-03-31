/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Force use of all arguments to prevent elimination */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
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
    
    /* Many independent computations to create register pressure */
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
    int t21 = t20 + a;
    int t22 = t21 + b;
    int t23 = t22 + c;
    int t24 = t23 + d;
    int t25 = t24 + e;
    int t26 = t25 + f;
    int t27 = t26 + g;
    int t28 = t27 + h;
    int t29 = t28 + i;
    int t30 = t29 + j;
    
    /* Force all values to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Large immediate offset requiring reload */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Variable index with complex computation */
    int complex_idx = (idx * 3 + 7) / 2;
    int val3 = global_array[complex_idx];
    
    /* Double register indirect style access */
    int base = idx * 100;
    int offset = (idx % 10) * 8;
    int val4 = global_array[base + offset];
    
    /* Misaligned 64-bit access on 32-bit boundary */
    long long big_val = global_big_array[idx];
    
    /* Double precision floating with potential alignment issues */
    double dbl_val = global_double_array[idx];
    
    /* Mixed size accesses */
    int val5 = ((int*)global_big_array)[idx * 2 + 1];
    
    return val1 + val2 + val3 + val4 + (int)big_val + (int)dbl_val + val5;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = c * 2;
    
    /* Clobber many registers to force spills */
    asm volatile(
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
    
    /* More computations after clobber */
    int e = d + 100;
    int f = e * 3;
    int g = f - x;
    int h = g + y;
    
    /* Another clobber with different registers */
    asm volatile(
        "# Clobber more registers\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    return a + b + c + d + e + f + g + h;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many values that need to be passed */
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
    double f3 = base * 3.5;
    
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - each may need reloads */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, f3, p1, p2);
    
    /* More computations to keep values live around call */
    int b1 = a1 * 2;
    int b2 = a2 * 3;
    int b3 = a3 * 4;
    
    return result + b1 + b2 + b3;
}

/* Test 5: Explicit register variables and special types */
int __attribute__((noinline)) test_register_vars(int x)
{
    /* Try to use specific registers (GCC may honor these) */
    register int r10_val asm("r10") = x * 2;
    register int r11_val asm("r11") = x * 3;
    
    /* Use 80-bit floating type (x87) if available */
    volatile long double ld1 = x * 1.5L;
    volatile long double ld2 = x * 2.5L;
    
    /* Mixed size operations */
    char c1 = x & 0xFF;
    short s1 = x & 0xFFFF;
    int i1 = x;
    long long ll1 = (long long)x * 1000;
    
    /* Force use of register variables */
    asm volatile("# Use r10: %0" : : "r"(r10_val));
    asm volatile("# Use r11: %0" : : "r"(r11_val));
    
    /* Complex expression with mixed types */
    long long result = (long long)c1 + (long long)s1 * 2 + (long long)i1 * 3 + ll1;
    result += (long long)(ld1 + ld2);
    
    return (int)result;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    printf("Starting reload pass tests...\n");
    
    /* Run each test with different seeds */
    total += test_register_pressure(seed);
    total += test_complex_addressing(seed % 1000);
    total += test_asm_clobber(seed, seed * 2);
    total += test_many_args(seed);
    total += test_register_vars(seed);
    
    /* Access globals to keep them alive */
    total += global_array[0];
    total += (int)global_big_array[0];
    total += (int)global_double_array[0];
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
