/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) 
many_args_function(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 * 100.0) + (int)(f2 * 50.0) + (int)(f3 * 25.0);
    result += (intptr_t)p1 % 1000 + (intptr_t)p2 % 500;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline))
test_register_pressure(volatile int a, volatile int b, volatile int c,
                       volatile int d, volatile int e, volatile int f)
{
    /* Force many simultaneous live variables */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = e * f;
    int t4 = a ^ c;
    int t5 = b | d;
    int t6 = e & f;
    int t7 = t1 + t2;
    int t8 = t3 - t4;
    int t9 = t5 * t6;
    int t10 = t7 ^ t8;
    int t11 = t9 | t10;
    int t12 = t1 & t2;
    int t13 = t3 + t4;
    int t14 = t5 - t6;
    int t15 = t7 * t8;
    int t16 = t9 ^ t10;
    int t17 = t11 | t12;
    int t18 = t13 & t14;
    int t19 = t15 + t16;
    int t20 = t17 - t18;
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    
    /* More computations to increase pressure */
    result += (a << 2) | (b << 3);
    result += (c >> 1) ^ (d >> 2);
    result += (e & 0xFF) * (f & 0xFF);
    
    return result;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline))
test_complex_addressing(volatile int idx1, volatile int idx2, 
                        volatile int idx3, volatile int idx4)
{
    int sum = 0;
    
    /* Large immediate offset - may require reload */
    sum += global_array[4096];
    sum += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    sum += global_array[idx1 * 2 + idx2];
    
    /* Complex index expression */
    sum += global_array[(idx1 * idx2) + (idx3 << 2) - idx4];
    
    /* Multi-word type with potential alignment issues */
    long long ll_val = global_big_array[idx1];
    ll_val += global_big_array[idx2 + 100];
    
    /* Double with complex addressing */
    double d_val = global_double_array[idx3];
    d_val += global_double_array[idx4 * 2];
    
    /* Mixed types in complex expression */
    sum += (int)ll_val + (int)(d_val * 100.0);
    
    /* Stack array with variable large offset */
    int local_array[500];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i;
    }
    sum += local_array[idx1 + 200];  /* Large variable offset */
    sum += local_array[idx2 * 3 + 300];
    
    return sum;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline))
test_asm_clobber(volatile int x, volatile int y, volatile int z)
{
    int result = 0;
    
    /* Computation before assembly */
    int a = x * y;
    int b = y + z;
    int c = z * x;
    int d = a ^ b;
    int e = b | c;
    int f = c & a;
    
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
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    /* Use variables after assembly - forces reloads */
    result = a + b + c + d + e + f;
    
    /* More computations with clobbered registers */
    result += (x << 3) | (y << 2);
    result ^= (z * 7);
    
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
    
    return result;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline))
test_many_args(volatile int base)
{
    int result = 0;
    
    /* Create many values to pass as arguments */
    int arg1 = base + 1;
    int arg2 = base + 2;
    int arg3 = base + 3;
    int arg4 = base + 4;
    int arg5 = base + 5;
    int arg6 = base + 6;
    int arg7 = base + 7;
    int arg8 = base + 8;
    int arg9 = base + 9;
    int arg10 = base + 10;
    double farg1 = base * 1.5;
    double farg2 = base * 2.5;
    double farg3 = base * 3.5;
    void *parg1 = (void*)(intptr_t)(base + 100);
    void *parg2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register/stack pressure */
    result = many_args_function(arg1, arg2, arg3, arg4, arg5,
                               arg6, arg7, arg8, arg9, arg10,
                               farg1, farg2, farg3, parg1, parg2);
    
    /* Call it again with different values */
    result += many_args_function(arg10, arg9, arg8, arg7, arg6,
                                arg5, arg4, arg3, arg2, arg1,
                                farg3, farg2, farg1, parg2, parg1);
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline))
test_mixed_types(volatile int a, volatile int b)
{
    /* Use explicit register variables to constrain register allocation */
    register int r1 asm("r10") = a * 2;
    register int r2 asm("r11") = b * 3;
    
    /* Mixed type computations */
    long long ll1 = (long long)a * b;
    long long ll2 = (long long)r1 * r2;
    
    double d1 = (double)a / (b ? b : 1);
    double d2 = (double)r1 / (r2 ? r2 : 1);
    
    /* Force use of explicit register variables in complex expression */
    int result = r1 + r2;
    result += (int)(ll1 % 1000);
    result += (int)(ll2 % 500);
    result += (int)(d1 * 100.0);
    result += (int)(d2 * 50.0);
    
    /* Access with complex addressing */
    result += global_array[r1 % 1000];
    result += global_big_array[r2 % 500];
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    volatile int seed = argc;
    volatile int a = (seed * 12345) % 100;
    volatile int b = (seed * 67890) % 100;
    volatile int c = (seed * 13579) % 100;
    volatile int d = (seed * 24680) % 100;
    volatile int e = (seed * 97531) % 100;
    volatile int f = (seed * 86420) % 100;
    
    int total = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i % 256;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 2LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 0.5;
    }
    
    printf("Starting reload tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(a, b, c, d, e, f);
    printf("Test 1 complete: %d\n", total);
    
    total += test_complex_addressing(a, b, c, d);
    printf("Test 2 complete: %d\n", total);
    
    total += test_asm_clobber(a, b, c);
    printf("Test 3 complete: %d\n", total);
    
    total += test_many_args(a);
    printf("Test 4 complete: %d\n", total);
    
    total += test_mixed_types(a, b);
    printf("Test 5 complete: %d\n", total);
    
    printf("Final result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
