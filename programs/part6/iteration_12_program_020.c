/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, double f4,
    void *p1, void *p2, void *p3)
{
    /* Force use of all arguments to prevent elimination */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3 + f4;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2 + (intptr_t)p3);
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
    
    /* Many independent computations to force register allocation */
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
    
    /* More computations to increase live range */
    int t21 = t20 + a - b;
    int t22 = t21 + c - d;
    int t23 = t22 + e - f;
    int t24 = t23 + g - h;
    int t25 = t24 + i - j;
    int t26 = t25 + k - l;
    int t27 = t26 + m - n;
    int t28 = t27 + o - p;
    int t29 = t28 + q - r;
    int t30 = t29 + s - t;
    
    return t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int complex_index;
    
    /* Force computation of index to be in register */
    complex_index = index * 3 + 7;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int val3 = global_array[complex_index + index * 2];
    
    /* Multi-word type requiring multiple registers */
    long long big_val = global_big_array[index];
    long long big_val2 = global_big_array[index + 100];
    
    /* Misaligned access simulation */
    char *byte_ptr = (char *)&global_big_array[index];
    int misaligned_int = *(int *)(byte_ptr + 1);
    
    /* Complex expression in array index */
    int val4 = global_array[(complex_index * 2) + (index << 3) - 17];
    
    /* Nested array access with computation */
    int val5 = global_array[global_array[index] & 0xFF];
    
    return val1 + val2 + val3 + (int)big_val + (int)big_val2 + 
           misaligned_int + val4 + val5;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int result;
    
    /* Computation before asm to create live values */
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = x - y;
    int e = x * y;
    int f = x << 3;
    int g = y >> 2;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber commonly used registers */
    asm volatile (
        "# Start of clobber assembly\n"
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        "# Doing some dummy operations\n"
        "add $1, %%eax\n"
        "add $2, %%ebx\n"
        "mov %%eax, %0\n"
        "mov %%ebx, %1\n"
        "# End of clobber assembly\n"
        : "+r" (a), "+r" (b)
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More computations after asm to force reloads */
    int h = c + d;
    int i = e + f;
    int j = g + a;
    int k = b + h;
    int l = i + j;
    
    result = k + l + a + b;
    
    /* Another asm block with different clobbers */
    asm volatile (
        "# Second clobber block\n"
        "mov %0, %%r12\n"
        "add $42, %%r12\n"
        "mov %%r12, %0\n"
        : "+r" (result)
        :
        : "r12", "r13", "r14", "r15", "cc"
    );
    
    return result;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
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
    
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    double f3 = base * 3.3;
    double f4 = base * 4.4;
    
    void *p1 = &global_array[0];
    void *p2 = &global_big_array[0];
    void *p3 = &global_double_array[0];
    
    /* Call function with many arguments - forces register/stack pressure */
    int result1 = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                     f1, f2, f3, f4, p1, p2, p3);
    
    /* Call it again with different values to prevent optimization */
    int result2 = many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                     f4, f3, f2, f1, p3, p2, p1);
    
    /* Do some computation with the results */
    int intermediate = result1 * 3 + result2 * 7;
    
    /* Another call with computed values */
    int result3 = many_args_function(
        intermediate, intermediate + 1, intermediate + 2,
        intermediate + 3, intermediate + 4, intermediate + 5,
        intermediate + 6, intermediate + 7, intermediate + 8,
        intermediate + 9,
        f1 * 2.0, f2 * 2.0, f3 * 2.0, f4 * 2.0,
        p1, p2, p3);
    
    return result1 + result2 + result3;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables for x86_64 */
    register int r10_var asm("r10") = seed * 3;
    register int r11_var asm("r11") = seed * 7;
    
    /* Mix different sized types */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed * 3;
    long long ll1 = (long long)seed * 1000;
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    
    /* Operations mixing types - may require conversions and reloads */
    int r1 = c1 + s1;
    int r2 = i1 + (int)ll1;
    float r3 = f1 + (float)d1;
    double r4 = d1 + (double)f1;
    
    /* Use the register variables */
    r10_var = r10_var * 2 + r1;
    r11_var = r11_var / 2 + r2;
    
    /* Access misaligned data */
    struct mixed {
        char c;
        int i;
        long long ll;
        double d;
    } __attribute__((packed)) m;
    
    m.c = c1;
    m.i = i1;
    m.ll = ll1;
    m.d = d1;
    
    /* Force reloads by using all values */
    int result = r10_var + r11_var + r1 + r2 + (int)r3 + (int)r4 + m.i;
    
    /* Large immediate in memory access */
    result += global_array[result & 0xFFF];
    
    return result;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argv to create volatile input to prevent constant propagation */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 1000LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", total);
    
    total += test_complex_addressing(seed);
    printf("Test 2 complete: %d\n", total);
    
    total += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: %d\n", total);
    
    total += test_many_args(seed);
    printf("Test 4 complete: %d\n", total);
    
    total += test_mixed_types(seed);
    printf("Test 5 complete: %d\n", total);
    
    printf("Final result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    return total == 0 ? 1 : 0;
}
