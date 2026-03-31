/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void* p1, void* p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                     volatile int c, volatile int d)
{
    /* Force many independent computations that must stay live */
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
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), 
                       "r"(t5), "r"(t6), "r"(t7), "r"(t8),
                       "r"(t9), "r"(t10), "r"(t11), "r"(t12),
                       "r"(t13), "r"(t14), "r"(t15), "r"(t16),
                       "r"(t17), "r"(t18), "r"(t19), "r"(t20),
                       "r"(t21), "r"(t22), "r"(t23), "r"(t24));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                      volatile int idx3)
{
    int result = 0;
    
    /* Large immediate offset - may need reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int complex_idx1 = (idx1 * idx2) + (idx3 << 2);
    result += global_array[complex_idx1];
    
    /* Multi-word move with long long */
    long long ll_val = global_big_array[idx1] + global_big_array[idx2];
    result += (int)ll_val;
    
    /* Misaligned access simulation with byte offset */
    char* byte_ptr = (char*)global_array;
    result += byte_ptr[idx1 * 4 + 1];  /* Misaligned int access */
    
    /* Double type requiring specific handling */
    double d_val = global_double_array[idx2] * 2.0;
    result += (int)d_val;
    
    /* Nested array indexing */
    result += global_array[global_array[idx1] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Clobber many registers to force spills */
    asm volatile(
        "# Clobber many registers\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use values after clobber - they must be reloaded */
    int e = c * d;
    int f = a * c + b * d;
    
    return e + f;
}

/* Test 4: Function with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(volatile int base)
{
    /* Compute many argument values */
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
    double farg1 = (double)base * 1.5;
    double farg2 = (double)base * 2.5;
    void* parg1 = (void*)(intptr_t)(base + 100);
    void* parg2 = (void*)(intptr_t)(base + 200);
    
    /* Call forces register allocation for all arguments */
    int result = many_args_function(arg1, arg2, arg3, arg4, arg5,
                                    arg6, arg7, arg8, arg9, arg10,
                                    farg1, farg2, parg1, parg2);
    
    /* More computations after call - registers may need reloading */
    int post1 = arg1 * arg2;
    int post2 = arg3 * arg4;
    int post3 = (int)farg1 + (int)farg2;
    
    return result + post1 + post2 + post3;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(volatile int x)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = x * 2;
    register int r2 asm("r11") = x * 3;
    
    /* Mix different sized types */
    long long ll1 = (long long)x * 1000LL;
    long long ll2 = (long long)x * 2000LL;
    double d1 = (double)x / 3.0;
    double d2 = (double)x / 7.0;
    
    /* Force use of all values */
    int sum_int = r1 + r2;
    long long sum_ll = ll1 + ll2;
    double sum_d = d1 + d2;
    
    /* Complex expression mixing types */
    int result = sum_int + (int)sum_ll + (int)sum_d;
    
    /* Access with large offset */
    result += global_array[result & 0xFFF];
    
    return result;
}

int main(int argc, char* argv[])
{
    volatile int seed = argc;
    int total = 0;
    
    /* Initialize some global data */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 5LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed, seed+1, seed+2, seed+3);
    total += test_complex_addressing(seed, seed+10, seed+20);
    total += test_asm_clobber(seed+100, seed+200);
    total += test_many_args(seed+300);
    total += test_mixed_types(seed+400);
    
    /* Use volatile to prevent optimization of final result */
    volatile int final_result = total;
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
