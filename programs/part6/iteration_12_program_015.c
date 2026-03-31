/* reload_test.c - Test program to trigger GCC's reload pass initialization */
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
    double f1, double f2, void *p1, void *p2, long long ll1)
{
    /* Force use of all arguments to prevent optimization */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    result += (int)ll1;
    return result;
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
    
    /* Many independent computations creating many live temporaries */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = t1 * t2;
    int t7 = t3 * t4;
    int t8 = t5 * t6;
    int t9 = t7 * t8;
    int t10 = t1 + t9;
    
    int t11 = b + c;
    int t12 = d + e;
    int t13 = f + g;
    int t14 = h + i;
    int t15 = j + a;
    int t16 = t11 * t12;
    int t17 = t13 * t14;
    int t18 = t15 * t16;
    int t19 = t17 * t18;
    int t20 = t11 + t19;
    
    int t21 = c + d;
    int t22 = e + f;
    int t23 = g + h;
    int t24 = i + j;
    int t25 = a + b;
    int t26 = t21 * t22;
    int t27 = t23 * t24;
    int t28 = t25 * t26;
    int t29 = t27 * t28;
    int t30 = t21 + t29;
    
    /* Force all temporaries to be live simultaneously */
    return t10 + t20 + t30 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with complex computation */
    int complex_idx = (index * 3 + 7) / 2;
    result += global_array[complex_idx + 1024];
    
    /* Double register indirect-like access */
    int base = index * 100;
    int offset = (index % 10) * 20;
    result += global_array[base + offset + 2048];
    
    /* Misaligned 64-bit access forcing piecewise moves */
    volatile long long ll_temp = global_big_array[index];
    result += (int)ll_temp;
    
    /* Double with potential alignment issues */
    volatile double d_temp = global_double_array[index % 100];
    result += (int)d_temp;
    
    /* Multi-word struct access */
    struct two_words {
        long long a;
        long long b;
    } __attribute__((packed));
    
    volatile struct two_words tw;
    tw.a = index;
    tw.b = index * 2;
    result += (int)tw.a + (int)tw.b;
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 5;
    int c = a + b;
    int d = a * b;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber general purpose registers */
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
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More computations after clobber - forces reloads */
    int e = c * d;
    int f = e + a;
    int g = f * b;
    
    /* Another assembly block clobbering different registers */
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
    
    return g + e + f;
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
    
    double f1 = base * 1.5;
    double f2 = base * 2.5;
    
    void *p1 = (void*)(intptr_t)(base + 100);
    void *p2 = (void*)(intptr_t)(base + 200);
    
    long long ll1 = (long long)base * 1000LL;
    
    /* Call function with many arguments - forces register pressure */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, p1, p2, ll1);
    
    /* Call it again with different values */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                f2, f1, p2, p1, ll1 * 2);
    
    return result;
}

/* Test 5: Machine-specific register constraints (x86_64 specific) */
#ifdef __x86_64__
int __attribute__((noinline)) test_register_constraints(int x) {
    /* Explicit register variables using scarce registers */
    register int r10_var asm("r10") = x * 2;
    register int r11_var asm("r11") = x * 3;
    
    /* Force use of these register variables in computations */
    int a = r10_var + 1;
    int b = r11_var + 2;
    
    /* Use them in complex expressions */
    asm volatile (
        "# Use specific registers\n\t"
        "add %1, %0\n\t"
        : "+r" (r10_var)
        : "r" (r11_var)
        : "cc"
    );
    
    /* More computations forcing register pressure */
    int c = a * b;
    int d = c + r10_var;
    int e = d * r11_var;
    
    /* Force x87 stack usage with double computations */
    volatile double dx = (double)x;
    volatile double dy = dx * 3.14159;
    volatile double dz = dy / 2.71828;
    
    return e + (int)dz + r10_var + r11_var;
}
#endif

/* Main function orchestrating all tests */
int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Starting reload pass stress tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    int result1 = test_register_pressure(seed);
    printf("Test 1 (register pressure): %d\n", result1);
    
    int result2 = test_complex_addressing(seed % 100);
    printf("Test 2 (complex addressing): %d\n", result2);
    
    int result3 = test_asm_clobber(seed, seed * 2);
    printf("Test 3 (asm clobber): %d\n", result3);
    
    int result4 = test_many_args(seed);
    printf("Test 4 (many args): %d\n", result4);
    
#ifdef __x86_64__
    int result5 = test_register_constraints(seed);
    printf("Test 5 (register constraints): %d\n", result5);
#else
    int result5 = 0;
    printf("Test 5 skipped (x86_64 specific)\n");
#endif
    
    /* Combine all results to ensure all code paths are live */
    int final_result = result1 + result2 + result3 + result4 + result5;
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
