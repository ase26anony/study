/* reload_test.c - Comprehensive test to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789L;
volatile double g_volatile_double = 3.14159;
volatile void* g_volatile_ptr = NULL;

/* Large global array with large offsets */
int global_array[10000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Complex computation using all arguments */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double product = f1 * f2 * f3;
    long addr_diff = (long)p2 - (long)p1;
    
    return sum + (int)product + (int)(addr_diff >> 2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Force many independent live variables */
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
    
    /* Many independent computations to force register allocation */
    int t1 = a + b + g_volatile_int;
    int t2 = c * d - g_volatile_int;
    int t3 = e / (f ? f : 1) + g_volatile_int;
    int t4 = g ^ h ^ g_volatile_int;
    int t5 = i | j | g_volatile_int;
    int t6 = k & l & g_volatile_int;
    int t7 = m << (n % 8);
    int t8 = o >> (p % 8);
    int t9 = q - r + s;
    int t10 = t * 2 - a;
    int t11 = b * c + d;
    int t12 = e * f - g;
    int t13 = h / (i ? i : 1) + j;
    int t14 = k ^ l ^ m;
    int t15 = n | o | p;
    int t16 = q & r & s;
    int t17 = t << (a % 8);
    int t18 = b >> (c % 8);
    int t19 = d - e + f;
    int t20 = g * 2 - h;
    
    /* Use all temporaries to keep them live */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int* base, int index)
{
    int result = 0;
    
    /* Large immediate offset */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Complex index calculation */
    result += base[(index * 3 + g_volatile_int) & 0xFF];
    
    /* Double register indirect with computation */
    int* ptr = base + index;
    result += ptr[g_volatile_int % 100];
    
    /* Multi-dimensional access */
    int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex 2D indexing */
    result += matrix[index % 10][(index * 7) % 10];
    
    /* Misaligned access with long long */
    long long ll_array[100];
    for (int i = 0; i < 100; i++) {
        ll_array[i] = (long long)i * i;
    }
    
    /* Force piecewise move of long long */
    long long ll_sum = 0;
    for (int i = 0; i < 10; i++) {
        ll_sum += ll_array[i * 3 + index];
    }
    
    return result + (int)(ll_sum % 1000000);
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int result = a + b + c + d;
    
    /* Computation before assembly */
    int t1 = a * b + g_volatile_int;
    int t2 = c * d - g_volatile_int;
    int t3 = (a ^ b) | (c ^ d);
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber commonly used registers */
    asm volatile(
        "# Dummy assembly\n\t"
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "xmm6", "xmm7",
          "cc", "memory"
    );
    
    /* More computations after assembly - forces reloads */
    int t4 = t1 * t2 + t3;
    int t5 = (t1 ^ t2) | (t3 ^ result);
    int t6 = t4 * t5 - g_volatile_int;
    
    /* Another assembly block with different clobbers */
    asm volatile(
        "# More dummy assembly\n\t"
        "nop\n\t"
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rdx", "rsi", "rdi", "r8", "r9", "r10",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc", "memory"
    );
    
    return t6 + t4 + t5;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int seed)
{
    /* Prepare many arguments with complex computations */
    int a1 = seed * 1 + g_volatile_int;
    int a2 = seed * 2 - g_volatile_int;
    int a3 = seed * 3 ^ g_volatile_int;
    int a4 = seed * 4 | g_volatile_int;
    int a5 = seed * 5 & g_volatile_int;
    int a6 = seed * 6 + g_volatile_int;
    int a7 = seed * 7 - g_volatile_int;
    int a8 = seed * 8 ^ g_volatile_int;
    int a9 = seed * 9 | g_volatile_int;
    int a10 = seed * 10 & g_volatile_int;
    
    double f1 = (double)seed * 1.1 + g_volatile_double;
    double f2 = (double)seed * 2.2 - g_volatile_double;
    double f3 = (double)seed * 3.3 * g_volatile_double;
    
    void* p1 = (void*)((long)global_array + seed * 100);
    void* p2 = (void*)((long)global_array + seed * 200 + 4096);
    
    /* Call function with many arguments - forces register pressure */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    /* More computations after call */
    int t1 = a1 + a2 + a3;
    int t2 = a4 * a5 - a6;
    int t3 = (a7 ^ a8) | (a9 ^ a10);
    
    return result + t1 + t2 + t3;
}

/* Test 5: Mixed types and register classes */
int __attribute__((noinline)) test_mixed_types(int seed)
{
    /* Use different types to force different register classes */
    char c1 = seed & 0xFF;
    char c2 = (seed >> 8) & 0xFF;
    short s1 = seed & 0xFFFF;
    short s2 = (seed >> 16) & 0xFFFF;
    int i1 = seed * 1;
    int i2 = seed * 2;
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed * 2000000LL;
    float f1 = (float)seed * 1.5f;
    float f2 = (float)seed * 2.5f;
    double d1 = (double)seed * 3.14159;
    double d2 = (double)seed * 2.71828;
    
    /* Mixed type computations */
    int r1 = c1 + c2 + s1 + s2;
    int r2 = i1 * i2 - (int)(ll1 % 1000);
    float r3 = f1 * f2 + (float)(ll2 % 1000);
    double r4 = d1 / (d2 != 0.0 ? d2 : 1.0);
    
    /* Force register pressure with all types live */
    asm volatile("# Mixed types barrier" : : : "memory");
    
    /* Use all results */
    return r1 + (int)r2 + (int)r3 + (int)r4;
}

/* Main orchestrator */
int main(int argc, char* argv[])
{
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    
    printf("Starting reload pass stress tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", result);
    
    int local_array[1000];
    for (int i = 0; i < 1000; i++) {
        local_array[i] = i * 5;
    }
    
    result += test_complex_addressing(local_array, seed % 1000);
    printf("Test 2 complete: %d\n", result);
    
    result += test_asm_clobber(seed, seed+1, seed+2, seed+3);
    printf("Test 3 complete: %d\n", result);
    
    result += test_many_args(seed);
    printf("Test 4 complete: %d\n", result);
    
    result += test_mixed_types(seed);
    printf("Test 5 complete: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    g_volatile_ptr = (void*)(long)result;
    
    return result != 0 ? 0 : 1;
}
