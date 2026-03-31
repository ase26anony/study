/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[20000];
volatile double global_double_array[5000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Force use of all arguments */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2 + f3);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, int e, int f)
{
    /* Use volatile inputs to prevent optimization */
    volatile int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e, v6 = f;
    
    /* Create many independent live variables exceeding register count */
    int t1 = v1 + v2;
    int t2 = v3 + v4;
    int t3 = v5 + v6;
    int t4 = v1 * v2;
    int t5 = v3 * v4;
    int t6 = v5 * v6;
    int t7 = t1 + t2;
    int t8 = t3 + t4;
    int t9 = t5 + t6;
    int t10 = t7 * t8;
    int t11 = t8 * t9;
    int t12 = t9 * t7;
    int t13 = t10 + t11;
    int t14 = t11 + t12;
    int t15 = t12 + t10;
    int t16 = t13 * t14;
    int t17 = t14 * t15;
    int t18 = t15 * t13;
    int t19 = t16 + t17;
    int t20 = t17 + t18;
    int t21 = t18 + t16;
    int t22 = t19 * t20;
    int t23 = t20 * t21;
    int t24 = t21 * t19;
    int t25 = t22 + t23;
    int t26 = t23 + t24;
    int t27 = t24 + t22;
    
    /* Force all values to be used in final computation */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, int index3)
{
    volatile int idx1 = index1, idx2 = index2, idx3 = index3;
    
    /* Force double register indirect addressing */
    int val1 = global_array[idx1 + idx2 * 2 + 100];
    int val2 = global_array[idx2 + idx3 * 3 + 200];
    int val3 = global_array[idx3 + idx1 * 4 + 300];
    
    /* Large immediate offsets that may not fit in addressing mode */
    int val4 = global_array[4096 + idx1];
    int val5 = global_array[8192 + idx2];
    
    /* Misaligned 64-bit access on 32-bit boundary */
    long long big_val = global_big_array[idx1];
    big_val += global_big_array[idx1 + 1];  /* May require separate loads */
    
    /* Double precision with potential alignment issues */
    double d1 = global_double_array[idx2];
    double d2 = global_double_array[idx2 + 1];
    
    return val1 + val2 + val3 + val4 + val5 + (int)big_val + (int)(d1 + d2);
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    volatile int v1 = a, v2 = b, v3 = c, v4 = d;
    
    /* Do some computation creating live values */
    int x1 = v1 * v2 + 12345;
    int x2 = v2 * v3 + 23456;
    int x3 = v3 * v4 + 34567;
    int x4 = v4 * v1 + 45678;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber commonly used registers */
    asm volatile(
        "# Force register spilling\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use the values after assembly to force reloads */
    return x1 + x2 + x3 + x4;
}

/* Test 4: Function calls with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int v = base;
    
    /* Create many values that need to be passed */
    int arg1 = v + 1;
    int arg2 = v + 2;
    int arg3 = v + 3;
    int arg4 = v + 4;
    int arg5 = v + 5;
    int arg6 = v + 6;
    int arg7 = v + 7;
    int arg8 = v + 8;
    int arg9 = v + 9;
    int arg10 = v + 10;
    double farg1 = v * 1.1;
    double farg2 = v * 2.2;
    double farg3 = v * 3.3;
    void *parg1 = (void*)(intptr_t)(v + 100);
    void *parg2 = (void*)(intptr_t)(v + 200);
    
    /* Call function with many arguments - forces register/stack allocation */
    int result = many_args_function(
        arg1, arg2, arg3, arg4, arg5,
        arg6, arg7, arg8, arg9, arg10,
        farg1, farg2, farg3, parg1, parg2
    );
    
    /* Do more work after call to force reloads of preserved values */
    int more1 = arg1 * arg2;
    int more2 = arg3 * arg4;
    int more3 = (int)(farg1 * 10.0);
    
    return result + more1 + more2 + more3;
}

/* Test 5: Explicit register variables and unusual types */
int __attribute__((noinline)) test_explicit_registers(int a, int b)
{
    /* Try to use explicit registers (GCC extension) */
    register int r1 asm("r10") = a * 2;
    register int r2 asm("r11") = b * 3;
    
    /* Use 80-bit long double which may use x87 stack on x86 */
    volatile long double ld1 = a * 1.23456789L;
    volatile long double ld2 = b * 9.87654321L;
    long double ld3 = ld1 + ld2;
    long double ld4 = ld1 * ld2;
    
    /* Vector-like operations using arrays */
    int arr[8] = {a, b, a+1, b+1, a+2, b+2, a+3, b+3};
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i] * (i + 1);
    }
    
    return r1 + r2 + (int)ld3 + (int)ld4 + sum;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
        if (i < 20000) global_big_array[i] = i * 5LL;
        if (i < 5000) global_double_array[i] = i * 2.5;
    }
    
    int total = 0;
    
    /* Run each test with different inputs to create varied reload patterns */
    total += test_register_pressure(base, base+1, base+2, base+3, base+4, base+5);
    total += test_complex_addressing(base % 100, (base+10) % 100, (base+20) % 100);
    total += test_asm_clobber(base, base+6, base+12, base+18);
    total += test_many_args(base + 100);
    total += test_explicit_registers(base + 50, base + 60);
    
    printf("Total result: %d\n", total);
    return total > 0 ? 0 : 1;
}
