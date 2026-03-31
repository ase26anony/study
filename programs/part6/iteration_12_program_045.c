/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789;
volatile double g_volatile_double = 3.14159;
volatile int* g_volatile_ptr = NULL;

/* Large global array with large offsets */
int global_array[10000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Force use of all arguments */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    volatile int psum = (int)((intptr_t)p1 + (intptr_t)p2);
    
    return sum + (int)fsum + psum;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, int e, int f)
{
    /* Use volatile inputs to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    
    /* Create many independent live variables */
    int t1 = v1 + v2;
    int t2 = v3 * v4;
    int t3 = v5 ^ v6;
    int t4 = v1 - v2;
    int t5 = v3 / (v4 ? v4 : 1);
    int t6 = v5 | v6;
    int t7 = v1 & v2;
    int t8 = v3 << v4;
    int t9 = v5 >> (v6 & 31);
    int t10 = t1 + t2;
    int t11 = t3 - t4;
    int t12 = t5 * t6;
    int t13 = t7 ^ t8;
    int t14 = t9 & t10;
    int t15 = t11 | t12;
    int t16 = t13 + t14;
    int t17 = t15 - t16;
    int t18 = t1 * t17;
    int t19 = t2 + t18;
    int t20 = t3 - t19;
    int t21 = t4 * t20;
    int t22 = t5 + t21;
    int t23 = t6 - t22;
    int t24 = t7 * t23;
    int t25 = t8 + t24;
    int t26 = t9 - t25;
    int t27 = t10 * t26;
    int t28 = t11 + t27;
    int t29 = t12 - t28;
    int t30 = t13 * t29;
    
    /* Force all variables to be used */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                         t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
    
    return result;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int* base, int index1, int index2)
{
    /* Force complex address calculations */
    volatile int idx1 = index1;
    volatile int idx2 = index2;
    
    /* Large immediate offset */
    int val1 = global_array[4096];  /* Large immediate offset */
    int val2 = global_array[idx1 * 2 + 1000];  /* Complex index with large offset */
    
    /* Double register indirect with computation */
    int val3 = base[idx1 + idx2 * 4];
    
    /* Multi-level computation in address */
    int val4 = global_array[(idx1 * idx2) % 1000 + 2000];
    
    /* Force use of all results */
    return val1 + val2 + val3 + val4;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int x1 = a * b;
    int x2 = c * d;
    int x3 = x1 + x2;
    int x4 = x1 - x2;
    int x5 = x3 * x4;
    int x6 = x3 / (x4 ? x4 : 1);
    
    /* Clobber many registers - force spills */
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
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* More computations after clobber */
    int x7 = x5 + x6;
    int x8 = x5 - x6;
    int x9 = x7 * x8;
    int x10 = x7 / (x8 ? x8 : 1);
    
    return x9 + x10;
}

/* Test 4: Function with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(void)
{
    /* Create many argument values with complex computations */
    int a1 = g_volatile_int + 1;
    int a2 = g_volatile_int * 2;
    int a3 = g_volatile_int / 3;
    int a4 = g_volatile_int - 4;
    int a5 = g_volatile_int ^ 5;
    int a6 = g_volatile_int & 6;
    int a7 = g_volatile_int | 7;
    int a8 = g_volatile_int << 2;
    int a9 = g_volatile_int >> 1;
    int a10 = ~g_volatile_int;
    
    double f1 = g_volatile_double * 1.1;
    double f2 = g_volatile_double / 2.2;
    double f3 = g_volatile_double + 3.3;
    
    void* p1 = (void*)((intptr_t)&global_array[0] + 512);
    void* p2 = (void*)((intptr_t)&global_array[0] + 1024);
    
    /* Call function with many arguments - will need to shuffle into registers */
    return many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                         f1, f2, f3, p1, p2);
}

/* Test 5: Mixed types and alignment issues */
long long __attribute__((noinline)) test_mixed_types(int a, double b, long long c)
{
    /* Force conversions between different types/sizes */
    volatile int vi = a;
    volatile double vd = b;
    volatile long long vll = c;
    
    /* Mixed type computations */
    double d1 = vi + vd;          /* int to double conversion */
    long long ll1 = (long long)vi * vll;  /* int to long long */
    int i1 = (int)(vd * 100.0);   /* double to int */
    double d2 = (double)vll / 2.0; /* long long to double */
    
    /* Unaligned access simulation */
    char buffer[64];
    long long* unaligned_ptr = (long long*)(buffer + 3);  /* Misaligned pointer */
    *unaligned_ptr = vll;  /* May require multiple moves */
    
    /* Force use of all results */
    return (long long)(d1 + d2) + ll1 + i1 + *unaligned_ptr;
}

/* Test 6: Explicit register variables */
int __attribute__((noinline)) test_explicit_registers(void)
{
    /* Try to use specific registers (x86-64 example) */
    register int r10_val asm("r10") = g_volatile_int * 2;
    register int r11_val asm("r11") = g_volatile_int * 3;
    
    /* Force computations using these registers */
    asm volatile (
        "add %1, %0\n"
        : "+r" (r10_val)
        : "r" (r11_val)
        : "cc"
    );
    
    /* More register pressure */
    int x1 = r10_val + 1;
    int x2 = r11_val - 1;
    int x3 = x1 * x2;
    int x4 = x1 / (x2 ? x2 : 1);
    int x5 = x3 ^ x4;
    int x6 = x3 & x4;
    int x7 = x5 | x6;
    int x8 = x7 << 2;
    int x9 = x8 >> 1;
    int x10 = ~x9;
    
    return r10_val + r11_val + x10;
}

int main(int argc, char* argv[])
{
    /* Initialize global array */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    srand(seed);
    
    int result = 0;
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(rand(), rand(), rand(), rand(), rand(), rand());
    
    int* dynamic_array = malloc(1000 * sizeof(int));
    for (int i = 0; i < 1000; i++) {
        dynamic_array[i] = i * 5;
    }
    result += test_complex_addressing(dynamic_array, rand() % 100, rand() % 100);
    free(dynamic_array);
    
    result += test_asm_clobber(rand(), rand(), rand(), rand());
    result += test_many_args();
    result += (int)test_mixed_types(rand(), (double)rand() / 100.0, rand() * 100LL);
    result += test_explicit_registers();
    
    /* Print result to ensure all code is live */
    printf("Final result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
