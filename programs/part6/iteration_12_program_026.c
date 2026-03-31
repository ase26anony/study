/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2)
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
    
    /* Create many independent computations to force register allocation */
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
    int t16 = t9 ^ t10;
    int t17 = t11 + t12;
    int t18 = t13 - t14;
    int t19 = t15 * t16;
    int t20 = t17 ^ t18;
    
    /* Force all values to be live simultaneously */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                      int index3, int index4)
{
    /* Force complex address calculations */
    volatile int complex_index1 = index1 * 2 + 3;
    volatile int complex_index2 = index2 * 4 - 5;
    volatile int complex_index3 = index3 * 3 + 7;
    volatile int complex_index4 = index4 * 5 - 11;
    
    /* Large immediate offsets forcing reloads */
    int val1 = global_array[4096 + complex_index1];  /* Large offset */
    int val2 = global_array[8192 + complex_index2];  /* Another large offset */
    
    /* Double register indirect with complex computation */
    int val3 = global_array[complex_index1 + complex_index2 * 2];
    int val4 = global_array[complex_index3 * 3 + complex_index4];
    
    /* Multi-word types requiring multiple registers */
    long long ll1 = global_big_array[complex_index1];
    long long ll2 = global_big_array[complex_index2];
    long long ll3 = global_big_array[complex_index3];
    long long ll4 = global_big_array[complex_index4];
    
    /* Double precision requiring specific register classes */
    double d1 = global_double_array[complex_index1];
    double d2 = global_double_array[complex_index2];
    double d3 = global_double_array[complex_index3];
    double d4 = global_double_array[complex_index4];
    
    /* Combine results forcing register pressure */
    return val1 + val2 + val3 + val4 + 
           (int)(ll1 >> 32) + (int)(ll2 >> 32) +
           (int)(ll3 >> 32) + (int)(ll4 >> 32) +
           (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    volatile int w = d;
    
    int result1, result2, result3, result4;
    
    /* Computation before assembly */
    int t1 = x * y;
    int t2 = z + w;
    int t3 = x - z;
    int t4 = y * w;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber commonly used registers */
    asm volatile(
        "# Force register spills\n\t"
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        "mov %2, %%ecx\n\t"
        "mov %3, %%edx\n\t"
        "add %%ebx, %%eax\n\t"
        "sub %%ecx, %%edx\n\t"
        "imul %%edx, %%eax\n\t"
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4)
        : "0"(t1), "1"(t2), "2"(t3), "3"(t4)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More computations after assembly to force reloads */
    int t5 = result1 + result2;
    int t6 = result3 * result4;
    int t7 = t5 - t6;
    int t8 = result1 ^ result2;
    int t9 = result3 + result4;
    int t10 = t7 * t8;
    
    return t5 + t6 + t7 + t8 + t9 + t10;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many values to pass as arguments */
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
    void *p1 = (void*)&global_array[0];
    void *p2 = (void*)&global_big_array[0];
    
    /* Call function with many arguments - forces register pressure */
    int result1 = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                 f1, f2, p1, p2);
    
    /* Call again with different values to prevent optimization */
    int result2 = many_args_func(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                 f2, f1, p2, p1);
    
    /* More computations to keep values live */
    int t1 = result1 * a1;
    int t2 = result2 + a2;
    int t3 = t1 - t2;
    int t4 = result1 ^ result2;
    int t5 = a3 * a4;
    int t6 = a5 + a6;
    
    return result1 + result2 + t1 + t2 + t3 + t4 + t5 + t6;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, int c, int d)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = a;
    register int r2 asm("r11") = b;
    register long long r3 asm("rax") = (long long)c * d;
    register double r4 asm("xmm0") = (double)a / (b + 1);
    
    /* Force use of all register variables */
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile long long v3 = r3;
    volatile double v4 = r4;
    
    /* Mixed type computations */
    int i1 = v1 + (int)v4;
    long long ll1 = v3 + (long long)v2;
    double d1 = v4 * 2.0;
    int i2 = (int)ll1;
    double d2 = d1 + (double)i1;
    long long ll2 = (long long)i2 * 3;
    
    /* Access with complex addressing */
    int arr_val1 = global_array[v1 + 2048];  /* Large offset */
    int arr_val2 = global_array[v2 + 4096];  /* Another large offset */
    
    return i1 + i2 + (int)ll1 + (int)ll2 + (int)d1 + (int)d2 + arr_val1 + arr_val2;
}

/* Main function orchestrating all tests */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 5LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 2.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3,
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(base, base+8, base+16, base+24);
    
    total += test_asm_clobber(base+32, base+40, base+48, base+56);
    
    total += test_many_args(base+64);
    
    total += test_mixed_types(base+128, base+256, base+512, base+1024);
    
    /* Print result to ensure all code is live */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
