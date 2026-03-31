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
    volatile int idx = index;
    
    /* Large immediate offset forcing reload */
    int val1 = global_array[4096];  /* Large immediate offset */
    int val2 = global_array[8192];  /* Another large offset */
    
    /* Complex index calculation */
    int complex_idx = (idx * 37 + 12345) % 5000;
    
    /* Double register indirect-like access */
    int val3 = global_array[complex_idx + idx];
    
    /* Multi-word move (unaligned long long access) */
    long long ll_val = global_big_array[idx];
    
    /* Mixed type access forcing different register classes */
    double d_val = global_double_array[idx % 1000];
    
    /* Complex addressing with multiple operations */
    int val4 = global_array[(idx << 2) + (idx >> 1) + 256];
    
    /* Force use of all computed values */
    return val1 + val2 + val3 + (int)ll_val + (int)d_val + val4;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "# Clobber many registers to force spills\n\t"
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        : /* no outputs */
        : "r" (c), "r" (d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "memory", "cc"
    );
    
    /* More computations after clobber */
    int e = c * 11;
    int f = d * 13;
    int g = e + f;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# Clobber more registers\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    return g + a + b;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Prepare many arguments from complex expressions */
    int a1 = base + 1;
    int a2 = base * 2;
    int a3 = base / 3;
    int a4 = base - 4;
    int a5 = base % 5;
    int a6 = base << 1;
    int a7 = base >> 2;
    int a8 = base | 0xFF;
    int a9 = base & 0xF0;
    int a10 = base ^ 0x55;
    
    double f1 = (double)base * 1.5;
    double f2 = (double)base / 2.5;
    
    void *p1 = (void*)(intptr_t)(base + 1000);
    void *p2 = (void*)(intptr_t)(base + 2000);
    
    long long ll1 = (long long)base * 1000000LL;
    
    /* Call function with many arguments - each may need reloads */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, p1, p2, ll1);
    
    /* More computations after call */
    int b1 = result + a1;
    int b2 = result * a2;
    int b3 = result - a3;
    int b4 = result / (a4 ? a4 : 1);
    
    return b1 + b2 + b3 + b4;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mix different data types */
    short s1 = seed;
    short s2 = seed + 1;
    char c1 = seed & 0xFF;
    char c2 = (seed >> 8) & 0xFF;
    
    /* Long long operations forcing multi-register allocation */
    long long ll1 = (long long)seed * 1000LL;
    long long ll2 = (long long)seed * 2000LL;
    long long ll3 = ll1 + ll2;
    long long ll4 = ll1 * ll2;
    
    /* Double operations forcing floating point registers */
    double d1 = (double)seed * 1.234;
    double d2 = (double)seed * 5.678;
    double d3 = d1 + d2;
    double d4 = d1 * d2;
    
    /* Complex expression mixing all types */
    int result = r1 + r2 + s1 + s2 + c1 + c2;
    result += (int)(ll3 >> 32) + (int)ll4;
    result += (int)d3 + (int)d4;
    
    /* Access with complex addressing */
    result += global_array[result % 10000];
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 1000LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 3.14159;
    }
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    total += test_complex_addressing(seed % 500);
    total += test_asm_clobber(seed, seed * 2);
    total += test_many_args(seed);
    total += test_mixed_types(seed);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total == 0 ? 1 : 0;
}
