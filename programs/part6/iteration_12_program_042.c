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
    /* Force use of all arguments */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2;
    volatile intptr_t psum = (intptr_t)p1 + (intptr_t)p2;
    
    return (int)(sum + (int)fsum + (int)psum);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                    int e, int f, int g, int h)
{
    /* Use volatile inputs to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    volatile int v7 = g;
    volatile int v8 = h;
    
    /* Create many independent computations to force register pressure */
    /* Each computation uses different combinations to prevent CSE */
    int t1 = v1 + v2 * 3;
    int t2 = v3 - v4 / 2;
    int t3 = v5 | v6 & 0xFF;
    int t4 = v7 ^ v8;
    int t5 = t1 * t2 + 1;
    int t6 = t3 - t4 * 2;
    int t7 = t5 | t6;
    int t8 = t1 ^ t2 & t3;
    int t9 = t4 + t5 - t6;
    int t10 = t7 * t8 / (t9 + 1);
    int t11 = t1 + t3 + t5 + t7 + t9;
    int t12 = t2 + t4 + t6 + t8 + t10;
    int t13 = t11 * t12;
    int t14 = t13 - t1;
    int t15 = t14 + t2;
    int t16 = t15 * t3;
    int t17 = t16 / (t4 + 1);
    int t18 = t17 | t5;
    int t19 = t18 ^ t6;
    int t20 = t19 + t7;
    int t21 = t20 - t8;
    int t22 = t21 * t9;
    int t23 = t22 / (t10 + 1);
    int t24 = t23 | t11;
    int t25 = t24 ^ t12;
    int t26 = t25 + t13;
    int t27 = t26 - t14;
    int t28 = t27 * t15;
    int t29 = t28 / (t16 + 1);
    int t30 = t29 | t17;
    
    /* Force all temporaries to be live simultaneously */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                         t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
    
    return result;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                     int index3, int index4)
{
    volatile int idx1 = index1;
    volatile int idx2 = index2;
    volatile int idx3 = index3;
    volatile int idx4 = index4;
    
    /* Force complex address calculations */
    
    /* 1. Double register indirect with large offset */
    int val1 = global_array[4096 + idx1 * 2 + idx2];
    
    /* 2. Nested array access with computation */
    int val2 = global_array[global_array[idx3] + idx4];
    
    /* 3. Large immediate offset */
    int val3 = global_array[8192];
    
    /* 4. Multi-dimensional style access */
    int val4 = global_array[idx1 * 100 + idx2 * 10 + idx3];
    
    /* 5. Mixed size accesses forcing different reload types */
    long long big_val = global_big_array[idx1 + 1000];
    double dbl_val = global_double_array[idx2 + 500];
    
    /* 6. Misaligned access simulation */
    char* byte_ptr = (char*)global_big_array;
    int unaligned_int = *(int*)(byte_ptr + idx3 * 8 + 1);  /* Potentially unaligned */
    
    /* Use all values to keep them live */
    return val1 + val2 + val3 + val4 + (int)big_val + (int)dbl_val + unaligned_int;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    volatile int x1 = a;
    volatile int x2 = b;
    volatile int x3 = c;
    volatile int x4 = d;
    
    /* Do some computation that uses registers */
    int y1 = x1 * x2 + 12345;
    int y2 = x3 / x4 - 6789;
    int y3 = y1 | y2;
    int y4 = y1 ^ y2;
    
    /* Clobber many registers - force spills and reloads */
    /* For x86_64, clobber both caller-saved and some callee-saved regs */
    asm volatile(
        "# Start of clobber block\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "# End of clobber block\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "cc", "memory"
    );
    
    /* More computation after clobber - forces reloads */
    int z1 = y3 + y4;
    int z2 = y1 - y2;
    int z3 = z1 * z2;
    int z4 = z3 / (x1 + 1);
    
    return z1 + z2 + z3 + z4;
}

/* Test 4: Many function arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int a = base;
    volatile int b = base + 1;
    volatile int c = base + 2;
    volatile int d = base + 3;
    volatile int e = base + 4;
    volatile int f = base + 5;
    volatile int g = base + 6;
    volatile int h = base + 7;
    volatile int i = base + 8;
    volatile int j = base + 9;
    volatile double k = base * 1.5;
    volatile double l = base * 2.5;
    volatile void* m = (void*)(intptr_t)(base + 100);
    volatile void* n = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register pressure for arg passing */
    int result = many_args_function(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
    
    /* Do more work to keep values live around the call */
    int extra1 = a + b + c;
    int extra2 = d + e + f;
    int extra3 = g + h + i;
    int extra4 = j + (int)k + (int)l;
    
    return result + extra1 + extra2 + extra3 + extra4;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int a, int b, int c)
{
    /* Use explicit register variables to force specific register allocation */
    register int r10_val asm("r10") = a * 3;
    register int r11_val asm("r11") = b * 5;
    
    /* Mixed type computations */
    long long ll1 = (long long)a * b * c;
    long long ll2 = ll1 + 0x123456789ABCDEFLL;
    double d1 = (double)a / (b + 1.0);
    double d2 = (double)c * 3.14159;
    
    /* Force use of explicit register variables */
    int sum1 = r10_val + r11_val;
    
    /* Structure copy forcing multi-register moves */
    struct two_ints {
        int x;
        int y;
    } s1 = {a, b}, s2;
    
    s2 = s1;  /* This may generate reloads for structure copy */
    
    /* Use all values */
    return sum1 + (int)ll2 + (int)d1 + (int)d2 + s2.x + s2.y;
}

/* Main orchestrator */
int main(int argc, char* argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Starting reload tests with base = %d\n", base);
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 5LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    int total = 0;
    
    /* Run all tests sequentially */
    total += test_register_pressure(base, base+1, base+2, base+3, 
                                   base+4, base+5, base+6, base+7);
    
    total += test_complex_addressing(base % 100, (base+1) % 100, 
                                    (base+2) % 100, (base+3) % 100);
    
    total += test_asm_clobber(base+10, base+11, base+12, base+13);
    
    total += test_many_args(base+20);
    
    total += test_mixed_types(base+30, base+31, base+32);
    
    printf("Total result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    return total == 0 ? 1 : 0;
}
