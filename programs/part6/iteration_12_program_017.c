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
    double f1, double f2, void *p1, void *p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
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
    int t5 = t1 * t2;
    int t6 = t3 | t4;
    int t7 = t5 + t6;
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
    int t19 = t15 + t18;
    int t20 = (v1 << 2) + (v2 << 3);
    int t21 = (v3 >> 1) + (v4 >> 2);
    int t22 = t20 * t21;
    int t23 = t19 + t22;
    
    /* Force all temporaries to be live simultaneously */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), 
                      "r"(t5), "r"(t6), "r"(t7), "r"(t8),
                      "r"(t9), "r"(t10), "r"(t11), "r"(t12),
                      "r"(t13), "r"(t14), "r"(t15), "r"(t16),
                      "r"(t17), "r"(t18), "r"(t19), "r"(t20),
                      "r"(t21), "r"(t22), "r"(t23));
    
    return t23;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                     int index3, int index4)
{
    volatile int idx1 = index1;
    volatile int idx2 = index2;
    volatile int idx3 = index3;
    volatile int idx4 = index4;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096 + idx1];
    
    /* Complex index calculation - may need multiple registers */
    int val2 = global_array[(idx1 * idx2) + (idx3 << 2) + idx4];
    
    /* Double register indirect style access */
    int val3 = global_array[global_array[idx1] + idx2];
    
    /* Multi-word type with potential alignment issues */
    long long big_val = global_big_array[idx1];
    long long big_val2 = global_big_array[idx2 + 256]; /* Large offset */
    
    /* Double precision with potential reload needs */
    double dbl_val = global_double_array[idx3];
    double dbl_val2 = global_double_array[idx4 + 128];
    
    /* Mix computations to keep values live */
    int result = val1 + val2 + val3 + (int)big_val + 
                 (int)(big_val2 >> 32) + (int)dbl_val + (int)dbl_val2;
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    volatile int w = d;
    
    /* Do some computation */
    int r1 = x * y;
    int r2 = z * w;
    int r3 = r1 + r2;
    
    /* Clobber many registers - forces spills and reloads */
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
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use the values after clobber - will need reloads */
    int r4 = r3 * x;
    int r5 = r4 + y;
    int r6 = r5 * z;
    
    return r6;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int v = base;
    
    /* Create many argument values with complex computations */
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
    
    /* Call function with many args - creates register pressure */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, p1, p2);
    
    /* More computations after call */
    int r1 = result + a1;
    int r2 = r1 * a2;
    int r3 = r2 + a3;
    
    return r3;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, int c)
{
    /* Try to use explicit register variables */
    register int r10_var asm("r10") = a * 2;
    register int r11_var asm("r11") = b * 3;
    
    /* Mixed size types */
    short s1 = a;
    short s2 = b;
    char c1 = c;
    char c2 = a + b;
    
    /* 64-bit operations on 32-bit arch would need special handling */
    long long ll1 = (long long)a * b;
    long long ll2 = (long long)c * 1000;
    
    /* Floating point mixed with integer */
    double d1 = a * 1.234;
    float f1 = b * 2.345f;
    
    /* Complex expression mixing all types */
    int result = (int)(r10_var + r11_var + s1 + s2 + c1 + c2 + 
                      (int)(ll1 >> 32) + (int)d1 + (int)f1);
    
    /* Force use of register variables */
    asm volatile("" : : "r"(r10_var), "r"(r11_var));
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 1000LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 3.14159;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3,
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(base % 100, (base+1) % 100,
                                    (base+2) % 100, (base+3) % 100);
    
    total += test_asm_clobber(base, base+10, base+20, base+30);
    
    total += test_many_args(base);
    
    total += test_mixed_types(base, base+5, base+10);
    
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
