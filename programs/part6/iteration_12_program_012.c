/* reload_test.c - Test program to trigger GCC reload pass initialization */
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
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Force use of all arguments to prevent elimination */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
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
    
    /* Many independent arithmetic expressions to force register allocation */
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
    
    /* Force all values to be live simultaneously */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int offset = 4096;  /* Large immediate */
    volatile int scale = 8;
    
    /* Force double register indirect addressing */
    int val1 = global_array[index + offset];  /* index + large offset */
    int val2 = global_array[offset + index * scale];  /* scaled index */
    
    /* Misaligned long long access */
    long long ll_val = global_big_array[index];
    int ll_part1 = (int)(ll_val & 0xFFFFFFFF);
    int ll_part2 = (int)(ll_val >> 32);
    
    /* Double with potential alignment issues */
    double d_val = global_double_array[index];
    int d_as_int = (int)d_val;
    
    /* Complex expression in array index */
    int complex_idx = (index * 3 + 7) / 2;
    int val3 = global_array[complex_idx + 1024];  /* Another large offset */
    
    return val1 + val2 + ll_part1 + ll_part2 + d_as_int + val3;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber general purpose, segment, and floating point */
    asm volatile(
        "# Dummy assembly\n"
        : 
        : "r"(a), "r"(b)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
    );
    
    /* More computations after clobber */
    int e = c * d;
    int f = (c << 3) | (d & 0xFF);
    int g = e ^ f;
    
    /* Another assembly block */
    asm volatile(
        "# More dummy assembly\n"
        : "+r"(g)
        :
        : "rax", "rbx", "rcx", "memory"
    );
    
    return g + a + b;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Create many different typed arguments */
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
    
    void* p1 = (void*)&global_array[0];
    void* p2 = (void*)&global_big_array[0];
    
    /* Call function with many arguments - forces register/stack allocation */
    int result = many_args_func(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
                               f1, f2, f3, p1, p2);
    
    /* More computations to keep values live */
    int extra = i1 + i2 + i3;
    double fextra = f1 + f2 + f3;
    
    return result + extra + (int)fextra;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed) {
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mixed size types */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed * 3;
    long long ll1 = (long long)seed * 1000;
    
    /* Floating point types */
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    
    /* Complex expression mixing all types */
    int t1 = r1 + r2;
    int t2 = c1 + s1 + i1;
    long long t3 = ll1 + (long long)t1;
    double t4 = f1 + d1 + t2;
    
    /* Force memory operations with different alignments */
    struct __attribute__((packed)) {
        char a;
        int b;
        long long c;
    } packed_struct;
    
    packed_struct.a = c1;
    packed_struct.b = i1;
    packed_struct.c = ll1;
    
    /* Access misaligned fields */
    int from_struct = packed_struct.b;
    long long ll_from_struct = packed_struct.c;
    
    return t1 + t2 + (int)t3 + (int)t4 + from_struct + (int)ll_from_struct;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    int result1 = test_register_pressure(seed);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_complex_addressing(seed % 100);
    printf("Test 2 result: %d\n", result2);
    
    int result3 = test_asm_clobber(seed, seed * 2);
    printf("Test 3 result: %d\n", result3);
    
    int result4 = test_many_args(seed);
    printf("Test 4 result: %d\n", result4);
    
    int result5 = test_mixed_types(seed);
    printf("Test 5 result: %d\n", result5);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3 + result4 + result5;
    printf("Final result: %d\n", final_result);
    
    return final_result == 0 ? 1 : 0;  /* Non-zero return if all zero (unlikely) */
}
