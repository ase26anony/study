/* reload_test.c - Test program to trigger GCC's reload pass initialization */

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
    double f1, double f2, void* p1, void* p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2 + (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                    int e, int f, int g, int h)
{
    /* Force many independent computations to create register pressure */
    volatile int t1 = a + b;
    volatile int t2 = c + d;
    volatile int t3 = e + f;
    volatile int t4 = g + h;
    volatile int t5 = t1 * t2;
    volatile int t6 = t3 * t4;
    volatile int t7 = t5 + t6;
    volatile int t8 = t1 - t2;
    volatile int t9 = t3 - t4;
    volatile int t10 = t8 * t9;
    volatile int t11 = t7 / (t10 ? t10 : 1);
    volatile int t12 = t1 ^ t2;
    volatile int t13 = t3 ^ t4;
    volatile int t14 = t12 | t13;
    volatile int t15 = t12 & t13;
    volatile int t16 = t14 - t15;
    volatile int t17 = t11 * t16;
    volatile int t18 = t5 << 2;
    volatile int t19 = t6 >> 1;
    volatile int t20 = t18 | t19;
    volatile int t21 = t17 + t20;
    volatile int t22 = t8 + t9;
    volatile int t23 = t10 * t22;
    volatile int t24 = t21 - t23;
    volatile int t25 = t12 + t13;
    volatile int t26 = t14 * t15;
    volatile int t27 = t16 / (t25 ? t25 : 1);
    volatile int t28 = t24 + t26 + t27;
    
    return t28;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int idx1, int idx2, int idx3)
{
    volatile int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    result += global_array[idx1 * 2 + idx2];
    
    /* Complex index computation */
    int complex_idx = (idx1 * idx2) + (idx3 << 3);
    result += global_array[complex_idx];
    
    /* Multi-word type with potential alignment issues */
    volatile long long ll_val = global_big_array[idx1];
    result += (int)ll_val;
    
    /* Double type access - may require multiple registers */
    volatile double d_val = global_double_array[idx2];
    result += (int)d_val;
    
    /* Nested array access with variable offset */
    result += global_array[global_array[idx3] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int x, int y, int z)
{
    volatile int a = x * y;
    volatile int b = y * z;
    volatile int c = z * x;
    
    /* Clobber many registers to force spills */
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
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    volatile int d = a + b + c;
    
    /* More computations after clobber */
    volatile int e = d * x;
    volatile int f = d * y;
    volatile int g = d * z;
    
    __asm__ volatile (
        "# Clobber more registers\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    return e + f + g;
}

/* Test 4: Function calls with many arguments */
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
    
    volatile double f1 = (double)v1 * 1.1;
    volatile double f2 = (double)v2 * 2.2;
    
    volatile void* p1 = (void*)(intptr_t)v3;
    volatile void* p2 = (void*)(intptr_t)v4;
    
    /* Call function with many arguments - forces register allocation pressure */
    int result = many_args_function(
        v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
        f1, f2, p1, p2
    );
    
    /* More computations to keep values live across call */
    volatile int r1 = result + v1;
    volatile int r2 = result + v2;
    volatile int r3 = result + v3;
    
    return r1 + r2 + r3;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, double c, double d)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = a * 2;
    register int r2 asm("r11") = b * 3;
    
    volatile double d1 = c * 2.0;
    volatile double d2 = d * 3.0;
    
    /* Force interaction between register variables and stack values */
    volatile int t1 = r1 + (int)d1;
    volatile int t2 = r2 + (int)d2;
    
    /* Long long operations that may need multiple registers */
    volatile long long ll1 = (long long)r1 * 1000LL;
    volatile long long ll2 = (long long)r2 * 2000LL;
    
    /* Complex expression mixing everything */
    volatile int result = t1 + t2 + (int)(ll1 >> 32) + (int)(ll2 >> 32);
    
    /* More register pressure */
    volatile int x1 = result * a;
    volatile int x2 = result * b;
    volatile int x3 = x1 + x2;
    volatile int x4 = x3 * (int)c;
    volatile int x5 = x4 * (int)d;
    
    return x5;
}

int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    volatile int seed = argc;
    
    /* Initialize some values from argv to make them truly variable */
    int arg1 = argv[0] ? (int)(intptr_t)argv[0] & 0xFF : 1;
    int arg2 = argc > 1 ? atoi(argv[1]) : 42;
    int arg3 = argc > 2 ? atoi(argv[2]) : 123;
    int arg4 = argc > 3 ? atoi(argv[3]) : 456;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 3LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(arg1, arg2, arg3, arg4, 
                                   seed, seed+1, seed+2, seed+3);
    
    total += test_complex_addressing(arg1, arg2, arg3);
    
    total += test_asm_clobber(arg1, arg2, arg3);
    
    total += test_many_args(seed);
    
    total += test_mixed_types(arg1, arg2, (double)arg3, (double)arg4);
    
    /* Additional stress: nested function calls with register pressure */
    for (int i = 0; i < 5; i++) {
        total += test_register_pressure(total, i, arg1, arg2, arg3, arg4, seed, i*2);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
