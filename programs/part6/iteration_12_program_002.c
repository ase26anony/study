/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Force usage of all arguments */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
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
    
    /* Many independent computations to force register allocation */
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
    
    /* Force all values to be used */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                     "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10),
                     "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15),
                     "r"(t16), "r"(t17), "r"(t18), "r"(t19), "r"(t20));
    
    return t20;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int idx = index;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096 + idx];
    
    /* Complex index calculation */
    int complex_idx = (idx * 7 + 3) / 2;
    int val2 = global_array[complex_idx];
    
    /* Multi-word move with potential alignment issues */
    long long big_val = global_big_array[idx];
    
    /* Double with potential reload for floating point registers */
    double dbl_val = global_double_array[idx];
    
    /* Combined addressing with multiple operations */
    int val3 = global_array[global_array[idx] + idx * 3];
    
    /* Force usage of all loaded values */
    int result = val1 + val2 + (int)big_val + (int)dbl_val + val3;
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
    /* Clobber many registers to force spills and reloads */
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
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "cc", "memory"
    );
    
    /* More computations after clobber */
    int e = c * d;
    int f = e + a;
    int g = f * b;
    
    /* Another clobber */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7");
    
    return g;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
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
    
    void* p1 = (void*)(intptr_t)(base + 100);
    void* p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register allocation
       for argument passing according to calling convention */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    /* Do more work to keep values live */
    result += a1 + a2 + a3;
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables to constrain register allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mix different sized types */
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    int i1 = seed;
    long long ll1 = (long long)seed * 1000;
    
    /* Force conversions and moves between different register classes */
    double d1 = (double)seed / 3.0;
    float f1 = (float)seed / 2.0f;
    
    /* Complex expression mixing all types */
    int result = (int)((r1 + r2) * (i1 + (int)ll1) + (int)(d1 * 100.0) + (int)(f1 * 50.0f));
    
    /* Force use of all variables */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(c1), "r"(s1), "r"(i1),
                     "r"(ll1), "x"(d1), "x"(f1));
    
    return result;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing reload pass with seed = %d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("test_register_pressure: %d\n", result);
    
    result += test_complex_addressing(seed % 100);
    printf("test_complex_addressing: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("test_asm_clobber: %d\n", result);
    
    result += test_many_args(seed);
    printf("test_many_args: %d\n", result);
    
    result += test_mixed_types(seed);
    printf("test_mixed_types: %d\n", result);
    
    /* Access global with large offset to potentially trigger reload */
    result += global_array[4096];
    
    printf("Final result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
