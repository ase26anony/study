/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long big_array[2000];
volatile double fp_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    volatile int result = 0;
    result += a1 + a2 + a3 + a4 + a5;
    result += a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2 + f3);
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
    
    /* Many independent arithmetic expressions to force register allocation */
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
    
    /* More computations to increase live range */
    int u1 = a * c;
    int u2 = e * g;
    int u3 = i * k;
    int u4 = m * o;
    int u5 = q * s;
    
    int u6 = u1 + u2;
    int u7 = u3 + u4;
    int u8 = u5 + u6;
    int u9 = u7 + u8;
    int u10 = u9 * 3;
    
    return t20 + u10;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with complex computation */
    int complex_idx = (index * 37 + 12345) % 5000;
    result += global_array[complex_idx];
    
    /* Double register indirect style access */
    int idx1 = index * 3;
    int idx2 = index * 7;
    result += global_array[idx1 + idx2];
    
    /* Misaligned 64-bit access forcing piecewise moves */
    volatile long long ll_result = 0;
    ll_result += big_array[index];
    ll_result += big_array[index + 100];
    
    /* Floating point with potential reloads */
    volatile double fp_result = 0.0;
    fp_result += fp_array[index % 100];
    fp_result += fp_array[(index * 17) % 100];
    
    return result + (int)ll_result + (int)fp_result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
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
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computations after clobber - values must be reloaded */
    int e = c * 2;
    int f = d * 3;
    int g = e + f;
    
    /* Another assembly block with different clobbers */
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
    
    return g + a + b;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Create many values that need to be live across calls */
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
    
    /* Call function with many arguments - forces register allocation */
    int result1 = many_args_func(
        v1, v2, v3, v4, v5,
        v6, v7, v8, v9, v10,
        1.1, 2.2, 3.3,
        (void*)(intptr_t)v1,
        (void*)(intptr_t)v2
    );
    
    /* More computations to keep values live */
    int intermediate = v1 * v2 + v3 * v4 + v5 * v6;
    
    /* Another call with shuffled arguments */
    int result2 = many_args_func(
        v10, v9, v8, v7, v6,
        v5, v4, v3, v2, v1,
        4.4, 5.5, 6.6,
        (void*)(intptr_t)v3,
        (void*)(intptr_t)v4
    );
    
    return result1 + result2 + intermediate;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mix different sized types */
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    int i1 = seed;
    long long ll1 = (long long)seed * 1000;
    double d1 = (double)seed / 3.0;
    
    /* Operations mixing types - may require conversions and reloads */
    int t1 = c1 + s1;
    long long t2 = i1 + ll1;
    double t3 = d1 + (double)i1;
    
    /* Access global with complex addressing */
    int idx = (r1 + r2) % 100;
    volatile int mem1 = global_array[idx];
    volatile long long mem2 = big_array[idx % 50];
    
    /* More mixed operations */
    int result = t1 + (int)(t2 % 1000) + (int)t3 + mem1 + (int)mem2;
    
    /* Force use of register variables */
    asm volatile("# Use r10, r11" : : "r"(r1), "r"(r2));
    
    return result + r1 + r2;
}

int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    printf("After register pressure test: %d\n", total);
    
    total += test_complex_addressing(seed % 100);
    printf("After complex addressing test: %d\n", total);
    
    total += test_asm_clobber(seed, seed * 2);
    printf("After asm clobber test: %d\n", total);
    
    total += test_many_args(seed);
    printf("After many args test: %d\n", total);
    
    total += test_mixed_types(seed);
    printf("After mixed types test: %d\n", total);
    
    printf("Final result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    return total == 0 ? 1 : 0;
}
