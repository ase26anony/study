/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789;
volatile double g_volatile_double = 3.14159;
volatile int* g_volatile_ptr = NULL;

/* Large global array to force complex addressing */
int global_array[10000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double* p1, long l1, void* ptr)
{
    /* Force use of all arguments */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + 
           (int)f1 + (int)f2 + (int)(*p1) + (int)l1 + (int)(intptr_t)ptr;
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
    
    /* Create many independent computations to force register pressure */
    int t1 = v1 + v2;
    int t2 = v3 - v4;
    int t3 = v5 * v6;
    int t4 = v1 * v3;
    int t5 = v2 * v4;
    int t6 = v5 + v6;
    int t7 = v1 - v3;
    int t8 = v2 - v4;
    int t9 = v5 * v1;
    int t10 = v6 * v2;
    int t11 = v3 + v5;
    int t12 = v4 + v6;
    int t13 = v1 * v5;
    int t14 = v2 * v6;
    int t15 = v3 - v1;
    int t16 = v4 - v2;
    int t17 = v5 + v3;
    int t18 = v6 + v4;
    int t19 = v1 * v6;
    int t20 = v2 * v5;
    int t21 = v3 * v4;
    int t22 = v1 + v4;
    int t23 = v2 + v5;
    int t24 = v3 + v6;
    int t25 = v4 * v1;
    int t26 = v5 * v2;
    int t27 = v6 * v3;
    int t28 = v1 - v5;
    int t29 = v2 - v6;
    int t30 = v3 - v2;
    
    /* Force all temporaries to be live simultaneously */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Force complex addressing modes */
    
    /* 1. Large immediate offset */
    int val1 = global_array[4096] + global_array[8192];
    
    /* 2. Variable index with computation */
    int val2 = global_array[idx * 3 + 7];
    
    /* 3. Nested array access with computation */
    int val3 = global_array[global_array[idx] & 0xFFF];
    
    /* 4. Multi-dimensional style addressing */
    int val4 = global_array[(idx << 4) + (idx >> 2)];
    
    /* 5. Address computation with multiple terms */
    int val5 = global_array[idx + 256] + global_array[idx + 512];
    
    /* Use long long to force multi-register operations */
    long long big_val = (long long)val1 * val2;
    big_val += (long long)val3 * val4;
    big_val += (long long)val5 * idx;
    
    /* Force misaligned access simulation */
    char* byte_ptr = (char*)&big_val;
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result += byte_ptr[i];
    }
    
    return result + (int)(big_val >> 32);
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int result = 0;
    
    /* Do some computation first */
    int t1 = a * b;
    int t2 = c * d;
    int t3 = a + c;
    int t4 = b + d;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber general purpose registers */
    asm volatile (
        "# Start of clobbering assembly\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "# End of clobbering assembly\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* More computation after clobbering - forces reloads */
    int t5 = t1 * t2;
    int t6 = t3 * t4;
    int t7 = t1 + t3;
    int t8 = t2 + t4;
    
    result = t5 + t6 + t7 + t8;
    
    /* Another assembly block */
    asm volatile (
        "# Another clobber\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    return result;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(void)
{
    /* Create many different values */
    int a1 = g_volatile_int;
    int a2 = a1 * 2;
    int a3 = a2 + 1;
    int a4 = a3 - 5;
    int a5 = a4 * 3;
    int a6 = a5 / 2;
    int a7 = a6 + 7;
    int a8 = a7 - 3;
    int a9 = a8 * 4;
    int a10 = a9 / 2;
    
    double f1 = g_volatile_double;
    double f2 = f1 * 2.0;
    
    double local_double = 2.71828;
    long local_long = g_volatile_long;
    void* local_ptr = &global_array[0];
    
    /* Call function with many arguments - forces register pressure for argument passing */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, &local_double, local_long, local_ptr);
    
    /* Call it again with different values */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                 f2, f1, &local_double, local_long + 1, local_ptr + 100);
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(void)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = g_volatile_int;
    register int r2 asm("r11") = r1 * 2;
    register int r3 asm("r12") = r2 + 1;
    
    /* Mix different sized types */
    short s1 = r1 & 0xFFFF;
    short s2 = r2 & 0xFFFF;
    short s3 = r3 & 0xFFFF;
    
    char c1 = r1 & 0xFF;
    char c2 = r2 & 0xFF;
    char c3 = r3 & 0xFF;
    
    long long ll1 = (long long)r1 * r2;
    long long ll2 = (long long)r2 * r3;
    long long ll3 = (long long)r3 * r1;
    
    /* Force conversions between types */
    double d1 = (double)r1;
    double d2 = (double)r2;
    double d3 = (double)r3;
    
    /* Complex expression with mixed types */
    int result = (int)(d1 + d2 + d3);
    result += (int)(ll1 >> 32);
    result += (int)(ll2 >> 32);
    result += (int)(ll3 >> 32);
    result += s1 + s2 + s3;
    result += c1 + c2 + c3;
    
    return result;
}

/* Main function orchestrates all tests */
int main(int argc, char* argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize global array */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    
    int total = 0;
    
    /* Run all tests */
    total += test_register_pressure(base, base+1, base+2, base+3, base+4, base+5);
    total += test_complex_addressing(base & 0xFFF);
    total += test_asm_clobber(base, base+1, base+2, base+3);
    total += test_many_args();
    total += test_mixed_types();
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
