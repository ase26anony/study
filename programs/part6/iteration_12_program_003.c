/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile long long global_array[8192];
volatile int global_index = 2048;
volatile double global_double[4096];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 + f2 - f3);
    result += (intptr_t)p1 ^ (intptr_t)p2;
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
    
    /* Many independent computations creating register pressure */
    int t1 = a + b * c - d;
    int t2 = e ^ f | g & h;
    int t3 = i * j + k / (l ? l : 1);
    int t4 = m - n + o * p;
    int t5 = q ^ r | s & t;
    int t6 = t1 + t2 - t3;
    int t7 = t4 * t5 + t6;
    int t8 = a * c + e * g;
    int t9 = i * k + m * o;
    int t10 = q * s + b * d;
    int t11 = f * h + j * l;
    int t12 = n * p + r * t;
    int t13 = t7 + t8 - t9;
    int t14 = t10 * t11 + t12;
    int t15 = t13 ^ t14 | t7 & t8;
    int t16 = t9 + t10 - t11 * t12;
    int t17 = t13 / (t14 ? t14 : 1) + t15;
    int t18 = t16 ^ t17 | t13 & t14;
    int t19 = t15 * t16 + t17 / (t18 ? t18 : 1);
    int t20 = t18 - t19 + t13 * t14;
    
    /* Force all values to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
long long __attribute__((noinline)) test_complex_addressing(int idx) {
    volatile int complex_index;
    
    /* Force computation of index in memory */
    asm volatile("" : "=r"(complex_index) : "0"(idx));
    
    /* Large immediate offset requiring reload */
    long long val1 = global_array[4096];
    
    /* Variable index with computation */
    long long val2 = global_array[complex_index * 2 + 256];
    
    /* Nested array access with variable offset */
    long long val3 = global_array[global_index + complex_index];
    
    /* Misaligned 64-bit access (could require multiple moves on some archs) */
    uint64_t misaligned;
    char* ptr = (char*)global_array + 1; /* Misaligned pointer */
    asm volatile("" : "+r"(ptr));
    misaligned = *(uint64_t*)ptr; /* May require piecewise load */
    
    /* Double register indirect-like pattern */
    long long val4 = global_array[(complex_index + (int)(val1 & 0xFF))];
    
    return val1 + val2 + val3 + val4 + misaligned;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y) {
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Clobber many registers forcing spills */
    asm volatile(
        "# Clobber many registers\n\t"
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        : 
        : "r"(c), "r"(d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
    );
    
    /* More computations after clobber */
    int e = a * b + c * d;
    int f = (a << 3) | (b >> 2);
    int g = e ^ f;
    
    /* Another clobber */
    asm volatile(
        "# Another clobber\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2"
    );
    
    return g + e - f;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base) {
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
    void* p1 = (void*)(intptr_t)(base + 100);
    void* p2 = (void*)(intptr_t)(base + 200);
    
    /* Call forces register pressure for argument passing */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, f3, p1, p2);
    
    /* More computations after call */
    int t1 = a1 * a2 + a3 * a4;
    int t2 = a5 ^ a6 | a7 & a8;
    int t3 = a9 + a10 - (int)f1;
    
    return result + t1 + t2 + t3;
}

/* Test 5: Mixed types and register classes */
double __attribute__((noinline)) test_mixed_types(int n) {
    /* Mix int, float, double computations */
    volatile int vi = n;
    volatile float vf = n * 1.5f;
    volatile double vd = n * 2.5;
    volatile long long vl = n * 1000LL;
    
    /* Force use of different register classes */
    double d1 = vd + vi;
    double d2 = vf * vl;
    int i1 = vi + (int)vd;
    long long l1 = vl + (long long)vf;
    float f1 = vf + (float)vl;
    
    /* Complex expression mixing types */
    double result = d1 * d2 + i1 - l1 / 1000.0 + f1;
    
    /* Access misaligned double */
    char buffer[32];
    double* dp = (double*)(buffer + 3); /* Misaligned */
    *dp = result;
    double d3 = *dp; /* May require piecewise load/store */
    
    return result + d3;
}

int main(int argc, char* argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 8192; i++) {
        global_array[i] = i * 3LL;
    }
    for (int i = 0; i < 4096; i++) {
        global_double[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    total += test_complex_addressing(seed % 1000);
    total += test_asm_clobber(seed, seed * 2);
    total += test_many_args(seed);
    total += (int)test_mixed_types(seed % 100);
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
