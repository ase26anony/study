/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Use all arguments to prevent elimination */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
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
    
    /* Create many independent computations to force register allocation */
    int t1 = v1 + v2;
    int t2 = v3 * v4;
    int t3 = v5 ^ v6;
    int t4 = v7 | v8;
    int t5 = t1 * t2;
    int t6 = t3 + t4;
    int t7 = t5 - t6;
    int t8 = v1 * v3;
    int t9 = v2 * v4;
    int t10 = v5 * v7;
    int t11 = v6 * v8;
    int t12 = t8 + t9;
    int t13 = t10 + t11;
    int t14 = t12 * t13;
    int t15 = t7 + t14;
    int t16 = v1 + v3 + v5 + v7;
    int t17 = v2 + v4 + v6 + v8;
    int t18 = t16 * t17;
    int t19 = t15 - t18;
    int t20 = (v1 << 2) + (v2 << 3);
    int t21 = (v3 >> 1) + (v4 >> 2);
    int t22 = t19 + t20 + t21;
    
    /* More computations to exceed register file */
    int t23 = t22 * 31415;
    int t24 = t23 / 27182;
    int t25 = t24 % 10007;
    int t26 = t25 ^ 0xABCDEF;
    int t27 = t26 | 0x123456;
    int t28 = t27 & 0xF0F0F0F;
    int t29 = t28 << 4;
    int t30 = t29 >> 2;
    
    /* Use all temporaries in final computation */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index, int offset)
{
    volatile int idx = index;
    volatile int off = offset;
    
    /* Large immediate offset requiring reload */
    int val1 = global_array[4096];  /* Large immediate offset */
    int val2 = global_array[8192];  /* Another large immediate */
    
    /* Variable index with computation */
    int val3 = global_array[idx * 2 + 100];
    int val4 = global_array[off * 3 + 200];
    
    /* Complex addressing with multiple computations */
    int val5 = global_array[(idx * off) % 1000 + 3000];
    
    /* Misaligned access for 64-bit types on 32-bit arch */
    long long ll1 = global_big_array[idx];
    long long ll2 = global_big_array[off];
    
    /* Double register indirect-like pattern */
    int complex_idx = (idx * 17 + off * 23) % 500;
    int val6 = global_array[global_array[complex_idx] % 1000];
    
    /* Use double type which may need special handling */
    double d1 = global_double_array[idx % 100];
    double d2 = global_double_array[off % 100];
    
    /* Mixed type computations to force different register classes */
    return val1 + val2 + val3 + val4 + val5 + val6 + 
           (int)ll1 + (int)ll2 + (int)d1 + (int)d2;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int result;
    
    /* Do some computation that uses registers */
    int t1 = a * b + c * d;
    int t2 = (a << 3) | (b << 2);
    int t3 = c ^ d;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber general purpose registers */
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
    int t4 = t1 * t2;
    int t5 = t3 + t4;
    int t6 = t5 ^ a;
    int t7 = t6 | b;
    
    /* Another assembly block with different clobbers */
    asm volatile(
        "# Clobber more registers\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "r14", "r15", "cc"
    );
    
    result = t7 * c + d;
    return result;
}

/* Test 4: Function with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many different values for arguments */
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
    
    /* Call function with many arguments - forces register/stack moves */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    /* Do more work after call */
    result += a1 + a2 + a3;
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b)
{
    /* Try to use explicit register variables if supported */
    #ifdef __GNUC__
    register int r1 asm("r10") = a * 3;
    register int r2 asm("r11") = b * 5;
    #else
    register int r1 = a * 3;
    register int r2 = b * 5;
    #endif
    
    /* Use 64-bit types on 32-bit arch */
    long long ll1 = (long long)a * b;
    long long ll2 = (long long)a << 32;
    long long ll3 = ll1 + ll2;
    
    /* Use double computations */
    double d1 = a * 1.41421356;
    double d2 = b * 3.14159265;
    double d3 = d1 * d2;
    
    /* Structure with mixed types */
    struct mixed {
        int i;
        long long ll;
        double d;
    } m1, m2;
    
    m1.i = a;
    m1.ll = ll1;
    m1.d = d1;
    
    m2.i = b;
    m2.ll = ll2;
    m2.d = d2;
    
    /* Access structure members - may require complex addressing */
    int sum_i = m1.i + m2.i;
    long long sum_ll = m1.ll + m2.ll;
    double sum_d = m1.d + m2.d;
    
    return r1 + r2 + (int)ll3 + (int)sum_ll + (int)sum_d + sum_i;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    int total = 0;
    
    /* Run test 1: Register pressure */
    total += test_register_pressure(base, base+1, base+2, base+3,
                                   base+4, base+5, base+6, base+7);
    
    /* Run test 2: Complex addressing */
    total += test_complex_addressing(base % 100, (base + 50) % 100);
    
    /* Run test 3: Assembly clobber */
    total += test_asm_clobber(base, base+10, base+20, base+30);
    
    /* Run test 4: Many arguments */
    total += test_many_args(base);
    
    /* Run test 5: Mixed types */
    total += test_mixed_types(base, base+100);
    
    /* Print result to ensure all code is live */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
