/* reload_test.c - Program to trigger GCC's reload pass initialization */
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
    double f1, double f2, void *p1, void *p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                     int e, int f, int g, int h)
{
    /* Use volatile to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    volatile int v7 = g;
    volatile int v8 = h;
    
    /* Create many independent computations to force register pressure */
    int t1 = v1 + v2;
    int t2 = v3 + v4;
    int t3 = v5 + v6;
    int t4 = v7 + v8;
    int t5 = v1 * v3;
    int t6 = v2 * v4;
    int t7 = v5 * v7;
    int t8 = v6 * v8;
    int t9 = t1 + t2;
    int t10 = t3 + t4;
    int t11 = t5 + t6;
    int t12 = t7 + t8;
    int t13 = t9 * t10;
    int t14 = t11 * t12;
    int t15 = t13 + t14;
    int t16 = t1 * t3;
    int t17 = t2 * t4;
    int t18 = t5 * t7;
    int t19 = t6 * t8;
    int t20 = t16 + t17;
    int t21 = t18 + t19;
    int t22 = t20 * t21;
    int t23 = t15 + t22;
    
    /* More computations to ensure spillage */
    int t24 = t23 * 2;
    int t25 = t24 / 3;
    int t26 = t25 << 2;
    int t27 = t26 >> 1;
    int t28 = t27 ^ 0x55;
    int t29 = t28 | 0xAA;
    int t30 = t29 & 0xFF;
    
    return t30;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                      int index3, int index4)
{
    /* Force large immediate offsets */
    int val1 = global_array[4096];  /* Large offset */
    int val2 = global_array[8192];  /* Even larger offset */
    
    /* Complex array indexing with variable computations */
    int complex_idx1 = (index1 * index2) + (index3 / 2);
    int complex_idx2 = (index2 * 3) + (index4 * 5) - 7;
    
    /* Multi-level array access */
    int result1 = global_array[complex_idx1 + 1024];
    int result2 = global_array[complex_idx2 + 2048];
    
    /* Access with double computation in index */
    int result3 = global_array[(index1 + index2) * (index3 - index4)];
    
    /* Force misaligned access for long long */
    long long ll1 = global_big_array[complex_idx1];
    long long ll2 = global_big_array[complex_idx2];
    
    /* Force double precision floating point moves */
    double d1 = global_double_array[index1];
    double d2 = global_double_array[index2];
    
    /* Combine results in complex way */
    return val1 + val2 + result1 + result2 + result3 + (int)ll1 + (int)ll2 + (int)d1 + (int)d2;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int result = a + b + c + d;
    
    /* First do some computation to get values in registers */
    int t1 = a * b;
    int t2 = c * d;
    int t3 = t1 + t2;
    int t4 = t1 - t2;
    int t5 = t3 * t4;
    
    /* Clobber many registers to force spills and reloads */
    /* x86_64 specific register clobbers */
    asm volatile(
        "# Start of clobber assembly\n"
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        :
        : "r"(t3), "r"(t4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc"
    );
    
    /* More computations after clobber to force reloads */
    int t6 = t5 * 2;
    int t7 = t6 / 3;
    int t8 = t7 << 1;
    
    return result + t5 + t6 + t7 + t8;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
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
    
    /* Call function with many arguments - forces register pressure for argument passing */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, p1, p2);
    
    /* Do more work after call to force reloads of preserved registers */
    int t1 = result * 2;
    int t2 = t1 + a1;
    int t3 = t2 * a2;
    int t4 = t3 / a3;
    
    return t4;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, double c, double d)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = a;
    register int r2 asm("r11") = b;
    register double r3 asm("xmm8") = c;
    register double r4 asm("xmm9") = d;
    
    /* Mix integer and floating point computations */
    int i1 = r1 + r2;
    double f1 = r3 + r4;
    
    /* Convert between types to force moves between register classes */
    int i2 = (int)f1;
    double f2 = (double)i1;
    
    /* More mixed operations */
    int i3 = i1 * i2;
    double f3 = f1 * f2;
    
    /* Access memory with mixed types */
    global_array[i3] = (int)f3;
    global_double_array[i2] = f2;
    
    return i3 + (int)f3;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3, 
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(base, base+1, base+2, base+3);
    
    total += test_asm_clobber(base, base+1, base+2, base+3);
    
    total += test_many_args(base);
    
    total += test_mixed_types(base, base+1, (double)base*1.1, (double)base*2.2);
    
    /* Additional stress: nested function calls with register pressure */
    for (int i = 0; i < 10; i++) {
        total += test_register_pressure(total, i, base, argc, 
                                       total+i, base+i, argc, i);
    }
    
    printf("Total result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
