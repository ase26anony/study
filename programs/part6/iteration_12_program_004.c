/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[20000];
volatile double global_double_array[5000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, double f4,
    void *p1, void *p2, void *p3)
{
    /* Force use of all arguments */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3 + f4;
    volatile uintptr_t psum = (uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3;
    
    return (int)(sum + (int)fsum + (int)(psum & 0xFFFFFFFF));
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
    
    /* Force all values to be used in final computation */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index) {
    volatile int idx = index;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    int val2 = global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int val3 = global_array[idx * 3 + 100];
    int val4 = global_array[idx * 7 + 200];
    
    /* Complex addressing with multiple operations */
    int val5 = global_array[(idx * idx) % 1000 + 3000];
    
    /* Misaligned access simulation with 64-bit values */
    long long ll1 = global_big_array[idx];
    long long ll2 = global_big_array[idx + 100];
    
    /* Double precision floating with potential alignment issues */
    double d1 = global_double_array[idx];
    double d2 = global_double_array[idx + 50];
    
    /* Combine all results */
    return val1 + val2 + val3 + val4 + val5 + (int)ll1 + (int)ll2 + (int)d1 + (int)d2;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int result;
    
    /* Computation before assembly */
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = x * y;
    int e = a * b;
    int f = c + d;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber commonly used registers */
    asm volatile (
        "# Start of clobber block\n"
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        "# Doing some dummy operations\n"
        "add %%ebx, %%eax\n"
        "imul $123, %%eax, %%ecx\n"
        : 
        : "r"(f), "r"(e)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More computations after assembly - forcing reloads */
    int g = a * 2;
    int h = b * 3;
    int i = c * 4;
    int j = d * 5;
    
    result = g + h + i + j + f + e;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# Second clobber block\n"
        "mov %0, %%r8d\n"
        "mov %1, %%r9d\n"
        "add %%r9d, %%r8d\n"
        : 
        : "r"(result), "r"(x)
        : "r8", "r9", "r10", "r11", "memory"
    );
    
    return result + x + y;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
    /* Create many different values for arguments */
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
    
    void *p1 = (void*)&global_array[base];
    void *p2 = (void*)&global_big_array[base];
    void *p3 = (void*)&global_double_array[base];
    
    /* Call function with many arguments - forcing register/stack pressure */
    int result1 = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                 f1, f2, f3, f4, p1, p2, p3);
    
    /* Call again with different values */
    int result2 = many_args_func(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                 f4, f3, f2, f1, p3, p2, p1);
    
    return result1 + result2;
}

/* Test 5: Explicit register variables and special types */
#ifdef __x86_64__
int __attribute__((noinline)) test_explicit_registers(int x) {
    /* Explicit register variables - tie up specific registers */
    register int r10_var asm("r10") = x * 2;
    register int r11_var asm("r11") = x * 3;
    register int r12_var asm("r12") = x * 4;
    register int r13_var asm("r13") = x * 5;
    
    /* Use these variables in computations */
    int a = r10_var + r11_var;
    int b = r12_var + r13_var;
    
    /* Force spilling by using many other variables */
    int c = x * 6;
    int d = x * 7;
    int e = x * 8;
    int f = x * 9;
    int g = x * 10;
    int h = x * 11;
    int i = x * 12;
    int j = x * 13;
    int k = x * 14;
    int l = x * 15;
    
    /* Complex computation using all variables */
    int result = (a * b) + (c * d) + (e * f) + (g * h) + (i * j) + (k * l);
    
    /* Use register variables again to keep them live */
    result += r10_var - r11_var + r12_var - r13_var;
    
    return result;
}
#endif

/* Main function orchestrating all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = 0;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Starting reload pass tests...\n");
    
    /* Test 1: Register pressure */
    printf("Test 1: Register pressure...\n");
    result += test_register_pressure(seed);
    
    /* Test 2: Complex addressing */
    printf("Test 2: Complex addressing...\n");
    result += test_complex_addressing(seed % 100);
    
    /* Test 3: Assembly clobber */
    printf("Test 3: Assembly clobber...\n");
    result += test_asm_clobber(seed, seed * 2);
    
    /* Test 4: Many arguments */
    printf("Test 4: Many arguments...\n");
    result += test_many_args(seed);
    
    /* Test 5: Explicit registers (x86_64 specific) */
#ifdef __x86_64__
    printf("Test 5: Explicit registers...\n");
    result += test_explicit_registers(seed);
#endif
    
    /* Use global arrays to prevent optimization */
    global_array[0] = result;
    global_big_array[0] = result;
    global_double_array[0] = result;
    
    printf("Final result: %d\n", result);
    
    /* Return non-zero to ensure all code paths matter */
    return result != 0 ? 0 : 1;
}
