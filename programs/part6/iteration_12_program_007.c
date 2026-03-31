/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void* p1, void* p2)
{
    /* Force use of all arguments */
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
    int t8 = v6 - v8;
    int t9 = v1 ^ v5;
    int t10 = v2 * v6;
    int t11 = v3 + v7;
    int t12 = v4 - v8;
    int t13 = v1 * v8;
    int t14 = v2 ^ v7;
    int t15 = v3 - v6;
    int t16 = v4 + v5;
    int t17 = v1 + v7;
    int t18 = v2 * v8;
    int t19 = v3 ^ v5;
    int t20 = v4 - v6;
    int t21 = v1 - v4;
    int t22 = v2 + v3;
    int t23 = v5 ^ v8;
    int t24 = v6 * v7;
    
    /* Force all temporaries to be live simultaneously */
    volatile int sum = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                      t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                      t21 + t22 + t23 + t24;
    
    return sum;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with complex computation */
    int complex_idx = (index * 3 + 7) / 2;
    result += global_array[complex_idx + 1000];
    
    /* Multi-word types with potential alignment issues */
    volatile long long ll1 = global_big_array[index];
    volatile long long ll2 = global_big_array[index + 100];
    result += (int)(ll1 + ll2);
    
    /* Double with potential reload for 64-bit moves */
    volatile double d1 = global_double_array[index];
    volatile double d2 = global_double_array[index + 50];
    result += (int)(d1 + d2);
    
    /* Nested array access with variable offset */
    result += global_array[global_array[index] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with register clobbering */
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
    
    /* Use values after clobber - they must be reloaded */
    volatile int after_x = x * 2;
    volatile int after_y = y / 2;
    volatile int after_z = z + 1;
    
    return after_x + after_y + after_z;
}

/* Test 4: Function with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many different values for arguments */
    int i1 = base + 1;
    int i2 = base + 2;
    int i3 = base + 3;
    int i4 = base + 4;
    int i5 = base + 5;
    int i6 = base + 6;
    int i7 = base + 7;
    int i8 = base + 8;
    int i9 = base + 9;
    int i10 = base + 10;
    double d1 = base * 1.5;
    double d2 = base * 2.5;
    void* p1 = (void*)&global_array[0];
    void* p2 = (void*)&global_big_array[0];
    
    /* Call function with many args - forces register pressure for arg passing */
    int result = many_args_func(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
                               d1, d2, p1, p2);
    
    return result;
}

/* Test 5: Explicit register variables and special types */
int __attribute__((noinline)) test_special_registers(int a, int b, int c)
{
    /* Try to use specific registers (x86-64 example) */
    register int r10_val asm("r10") = a * b;
    register int r11_val asm("r11") = b * c;
    register int r12_val asm("r12") = c * a;
    
    /* Force use of these register variables in computation */
    volatile int sum = 0;
    
    /* Complex expression to keep all register variables live */
    sum += r10_val * 2;
    sum += r11_val / 3;
    sum += r12_val + 5;
    
    /* Additional computation to increase pressure */
    sum += (r10_val ^ r11_val) | r12_val;
    sum += (r10_val + r11_val) * r12_val;
    
    return sum;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    int index = argc > 2 ? atoi(argv[2]) : 100;
    
    volatile int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3,
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(index);
    
    total += test_asm_clobber(base, base+10, base+20, base+30);
    
    total += test_many_args(base);
    
    total += test_special_registers(base, base+5, base+10);
    
    /* Additional stress: nested calls with register pressure */
    for (int i = 0; i < 3; i++) {
        total += test_register_pressure(total, total+1, total+2, total+3,
                                       total+4, total+5, total+6, total+7);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
