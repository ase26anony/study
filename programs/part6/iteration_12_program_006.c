/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[20000];
volatile double global_double_array[5000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void* p1, void* p2)
{
    volatile int result = 0;
    result += a1 + a2 + a3 + a4 + a5;
    result += a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
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
    
    /* Force many independent computations to create live ranges */
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

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int result = 0;
    
    /* Large immediate offset - may need reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int complex_idx = (index * 3 + 7) / 2;
    result += global_array[complex_idx + 1000];
    
    /* Multi-word type with potential alignment issues */
    long long ll_val = global_big_array[index + 2000];
    result += (int)(ll_val >> 32) + (int)ll_val;
    
    /* Double type that might need special handling */
    double d_val = global_double_array[index + 100];
    result += (int)d_val;
    
    /* Nested array access with complex computation */
    result += global_array[global_array[index] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Force values to be in registers before asm */
    volatile int pre_asm = a * b + c * d;
    
    /* Clobber many registers - force spills and reloads */
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
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* Use values after asm - forcing reloads */
    volatile int post_asm = pre_asm + a + b + c + d;
    
    return post_asm;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    volatile int v1 = base + 1;
    volatile int v2 = base + 2;
    volatile int v3 = base + 3;
    volatile int v4 = base + 4;
    volatile int v5 = base + 5;
    volatile int v6 = base + 6;
    volatile int v7 = base + 7;
    volatile int v8 = base + 8;
    volatile int v9 = base + 9;
    volatile int v10 = base + 10;
    volatile double f1 = base * 1.5;
    volatile double f2 = base * 2.5;
    volatile void* p1 = (void*)&global_array;
    volatile void* p2 = (void*)&global_big_array;
    
    /* Call with many args - forces register allocation pressure */
    int result = many_args_func(
        v1, v2, v3, v4, v5,
        v6, v7, v8, v9, v10,
        f1, f2, p1, p2
    );
    
    /* More computations after call */
    result += v1 * v2;
    result += v3 * v4;
    result += v5 * v6;
    result += v7 * v8;
    result += v9 * v10;
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Try to use explicit register variables (if supported) */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mixed size types */
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    int i1 = seed;
    long long ll1 = (long long)seed * 1000;
    
    /* Force operations between different sized types */
    int t1 = c1 + s1;
    int t2 = i1 + (int)ll1;
    long long t3 = ll1 + i1;
    
    /* Use explicit register variables in computation */
    int t4 = r1 + r2;
    int t5 = t1 + t2;
    long long t6 = t3 + t4 + t5;
    
    /* Access misaligned data */
    struct __attribute__((packed)) {
        char a;
        int b;
        char c;
        long long d;
    } packed_struct;
    
    packed_struct.a = seed & 0xFF;
    packed_struct.b = seed;
    packed_struct.c = (seed >> 8) & 0xFF;
    packed_struct.d = (long long)seed * 10000;
    
    int result = packed_struct.b + (int)packed_struct.d + t4;
    
    return result;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", result);
    
    result += test_complex_addressing(seed % 1000);
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
