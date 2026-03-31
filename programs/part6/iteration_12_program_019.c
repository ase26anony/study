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
    double f1, double f2, void *p1, void *p2)
{
    /* Use all arguments to prevent elimination */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 
           + (int)f1 + (int)f2 + (int)(intptr_t)p1 + (int)(intptr_t)p2;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed) {
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
    
    /* Many independent arithmetic expressions creating register pressure */
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
    
    /* Mix in some floating point for additional register class pressure */
    volatile double fa = t1 * 0.5;
    volatile double fb = t2 * 0.25;
    volatile double fc = t3 * 0.125;
    volatile double fd = t4 * 0.0625;
    volatile double fe = t5 * 0.03125;
    
    double ft1 = fa + fb;
    double ft2 = fc + fd;
    double ft3 = fe + ft1;
    double ft4 = ft2 + ft3;
    
    return t20 + (int)ft4;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int idx = index;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    int val2 = global_array[2048];
    
    /* Variable index with computation - double register indirect */
    int val3 = global_array[idx * 2 + 17];
    int val4 = global_array[idx * 3 - 5];
    
    /* Complex addressing with multiple operations */
    int val5 = global_array[(idx * idx) % 1000];
    int val6 = global_array[(val1 + val2) % 1000];
    
    /* Misaligned access simulation with long long */
    long long ll1 = global_big_array[idx];
    long long ll2 = global_big_array[idx + 1];
    
    /* Double type requiring potential multi-register moves */
    double d1 = global_double_array[idx % 100];
    double d2 = global_double_array[(idx + 1) % 100];
    
    /* Mixed addressing modes in expression */
    return val1 + val2 + val3 + val4 + val5 + val6 
           + (int)ll1 + (int)ll2 + (int)d1 + (int)d2;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber general purpose registers */
    asm volatile (
        "# Start of clobber assembly\n\t"
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "# End of clobber assembly"
        : /* no outputs */
        : "r" (c), "r" (d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* More computations after clobber - forces reloads */
    int e = c * 2;
    int f = d * 3;
    int g = e + f;
    int h = e * f;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# Second clobber block\n\t"
        : /* no outputs */
        : /* no inputs */
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    return g + h;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Create many values that need to be in registers for the call */
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
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register allocation */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, p1, p2);
    
    /* More computations to keep values live */
    int b1 = result * 2;
    int b2 = result * 3;
    
    return b1 + b2;
}

/* Test 5: Explicit register variables and specific modes */
int __attribute__((noinline)) test_explicit_registers(int x) {
    /* Try to use specific registers (x86_64 example) */
    register int r10_var asm("r10") = x * 2;
    register int r11_var asm("r11") = x * 3;
    
    /* Force these to be used and spilled */
    asm volatile (
        "add %1, %0\n\t"
        : "+r" (r10_var)
        : "r" (r11_var)
    );
    
    /* Use 64-bit values that might need multiple registers on 32-bit */
    long long ll1 = (long long)x * 1000000000LL;
    long long ll2 = (long long)x * 2000000000LL;
    long long ll3 = ll1 + ll2;
    
    /* Double precision calculations */
    double d1 = x * 3.14159;
    double d2 = x * 2.71828;
    double d3 = d1 * d2;
    
    return (int)r10_var + (int)ll3 + (int)d3;
}

int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 3LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    int result1 = test_register_pressure(seed);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_complex_addressing(seed % 500);
    printf("Test 2 result: %d\n", result2);
    
    int result3 = test_asm_clobber(seed, seed * 2);
    printf("Test 3 result: %d\n", result3);
    
    int result4 = test_many_args(seed);
    printf("Test 4 result: %d\n", result4);
    
    int result5 = test_explicit_registers(seed);
    printf("Test 5 result: %d\n", result5);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3 + result4 + result5;
    printf("Final result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
