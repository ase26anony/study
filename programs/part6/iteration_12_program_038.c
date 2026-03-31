/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long long g_volatile_ll = 0x123456789ABCDEF0LL;
volatile double g_volatile_double = 3.141592653589793;
volatile int* g_volatile_ptr = NULL;

/* Large global array to force complex addressing */
int global_array[10000] = {0};

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Force use of all arguments */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 
           + (int)f1 + (int)f2 + (int)f3 + (int)(intptr_t)p1 + (int)(intptr_t)p2;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Use volatile inputs to prevent optimization */
    volatile int a = seed + 1;
    volatile int b = seed + 2;
    volatile int c = seed + 3;
    volatile int d = seed + 4;
    volatile int e = seed + 5;
    volatile int f = seed + 6;
    volatile int g = seed + 7;
    volatile int h = seed + 8;
    volatile int i = seed + 9;
    volatile int j = seed + 10;
    volatile int k = seed + 11;
    volatile int l = seed + 12;
    volatile int m = seed + 13;
    volatile int n = seed + 14;
    volatile int o = seed + 15;
    volatile int p = seed + 16;
    volatile int q = seed + 17;
    volatile int r = seed + 18;
    volatile int s = seed + 19;
    volatile int t = seed + 20;
    
    /* Force many independent computations that must stay live */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = k + l;
    int t7 = m + n;
    int t8 = o + p;
    int t9 = q + r;
    int t10 = s + t;
    
    int t11 = t1 * t2;
    int t12 = t3 * t4;
    int t13 = t5 * t6;
    int t14 = t7 * t8;
    int t15 = t9 * t10;
    
    int t16 = t11 + t12;
    int t17 = t13 + t14;
    int t18 = t15 + t16;
    int t19 = t17 + t18;
    int t20 = t19 * 2;
    
    /* More computations to increase pressure */
    int u1 = a * c;
    int u2 = e * g;
    int u3 = i * k;
    int u4 = m * o;
    int u5 = q * s;
    
    int u6 = b * d;
    int u7 = f * h;
    int u8 = j * l;
    int u9 = n * p;
    int u10 = r * t;
    
    int u11 = u1 + u2 + u3 + u4 + u5;
    int u12 = u6 + u7 + u8 + u9 + u10;
    
    /* Force all values to be used in final result */
    return t20 + u11 + u12 + g_volatile_int;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    /* Variable index with complex computation */
    int idx1 = index * g_volatile_int;
    int idx2 = idx1 + (g_volatile_int >> 3);
    int idx3 = idx2 * 7;
    
    /* Large immediate offset */
    int val1 = global_array[4096];  /* Large immediate offset */
    int val2 = global_array[idx3];  /* Variable index */
    
    /* Double register indirect-like access */
    int* ptr = global_array + 2048;
    int val3 = ptr[idx2];  /* Base + scaled index */
    
    /* Multi-word type forcing piecewise moves */
    long long ll_val = g_volatile_ll;
    long long ll_result = ll_val + (long long)val1 + (long long)val2;
    
    /* Misaligned access simulation */
    char* char_ptr = (char*)global_array;
    int val4 = *(int*)(char_ptr + 1);  /* Potentially misaligned */
    
    /* Complex expression in array index */
    int val5 = global_array[(idx1 * idx2 + idx3) % 10000];
    
    return val1 + val2 + val3 + (int)ll_result + val4 + val5;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int result;
    
    /* Computation before assembly */
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = c * 2;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber commonly used registers */
    asm volatile (
        "# Clobber many registers\n\t"
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
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* More computations after assembly */
    int e = d + 42;
    int f = e * g_volatile_int;
    int g = f - x;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# Clobber more registers\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    result = g + y;
    return result;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many argument values with complex computations */
    int a1 = base + 1;
    int a2 = base * 2;
    int a3 = base + g_volatile_int;
    int a4 = base * 3;
    int a5 = a1 + a2 + a3 + a4;
    int a6 = a5 * 2;
    int a7 = a6 - base;
    int a8 = a7 + g_volatile_int;
    int a9 = a8 * 3;
    int a10 = a9 / 2;
    
    double f1 = (double)a1 * 1.1;
    double f2 = (double)a2 * 2.2;
    double f3 = g_volatile_double;
    
    void* p1 = (void*)(intptr_t)(a3 + a4);
    void* p2 = (void*)(intptr_t)(a5 + a6);
    
    /* Call function with many arguments - forces register/stack moves */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    /* Call it again with different values */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                 f3, f2, f1, p2, p1);
    
    return result;
}

/* Test 5: Explicit register variables and special types */
#ifdef __x86_64__
int __attribute__((noinline)) test_explicit_registers(int x)
{
    /* Explicit register variables - compete for specific registers */
    register int r10_var asm("r10") = x + 1;
    register int r11_var asm("r11") = x + 2;
    
    /* Use 64-bit types that might need multiple registers on 32-bit */
    long long ll1 = g_volatile_ll + x;
    long long ll2 = ll1 * 2;
    long long ll3 = ll2 + g_volatile_ll;
    
    /* Double precision computations */
    double d1 = (double)x * 1.234;
    double d2 = d1 + g_volatile_double;
    double d3 = d2 * 3.456;
    
    /* Mix explicit registers with other computations */
    int a = r10_var * 3;
    int b = r11_var * 7;
    
    /* Force spilling by using many variables */
    int c = a + b;
    int d = c + (int)ll1;
    int e = d + (int)d1;
    int f = e + (int)ll2;
    int g = f + (int)d2;
    int h = g + (int)ll3;
    int i = h + (int)d3;
    
    return i + r10_var + r11_var;
}
#endif

/* Main function orchestrates all tests */
int main(int argc, char* argv[])
{
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 1000;
    
    printf("Starting reload pass stress tests...\n");
    
    /* Run all tests sequentially */
    result += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", result);
    
    result += test_complex_addressing(seed % 5000);
    printf("Test 2 complete: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: %d\n", result);
    
    result += test_many_args(seed);
    printf("Test 4 complete: %d\n", result);
    
#ifdef __x86_64__
    result += test_explicit_registers(seed);
    printf("Test 5 complete: %d\n", result);
#endif
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
