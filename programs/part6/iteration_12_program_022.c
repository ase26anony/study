/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[20000];
volatile double global_double_array[5000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                    int e, int f, int g, int h)
{
    /* Use volatile inputs to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    volatile int v7 = g;
    volatile int v8 = h;
    
    /* Create many independent computations to force register pressure */
    int t1 = v1 + v2;
    int t2 = v3 * v4;
    int t3 = v5 - v6;
    int t4 = v7 ^ v8;
    int t5 = v1 * v3;
    int t6 = v2 + v4;
    int t7 = v5 * v7;
    int t8 = v6 ^ v8;
    int t9 = t1 + t2;
    int t10 = t3 - t4;
    int t11 = t5 * t6;
    int t12 = t7 ^ t8;
    int t13 = t9 + t10;
    int t14 = t11 - t12;
    int t15 = t13 * t14;
    int t16 = t1 ^ t3;
    int t17 = t5 + t7;
    int t18 = t9 - t11;
    int t19 = t13 ^ t15;
    int t20 = t2 * t4;
    int t21 = t6 + t8;
    int t22 = t10 - t12;
    int t23 = t14 ^ t16;
    int t24 = t17 * t18;
    int t25 = t19 + t20;
    int t26 = t21 - t22;
    int t27 = t23 * t24;
    int t28 = t25 ^ t26;
    int t29 = t27 + t28;
    int t30 = t15 - t18;
    
    /* Force all values to be used together */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                     int index3, int index4)
{
    volatile int idx1 = index1;
    volatile int idx2 = index2;
    volatile int idx3 = index3;
    volatile int idx4 = index4;
    
    /* Large immediate offsets */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Complex index calculations */
    int val3 = global_array[idx1 * idx2 + 256];
    int val4 = global_array[idx3 * 17 + idx4 * 13];
    
    /* Multi-word types forcing piecewise moves */
    long long ll1 = global_big_array[idx1 + 2048];
    long long ll2 = global_big_array[idx2 * 2 + 1024];
    
    /* Double precision requiring specific handling */
    double d1 = global_double_array[idx3];
    double d2 = global_double_array[idx4 + 1000];
    
    /* Mixed addressing modes in expressions */
    int result = val1 + val2 + val3 + val4;
    result += (int)(ll1 >> 32) + (int)(ll2 & 0xFFFFFFFF);
    result += (int)(d1 + d2);
    
    return result;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int x1 = a * b;
    int x2 = c + d;
    int x3 = a ^ c;
    int x4 = b * d;
    
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
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More computations after clobber */
    int y1 = x1 * x2;
    int y2 = x3 + x4;
    int y3 = y1 ^ y2;
    int y4 = x1 * x3;
    
    /* Another clobber */
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
    
    return y1 + y2 + y3 + y4;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int v = base;
    
    /* Prepare many arguments with complex computations */
    int a1 = v * 1;
    int a2 = v * 2;
    int a3 = v * 3;
    int a4 = v * 4;
    int a5 = v * 5;
    int a6 = v * 6;
    int a7 = v * 7;
    int a8 = v * 8;
    int a9 = v * 9;
    int a10 = v * 10;
    double f1 = v * 1.5;
    double f2 = v * 2.5;
    void *p1 = (void*)(intptr_t)(v * 100);
    void *p2 = (void*)(intptr_t)(v * 200);
    
    /* Call function with many arguments - each needs register/stack slot */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, p1, p2);
    
    /* Call it again with different values */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                f2, f1, p2, p1);
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, int c, int d)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = a + b;
    register int r2 asm("r11") = c * d;
    
    /* 64-bit operations */
    long long ll1 = (long long)a * b * c * d;
    long long ll2 = ll1 << 3;
    long long ll3 = ll2 >> 2;
    
    /* Floating point mixed with integer */
    double d1 = (double)a / (b + 1);
    double d2 = (double)c * d;
    
    /* Structure with mixed types */
    struct mixed {
        int i;
        long long ll;
        double d;
    } m;
    
    m.i = r1 + r2;
    m.ll = ll1 + ll2 + ll3;
    m.d = d1 * d2;
    
    /* Access structure elements */
    int result = m.i + (int)(m.ll >> 32) + (int)m.d;
    
    /* Force use of register variables */
    asm volatile("# Using r10, r11" : : "r"(r1), "r"(r2));
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize some global array values */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 20000; i++) {
        global_big_array[i] = i * 5LL;
    }
    for (int i = 0; i < 5000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3,
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(base, base+10, base+20, base+30);
    
    total += test_asm_clobber(base, base+1, base+2, base+3);
    
    total += test_many_args(base);
    
    total += test_mixed_types(base, base+1, base+2, base+3);
    
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
