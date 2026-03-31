/* reload_test.c - Test program to trigger GCC's reload pass initialization */
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
    double f1, double f2, void* p1, void* p2)
{
    /* Force register pressure in callee too */
    volatile int t1 = a1 + a2;
    volatile int t2 = a3 + a4;
    volatile int t3 = a5 + a6;
    volatile int t4 = a7 + a8;
    volatile int t5 = a9 + a10;
    
    /* Complex addressing in callee */
    global_array[(int)f1 + t1 + 4096] = t2;
    
    return t1 + t2 + t3 + t4 + t5 + (int)f1 + (int)f2;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                    int e, int f, int g, int h)
{
    /* Use volatile to prevent optimization */
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
    int t2 = v3 + v4;
    int t3 = v5 + v6;
    int t4 = v7 + v8;
    int t5 = v1 * v3;
    int t6 = v2 * v4;
    int t7 = v5 * v7;
    int t8 = v6 * v8;
    int t9 = t1 + t2;
    int t10 = t3 + t4;
    int t11 = t5 + t6;
    int t12 = t7 + t8;
    int t13 = t9 * t10;
    int t14 = t11 * t12;
    int t15 = t13 + t14;
    int t16 = t1 * t3;
    int t17 = t2 * t4;
    int t18 = t5 * t7;
    int t19 = t6 * t8;
    int t20 = t16 + t17;
    int t21 = t18 + t19;
    int t22 = t20 * t21;
    int t23 = t15 + t22;
    
    /* More computations to increase pressure */
    int t24 = t23 * a;
    int t25 = t24 + b;
    int t26 = t25 * c;
    int t27 = t26 + d;
    int t28 = t27 * e;
    int t29 = t28 + f;
    int t30 = t29 * g;
    int t31 = t30 + h;
    
    /* Use all temporaries in final computation */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + 
           t9 + t10 + t11 + t12 + t13 + t14 + t15 +
           t16 + t17 + t18 + t19 + t20 + t21 + t22 +
           t23 + t24 + t25 + t26 + t27 + t28 + t29 +
           t30 + t31;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index, int offset)
{
    volatile int idx = index;
    volatile int off = offset;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Variable index with large offset */
    int val3 = global_array[idx + 2048];
    
    /* Complex expression in index */
    int val4 = global_array[(idx * off) + 1024];
    
    /* Multi-word access forcing piecewise moves */
    long long big_val = global_big_array[idx];
    
    /* Double indirection simulation */
    int temp = idx + off;
    int val5 = global_array[global_array[temp % 100] % 1000];
    
    /* Mixed types causing alignment issues */
    double dval = global_double_array[idx];
    
    /* Use all values to keep them live */
    return val1 + val2 + val3 + val4 + (int)big_val + val5 + (int)dval;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int result;
    
    /* Do some computation that uses registers */
    int t1 = a * b;
    int t2 = c * d;
    int t3 = t1 + t2;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber general purpose registers */
    asm volatile (
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
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computation after clobber - forces reloads */
    int t4 = t3 * a;
    int t5 = t4 + b;
    int t6 = t5 * c;
    result = t6 + d;
    
    /* Another assembly block */
    asm volatile (
        "# Another clobber\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    return result + t1 + t2 + t3 + t4 + t5 + t6;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
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
    
    /* Create register pressure before call */
    int p1 = v1 * v2;
    int p2 = v3 * v4;
    int p3 = v5 * v6;
    int p4 = v7 * v8;
    int p5 = v9 * v10;
    
    /* Call function with many arguments - forces register allocation
       for argument passing, potentially causing reloads */
    int result = many_args_function(
        p1, p2, p3, p4, p5,
        v1, v2, v3, v4, v5,
        (double)v6, (double)v7,
        (void*)&v8, (void*)&v9
    );
    
    /* More computations after call */
    int q1 = result * p1;
    int q2 = q1 + p2;
    int q3 = q2 * p3;
    int q4 = q3 + p4;
    
    return q4 + p5;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = a * 2;
    register int r2 asm("r11") = b * 3;
    
    /* Mix integer and floating point computations */
    double d1 = (double)a / 3.14159;
    double d2 = (double)b / 2.71828;
    
    /* Long long operations that might need multiple registers */
    long long ll1 = (long long)a * 1000000LL;
    long long ll2 = (long long)b * 2000000LL;
    long long ll3 = ll1 + ll2;
    
    /* Complex addressing with mixed types */
    global_double_array[a % 100] = d1 + d2;
    global_big_array[b % 100] = ll3;
    
    /* Use explicit register variables in computation */
    int t1 = r1 + r2;
    double t2 = d1 * d2;
    long long t3 = ll3 / 1000LL;
    
    /* Force all values to be used */
    return t1 + (int)t2 + (int)t3 + (int)d1 + (int)d2;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Starting reload pass tests...\n");
    
    /* Run each test with inputs derived from volatile/argv */
    volatile int seed = base;
    
    result += test_register_pressure(
        seed + 1, seed + 2, seed + 3, seed + 4,
        seed + 5, seed + 6, seed + 7, seed + 8
    );
    
    result += test_complex_addressing(seed % 100, seed % 50);
    
    result += test_asm_clobber(seed + 10, seed + 20, seed + 30, seed + 40);
    
    result += test_many_args(seed + 100);
    
    result += test_mixed_types(seed + 50, seed + 60);
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Access global arrays to keep them live */
    global_array[0] = result;
    global_big_array[0] = result;
    global_double_array[0] = result;
    
    return result != 0 ? 0 : 1;
}
