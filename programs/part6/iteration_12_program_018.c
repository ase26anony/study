/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void* p1, void* p2)
{
    volatile int result = 0;
    result += a1 + a2 + a3 + a4 + a5;
    result += a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                    volatile int c, volatile int d)
{
    /* Force many independent computations to create live ranges */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = a * b;
    int t4 = c * d;
    int t5 = a - b;
    int t6 = c - d;
    int t7 = a ^ b;
    int t8 = c ^ d;
    int t9 = a | b;
    int t10 = c | d;
    int t11 = a & b;
    int t12 = c & d;
    int t13 = a << 2;
    int t14 = c << 3;
    int t15 = b >> 1;
    int t16 = d >> 2;
    int t17 = t1 + t2;
    int t18 = t3 + t4;
    int t19 = t5 + t6;
    int t20 = t7 + t8;
    int t21 = t9 + t10;
    int t22 = t11 + t12;
    int t23 = t13 + t14;
    int t24 = t15 + t16;
    
    /* Force all values to be used to prevent dead code elimination */
    volatile int sum = 0;
    sum += t1; sum += t2; sum += t3; sum += t4;
    sum += t5; sum += t6; sum += t7; sum += t8;
    sum += t9; sum += t10; sum += t11; sum += t12;
    sum += t13; sum += t14; sum += t15; sum += t16;
    sum += t17; sum += t18; sum += t19; sum += t20;
    sum += t21; sum += t22; sum += t23; sum += t24;
    
    return sum;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                     volatile int idx3, volatile int idx4)
{
    int result = 0;
    
    /* Double register indirect with complex computation */
    result += global_array[idx1 + idx2 * 2];
    result += global_array[idx3 * 3 + idx4];
    
    /* Large immediate offset (4096 is often too large for some archs) */
    result += global_array[4096 + idx1];
    result += global_array[8192 + idx2];
    
    /* Multi-word moves with long long */
    long long ll1 = global_big_array[idx1];
    long long ll2 = global_big_array[idx2 + 100];
    long long ll3 = global_big_array[idx3 + 200];
    long long ll4 = global_big_array[idx4 + 300];
    
    result += (int)(ll1 + ll2 + ll3 + ll4);
    
    /* Double precision floating point with potential alignment issues */
    double d1 = global_double_array[idx1];
    double d2 = global_double_array[idx2 + 100];
    double d3 = global_double_array[idx3 + 200];
    
    result += (int)(d1 + d2 + d3);
    
    /* Complex array indexing with multiple computations */
    for (int i = 0; i < 10; i++) {
        result += global_array[(idx1 * i + idx2 * (i+1)) % 1000];
        result += global_array[(idx3 * (i+2) + idx4 * (i+3)) % 1000 + 1000];
    }
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 2;
    int b = y * 3;
    int c = x + y;
    int d = x - y;
    int e = x ^ y;
    int f = x | y;
    
    /* Clobber many registers to force spills */
    __asm__ volatile (
        "# Clobber many registers\n"
        "nop\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use all variables after asm to ensure they're live across it */
    volatile int result = a + b + c + d + e + f;
    
    /* Another asm with different clobbers */
    __asm__ volatile (
        "# More clobbers\n"
        "nop\n"
        : 
        : 
        : "r8", "r9", "r10", "r11", "r12", "r13"
    );
    
    return result;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(volatile int base)
{
    /* Create many argument values that need registers */
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
    void* p1 = (void*)&global_array[0];
    void* p2 = (void*)&global_array[1000];
    
    /* Call function with many arguments - each needs register/stack slot */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, p1, p2);
    
    /* Call it again with different values to increase pressure */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                 f2, f1, p2, p1);
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(volatile int x, volatile int y)
{
    /* Use explicit register variables to constrain register allocation */
    register int r1 asm("r10") = x * 2;
    register int r2 asm("r11") = y * 3;
    
    /* Mix different sized types */
    char c1 = x & 0xFF;
    short s1 = y & 0xFFFF;
    int i1 = x + y;
    long long ll1 = (long long)x * y;
    
    /* Force conversions between types */
    double d1 = (double)r1;
    float f1 = (float)r2;
    
    /* Complex expression mixing all types */
    volatile int result = 0;
    result += c1 + s1 + i1 + (int)ll1 + (int)d1 + (int)f1;
    
    /* Use the register variables in complex addressing */
    result += global_array[r1 % 1000];
    result += global_array[r2 % 1000 + 1000];
    
    return result;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use argv to create volatile inputs preventing constant propagation */
    volatile int input1 = argc > 1 ? atoi(argv[1]) : 1234;
    volatile int input2 = argc > 2 ? atoi(argv[2]) : 5678;
    volatile int input3 = argc > 3 ? atoi(argv[3]) : 9012;
    volatile int input4 = argc > 4 ? atoi(argv[4]) : 3456;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i % 100;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 2LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(input1, input2, input3, input4);
    total += test_complex_addressing(input1, input2, input3, input4);
    total += test_asm_clobber(input1, input2);
    total += test_many_args(input1);
    total += test_mixed_types(input1, input2);
    
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
