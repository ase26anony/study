/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long big_global[2000];
volatile double fp_global[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2, long long ll1)
{
    /* Force use of all arguments to prevent elimination */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    result += (int)ll1;
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
    
    /* Many independent arithmetic expressions to create live ranges */
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
    int t20 = t19 * seed;
    
    /* Force all values to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int complex_index;
    
    /* Force computation of index in memory */
    asm volatile("" : "=r"(complex_index) : "0"(index));
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096 + complex_index];
    
    /* Double register indirect-like access */
    int val2 = global_array[(complex_index * 37) & 0xFFF];
    
    /* Multi-word move with potential alignment issues */
    long long ll_val = big_global[complex_index % 1000];
    
    /* Mixed size accesses */
    double d_val = fp_global[complex_index % 500];
    
    /* Complex expression in array index */
    int val3 = global_array[(val1 + val2 + (int)ll_val) % 10000];
    
    return val1 + val2 + val3 + (int)d_val + (int)ll_val;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
    /* Clobber many registers - forces spills and reloads */
    asm volatile(
        "# Clobber many registers\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Use values after clobber - will need reloads */
    int e = c * d;
    int f = a + c + d;
    
    asm volatile(
        "# Another clobber\n\t"
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx"
    );
    
    return e + f + a + b;
}

/* Test 4: Function call with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(int base) {
    /* Compute many values that need to be passed in registers */
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
    
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    long long ll1 = (long long)base * 1000LL;
    
    /* This call will need to move many values to specific registers */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, p1, p2, ll1);
    
    /* More computations after call to create overlapping live ranges */
    int b1 = result + a1;
    int b2 = result + a2;
    int b3 = result + a3;
    int b4 = result + a4;
    
    return b1 + b2 + b3 + b4 + result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Try to use explicit registers (GCC extension) */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mixed size operations */
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed * 2000000LL;
    
    /* Floating point mixed with integer */
    double d1 = seed * 1.234;
    double d2 = seed * 5.678;
    
    /* Force use of explicit register variables */
    int sum_int = r1 + r2 + (int)ll1 + (int)ll2;
    
    /* Operations that might need type conversions */
    double sum_fp = d1 + d2 + (double)r1;
    
    /* Access with large offset */
    int mem_val = global_array[3000 + (seed % 100)];
    
    /* Complex expression forcing temporary values */
    int result = (sum_int * (int)sum_fp) + mem_val + 
                 ((int)d1 * (int)d2) + (r1 * r2);
    
    return result;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    result += test_complex_addressing(seed);
    result += test_asm_clobber(seed, seed * 2);
    result += test_many_args(seed);
    result += test_mixed_types(seed);
    
    /* Force use of global arrays to prevent elimination */
    global_array[0] = result;
    big_global[0] = result;
    fp_global[0] = result;
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
