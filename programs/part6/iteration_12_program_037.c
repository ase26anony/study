/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789L;
volatile double g_volatile_double = 3.14159;
volatile int* g_volatile_ptr = NULL;

/* Large global array with offset that may require reload */
int global_array[10000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) 
dummy_function_with_many_args(int a1, int a2, int a3, int a4, int a5,
                              int a6, int a7, int a8, int a9, int a10,
                              double f1, double f2, double f3, void* p1, void* p2) {
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + 
           (int)f1 + (int)f2 + (int)f3 + (int)(long)p1 + (int)(long)p2;
}

/* Test 1: Extreme register pressure with many live scalar variables */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                     int e, int f, int g, int h) {
    /* Force many independent computations to create register pressure */
    int t1 = a + b + g_volatile_int;
    int t2 = c + d + g_volatile_int;
    int t3 = e + f + g_volatile_int;
    int t4 = g + h + g_volatile_int;
    int t5 = a * b - g_volatile_int;
    int t6 = c * d - g_volatile_int;
    int t7 = e * f - g_volatile_int;
    int t8 = g * h - g_volatile_int;
    int t9 = (a << 2) | (b & 0xFF);
    int t10 = (c << 2) | (d & 0xFF);
    int t11 = (e << 2) | (f & 0xFF);
    int t12 = (g << 2) | (h & 0xFF);
    int t13 = t1 ^ t2 ^ t3;
    int t14 = t4 ^ t5 ^ t6;
    int t15 = t7 ^ t8 ^ t9;
    int t16 = t10 ^ t11 ^ t12;
    int t17 = t13 * t14 + g_volatile_int;
    int t18 = t15 * t16 + g_volatile_int;
    int t19 = t17 / (t18 ? t18 : 1);
    int t20 = t19 * t13 - t14;
    
    /* More computations to exceed any reasonable register count */
    int t21 = t20 + (a & b) | (c ^ d);
    int t22 = t21 * (e | f) & (g ^ h);
    int t23 = t22 << (a % 8);
    int t24 = t23 >> (b % 8);
    int t25 = t24 * t1 / (t2 ? t2 : 1);
    int t26 = t25 + t3 - t4;
    int t27 = t26 * t5 % (t6 ? t6 : 1);
    int t28 = t27 | t7 & t8;
    int t29 = t28 ^ t9 | t10;
    int t30 = t29 + t11 - t12;
    
    /* Use all temporaries to ensure they stay live */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int* base, int index1, 
                                                      int index2, int offset) {
    int result = 0;
    
    /* Double register indirect with complex computation */
    result += base[index1 + index2 * 2 + g_volatile_int];
    
    /* Large immediate offset that may not fit in addressing mode */
    result += global_array[4096 + offset];
    
    /* Complex address computation with multiple operations */
    result += base[(index1 * index2) / (offset ? offset : 1) + g_volatile_int % 256];
    
    /* Multi-word move with 64-bit values on 32-bit arch or misalignment */
    long long big_value = (long long)index1 * (long long)index2;
    result += (int)(big_value >> 32) + (int)big_value;
    
    /* Nested array access with variable indices */
    int* ptr = &base[index1];
    result += ptr[index2] + ptr[offset];
    
    /* Address computation with function call (spills all caller-saved regs) */
    result += base[abs(index1 - index2) + offset];
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d) {
    int result = a + b + c + d;
    
    /* Do some computation that uses registers */
    int t1 = a * b + g_volatile_int;
    int t2 = c * d + g_volatile_int;
    int t3 = t1 ^ t2;
    int t4 = (a << 4) | (b & 0xF);
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber general purpose registers */
    asm volatile(
        "# Dummy assembly to clobber registers\n"
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
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computation after clobber - forces reloads */
    int t5 = t3 + t4 + g_volatile_int;
    int t6 = t5 * a / (b ? b : 1);
    int t7 = t6 | c & d;
    
    /* Another assembly block clobbering different registers */
    asm volatile(
        "# More clobbering\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        : 
        : 
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    result += t1 + t2 + t3 + t4 + t5 + t6 + t7;
    return result;
}

/* Test 4: Function call with many arguments forcing register/stack moves */
int __attribute__((noinline)) test_many_args(int iter) {
    /* Create many different values to pass */
    int a1 = iter + 1;
    int a2 = iter + 2;
    int a3 = iter + 3;
    int a4 = iter + 4;
    int a5 = iter + 5;
    int a6 = iter + 6;
    int a7 = iter + 7;
    int a8 = iter + 8;
    int a9 = iter + 9;
    int a10 = iter + 10;
    double f1 = iter * 1.1;
    double f2 = iter * 2.2;
    double f3 = iter * 3.3;
    void* p1 = (void*)(long)(iter + 100);
    void* p2 = (void*)(long)(iter + 200);
    
    /* Call function with many args - forces register allocation pressure */
    int result = dummy_function_with_many_args(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                               f1, f2, f3, p1, p2);
    
    /* Do it multiple times in a loop with different values */
    for (int i = 0; i < 3; i++) {
        result += dummy_function_with_many_args(a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
                                                a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
                                                f1 + i, f2 + i, f3 + i, 
                                                (void*)(long)(iter + i), p2);
    }
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b) {
    /* Use explicit register variables to constrain register allocation */
    register int r1 asm("r10") = a + g_volatile_int;
    register int r2 asm("r11") = b + g_volatile_int;
    
    /* Mix different data types */
    double d1 = a * 1.5;
    double d2 = b * 2.5;
    long long ll1 = (long long)a * b;
    long long ll2 = (long long)r1 * r2;
    
    /* Force conversions and moves between different register classes */
    int i1 = (int)d1;
    int i2 = (int)d2;
    int i3 = (int)(ll1 >> 32);
    int i4 = (int)ll2;
    
    /* Complex expression mixing all values */
    int result = r1 + r2 + i1 + i2 + i3 + i4;
    
    /* Use volatile to prevent optimization */
    result += (int)g_volatile_double;
    
    return result;
}

/* Main function orchestrates all tests */
int main(int argc, char* argv[]) {
    /* Initialize global array */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 2;
    }
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 1234;
    g_volatile_ptr = &seed;
    
    int total = 0;
    
    /* Run test 1: Register pressure */
    int base1 = seed;
    total += test_register_pressure(base1, base1+1, base1+2, base1+3,
                                    base1+4, base1+5, base1+6, base1+7);
    
    /* Run test 2: Complex addressing */
    int* dynamic_array = (int*)malloc(1000 * sizeof(int));
    for (int i = 0; i < 1000; i++) {
        dynamic_array[i] = i * 3;
    }
    total += test_complex_addressing(dynamic_array, seed % 100, 
                                     (seed * 3) % 100, seed % 50);
    free(dynamic_array);
    
    /* Run test 3: Assembly clobber */
    total += test_asm_clobber(seed, seed+10, seed+20, seed+30);
    
    /* Run test 4: Many function arguments */
    total += test_many_args(seed % 100);
    
    /* Run test 5: Mixed types */
    total += test_mixed_types(seed, seed * 2);
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
