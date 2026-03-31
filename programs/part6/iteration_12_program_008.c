/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                    volatile int c, volatile int d)
{
    /* Force many independent computations that must stay live */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = a * b;
    int t4 = c * d;
    int t5 = a - b;
    int t6 = c - d;
    int t7 = a ^ b;
    int t8 = c ^ d;
    int t9 = a | b;
    int t10 = c | d;
    int t11 = a & b;
    int t12 = c & d;
    int t13 = ~a;
    int t14 = ~c;
    int t15 = t1 + t2;
    int t16 = t3 + t4;
    int t17 = t5 + t6;
    int t18 = t7 + t8;
    int t19 = t9 + t10;
    int t20 = t11 + t12;
    int t21 = t13 + t14;
    int t22 = t15 + t16;
    int t23 = t17 + t18;
    int t24 = t19 + t20;
    int t25 = t21 + t22;
    int t26 = t23 + t24;
    int t27 = t25 + t26;
    
    /* Use all temporaries to prevent dead code elimination */
    volatile int sum = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                      t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                      t21 + t22 + t23 + t24 + t25 + t26 + t27;
    
    return sum;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                     volatile int idx3, volatile int idx4)
{
    int result = 0;
    
    /* Large immediate offset - may need reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with arithmetic - double register indirect */
    result += global_array[idx1 * 2 + 100];
    result += global_array[idx2 * 3 + 200];
    
    /* Complex expression in index */
    result += global_array[(idx1 * idx2) + (idx3 << 2) + 500];
    
    /* Multi-word type with potential alignment issues */
    long long ll1 = global_big_array[idx1];
    long long ll2 = global_big_array[idx2 + 100];
    long long ll3 = global_big_array[idx3 + 200];
    
    /* Double precision floating point */
    double d1 = global_double_array[idx1];
    double d2 = global_double_array[idx2];
    double d3 = global_double_array[idx3];
    
    /* Use all values */
    result += (int)ll1 + (int)ll2 + (int)ll3;
    result += (int)d1 + (int)d2 + (int)d3;
    
    /* More complex: nested array access */
    result += global_array[global_array[idx1] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 2;
    int b = y * 3;
    int c = a + b;
    int d = a - b;
    int e = a ^ b;
    int f = a | b;
    int g = a & b;
    
    /* Clobber many registers - forces spills and reloads */
    asm volatile (
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
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    /* Use all variables after clobber - they must be reloaded */
    volatile int result = a + b + c + d + e + f + g;
    
    /* Another assembly with input/output operands */
    int input = x + y;
    int output;
    asm volatile (
        "movl %1, %%eax\n"
        "addl $100, %%eax\n"
        "movl %%eax, %0\n"
        : "=r" (output)
        : "r" (input)
        : "%eax"
    );
    
    return result + output;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(volatile int base)
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
    double f1 = base * 1.5;
    double f2 = base * 2.5;
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many args - forces register pressure for argument passing */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, p1, p2);
    
    /* More computations after call to keep values live */
    int b1 = result + a1;
    int b2 = result + a2;
    int b3 = result + a3;
    int b4 = result + a4;
    int b5 = result + a5;
    
    return b1 + b2 + b3 + b4 + b5;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(volatile int x, volatile long long y)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = x + 1;
    register int r2 asm("r11") = x + 2;
    
    /* Mixed size operations */
    long long ll1 = y + 1000LL;
    long long ll2 = y + 2000LL;
    long long ll3 = y + 3000LL;
    
    /* Double precision operations */
    double d1 = (double)x * 1.234;
    double d2 = (double)x * 2.345;
    double d3 = (double)x * 3.456;
    
    /* Type conversions that may need reloads */
    int i1 = (int)ll1;
    int i2 = (int)ll2;
    int i3 = (int)d1;
    int i4 = (int)d2;
    
    /* Use all variables */
    volatile int result = r1 + r2 + i1 + i2 + i3 + i4;
    
    /* Structure with mixed types */
    struct mixed {
        int a;
        long long b;
        double c;
    } m;
    
    m.a = x;
    m.b = y;
    m.c = d1;
    
    result += m.a + (int)m.b + (int)m.c;
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    int result = 0;
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed, seed+1, seed+2, seed+3);
    result += test_complex_addressing(seed%100, (seed+1)%100, (seed+2)%100, (seed+3)%100);
    result += test_asm_clobber(seed, seed+10);
    result += test_many_args(seed);
    result += test_mixed_types(seed, seed * 100LL);
    
    /* Access global with large offset */
    result += global_array[5000];
    
    printf("Result: %d\n", result);
    
    return 0;
}
