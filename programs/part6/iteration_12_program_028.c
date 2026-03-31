/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation to prevent optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 + f2 - f3);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                    volatile int c, volatile int d)
{
    /* Force many independent computations that must stay live */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = a * b;
    int t4 = c ^ d;
    int t5 = a | b;
    int t6 = c & d;
    int t7 = t1 + t2;
    int t8 = t3 - t4;
    int t9 = t5 ^ t6;
    int t10 = t7 * t8;
    int t11 = t9 + t10;
    int t12 = t1 & t2;
    int t13 = t3 | t4;
    int t14 = t5 - t6;
    int t15 = t7 ^ t8;
    int t16 = t9 * t10;
    int t17 = t11 + t12;
    int t18 = t13 - t14;
    int t19 = t15 ^ t16;
    int t20 = t17 * t18;
    
    /* Use all temporaries in a complex expression */
    int result = t1 + t2 - t3 * t4 + t5 / (t6 ? t6 : 1) +
                 t7 ^ t8 | t9 & t10 +
                 t11 - t12 * t13 + t14 / (t15 ? t15 : 1) +
                 t16 ^ t17 | t18 & t19 + t20;
    
    /* Force spills by using all results */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                       "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10),
                       "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15),
                       "r"(t16), "r"(t17), "r"(t18), "r"(t19), "r"(t20));
    
    return result;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                     volatile int idx3, volatile int idx4)
{
    int result = 0;
    
    /* 1. Double register indirect with complex index */
    result += global_array[idx1 + idx2 * 3 - idx3 / (idx4 ? idx4 : 1)];
    
    /* 2. Large immediate offset (4096) */
    result += global_array[4096 + idx1];
    
    /* 3. Multi-word move with long long */
    long long ll1 = global_big_array[idx1];
    long long ll2 = global_big_array[idx2 + 100];
    long long ll3 = global_big_array[idx3 + 200];
    long long ll4 = global_big_array[idx4 + 300];
    
    /* Force use of all long long values */
    result += (int)(ll1 + ll2 - ll3 + ll4);
    
    /* 4. Misaligned access simulation with byte offsets */
    char *byte_ptr = (char *)global_array;
    result += byte_ptr[idx1 * 7 + 3];  /* Odd offset */
    result += byte_ptr[idx2 * 9 + 5];  /* Another odd offset */
    
    /* 5. Double type requiring possible x87 or SSE reloads */
    double d1 = global_double_array[idx1];
    double d2 = global_double_array[idx2];
    double d3 = global_double_array[idx3];
    double d4 = global_double_array[idx4];
    
    result += (int)(d1 + d2 + d3 + d4);
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y,
                                              volatile int z, volatile int w)
{
    int a = x * y;
    int b = z + w;
    int c = x ^ z;
    int d = y | w;
    
    /* Complex computation before asm */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = a * b;
    int t4 = c ^ d;
    
    /* Inline asm that clobbers many registers */
    asm volatile(
        "/* Clobber many registers to force spills */\n\t"
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
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Use values after asm to force reloads */
    int result = t1 + t2 - t3 * t4;
    result += a * b + c - d;
    
    return result;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(volatile int base)
{
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
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    double f3 = base * 3.3;
    
    /* Call function with many args - each may need register reload */
    int result = many_args_func(i1, i2, i3, i4, i5,
                                i6, i7, i8, i9, i10,
                                f1, f2, f3,
                                (void *)&global_array[i1],
                                (void *)&global_big_array[i2]);
    
    return result;
}

/* Test 5: Explicit register variables to stress specific register classes */
int __attribute__((noinline)) test_explicit_registers(volatile int a, volatile int b)
{
    /* Try to allocate specific registers (x86-64 specific) */
    register int r10_val asm("r10") = a * 3;
    register int r11_val asm("r11") = b * 5;
    register int r12_val asm("r12") = a + b;
    register int r13_val asm("r13") = a ^ b;
    
    /* Force use of these register variables in complex expressions */
    int t1 = r10_val + r11_val;
    int t2 = r12_val - r13_val;
    int t3 = r10_val * r11_val;
    int t4 = r12_val ^ r13_val;
    
    /* More computations to create pressure */
    int t5 = t1 + t2;
    int t6 = t3 - t4;
    int t7 = t5 * t6;
    int t8 = t1 ^ t2;
    int t9 = t3 | t4;
    
    /* Use 64-bit values to potentially require different register class */
    long long ll1 = (long long)r10_val * r11_val;
    long long ll2 = (long long)r12_val * r13_val;
    long long ll3 = ll1 + ll2;
    long long ll4 = ll1 - ll2;
    
    int result = t7 + t8 - t9 + (int)(ll3 + ll4);
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    volatile int seed = argc;
    
    /* Initialize some global data */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
        if (i < 2000) global_big_array[i] = i * 5LL;
        if (i < 1000) global_double_array[i] = i * 1.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed, seed+1, seed+2, seed+3);
    total += test_complex_addressing(seed+10, seed+11, seed+12, seed+13);
    total += test_asm_clobber(seed+20, seed+21, seed+22, seed+23);
    total += test_many_args(seed+30);
    total += test_explicit_registers(seed+40, seed+41);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
