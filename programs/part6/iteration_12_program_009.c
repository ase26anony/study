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
    int a, int b, int c, int d, int e,
    int f, int g, int h, int i, int j,
    double k, double l, void *m, void *n, void *o)
{
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    result += (int)k + (int)l;
    result += (int)(intptr_t)m + (int)(intptr_t)n + (int)(intptr_t)o;
    return result;
}

/* Function to create extreme register pressure */
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
    
    /* Many independent computations to create many live values */
    int t1 = v1 + v2;
    int t2 = v3 + v4;
    int t3 = v5 + v6;
    int t4 = v7 + v8;
    int t5 = v1 * v2;
    int t6 = v3 * v4;
    int t7 = v5 * v6;
    int t8 = v7 * v8;
    int t9 = t1 + t2;
    int t10 = t3 + t4;
    int t11 = t5 + t6;
    int t12 = t7 + t8;
    int t13 = t9 - t10;
    int t14 = t11 - t12;
    int t15 = t13 * t14;
    int t16 = t1 * t3;
    int t17 = t2 * t4;
    int t18 = t5 * t7;
    int t19 = t6 * t8;
    int t20 = t16 + t17;
    int t21 = t18 + t19;
    int t22 = t20 - t21;
    int t23 = t15 + t22;
    int t24 = t9 * t11;
    int t25 = t10 * t12;
    int t26 = t24 - t25;
    int t27 = t13 + t14;
    int t28 = t26 * t27;
    int t29 = t23 + t28;
    
    /* Force all values to be used */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 +
                         t9 + t10 + t11 + t12 + t13 + t14 + t15 +
                         t16 + t17 + t18 + t19 + t20 + t21 + t22 +
                         t23 + t24 + t25 + t26 + t27 + t28 + t29;
    
    return result;
}

/* Function with complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Complex index calculation */
    int idx1 = index * 3 + 7;
    int idx2 = index * 5 - 11;
    
    /* Double register indirect-like access */
    result += global_array[idx1 + idx2];
    
    /* Misaligned 64-bit access */
    long long ll_val = global_big_array[index];
    result += (int)ll_val;
    
    /* Double with potential alignment issues */
    double d_val = global_double_array[index % 1000];
    result += (int)d_val;
    
    /* Multi-word move simulation */
    struct two_words {
        long long a;
        long long b;
    } __attribute__((packed));
    
    volatile struct two_words tw;
    tw.a = ll_val;
    tw.b = ll_val * 2;
    result += (int)(tw.a + tw.b);
    
    return result;
}

/* Function with inline assembly clobbering registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int x = a * b;
    int y = c * d;
    int z = x + y;
    
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
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computations after clobber */
    int x2 = x * 2;
    int y2 = y * 3;
    int z2 = z * 4;
    
    return x2 + y2 + z2;
}

/* Function using explicit register variables */
int __attribute__((noinline)) test_explicit_registers(void)
{
    /* Try to allocate specific registers */
    register int r10_val asm("r10") = 100;
    register int r11_val asm("r11") = 200;
    register int r12_val asm("r12") = 300;
    register int r13_val asm("r13") = 400;
    
    /* Force use of these registers in computation */
    int result;
    asm volatile(
        "add %1, %0\n"
        "add %2, %0\n"
        "add %3, %0\n"
        : "=r"(result)
        : "r"(r10_val), "r"(r11_val), "r"(r12_val), "0"(r13_val)
        : "cc"
    );
    
    return result;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    volatile int seed = argc;
    int total = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int arg1 = (argv[0] != 0) ? 1 : 0;
    int arg2 = (argc > 1) ? atoi(argv[1]) : 42;
    int arg3 = (argc > 2) ? atoi(argv[2]) : 123;
    int arg4 = (argc > 3) ? atoi(argv[3]) : 456;
    
    /* Test 1: Register pressure */
    total += test_register_pressure(
        arg1 + 1, arg1 + 2, arg1 + 3, arg1 + 4,
        arg1 + 5, arg1 + 6, arg1 + 7, arg1 + 8
    );
    
    /* Test 2: Complex addressing */
    total += test_complex_addressing(arg2);
    
    /* Test 3: Many function arguments */
    total += many_args_function(
        arg1, arg2, arg3, arg4, arg1 + arg2,
        arg2 + arg3, arg3 + arg4, arg4 + arg1,
        arg1 * 2, arg2 * 3,
        3.14159, 2.71828,
        (void*)(intptr_t)arg1,
        (void*)(intptr_t)arg2,
        (void*)(intptr_t)arg3
    );
    
    /* Test 4: Assembly clobber */
    total += test_asm_clobber(arg1, arg2, arg3, arg4);
    
    /* Test 5: Explicit registers */
    total += test_explicit_registers();
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
