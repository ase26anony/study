/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to split basic blocks */
static int __attribute__((noinline, noclone)) 
helper_compute(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

/* Another helper with different control flow */
static double __attribute__((noinline, noclone))
helper_fp_compute(double x, double y, int choice) {
    switch (choice & 3) {
        case 0: return x * y + x - y;
        case 1: return x / (y + 1.0) * 2.0;
        case 2: return y * y - x * x;
        default: return (x + y) * (x - y);
    }
}

/* Main test function with high register pressure */
static volatile long __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.14f;
    double d = input4 / 2.71828;
    
    int e = a * 2;
    long f = b >> 1;
    float g = c + 1.0f;
    double h = d - 0.5;
    
    int i = e | 0xFF;
    long j = f & 0xFFFF;
    float k = g * 2.0f;
    double l = h / 3.0;
    
    int m = i ^ 0xAA;
    long n = j | 0x1234;
    float o = k - 1.5f;
    double p = l * 4.0;
    
    int q = m << 2;
    long r = n >> 4;
    float s = o / 2.0f;
    double t = p + 1.0;
    
    int u = q % 17;
    long v = r * 3;
    float w = s * s;
    double x = t * t;
    
    int y = u + 100;
    long z = v - 50;
    float aa = w + c;
    double bb = x - d;
    
    int cc = y & z;
    long dd = z | y;
    float ee = aa * g;
    double ff = bb / h;
    
    int gg = helper_compute(cc, dd & 0xFF, e);
    long hh = dd * ee;
    float ii = ee + ff;
    double jj = ff * gg;
    
    /* Vector variables for additional pressure */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {q, u, cc, gg};
    v4sf vecf1 = {c, g, k, o};
    v4sf vecf2 = {s, aa, ee, ii};
    v2df vecd1 = {d, h};
    v2df vecd2 = {l, p};
    
    /* Long serial chain of interdependent operations */
    int t1 = a + b;
    long t2 = t1 * c;
    float t3 = t2 - d;
    double t4 = t3 * e;
    int t5 = t4 + f;
    long t6 = t5 ^ g;
    float t7 = t6 / h;
    double t8 = t7 * i;
    int t9 = t8 - j;
    long t10 = t9 | k;
    float t11 = t10 + l;
    double t12 = t11 * m;
    int t13 = t12 - n;
    long t14 = t13 & o;
    float t15 = t14 / p;
    double t16 = t15 + q;
    int t17 = t16 * r;
    long t18 = t17 ^ s;
    float t19 = t18 - t;
    double t20 = t19 * u;
    int t21 = t20 + v;
    long t22 = t21 | w;
    float t23 = t22 / x;
    double t24 = t23 * y;
    int t25 = t24 - z;
    long t26 = t25 & aa;
    float t27 = t26 + bb;
    double t28 = t27 * cc;
    int t29 = t28 - dd;
    long t30 = t29 | ee;
    
    /* Vector operations mixed with scalar */
    vec1 = vec1 + vec2;
    vecf1 = vecf1 * vecf2;
    vecd1 = vecd1 - vecd2;
    
    /* Inline assembly to clobber physical registers */
    asm volatile (
        "# Clobber many registers\n\t"
        :
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Complex expression computed, used multiple times, then recomputed */
    int complex1 = (t1 * t2) + (t3 / t4) - (t5 ^ t6) | (t7 & t8);
    float complex2 = (t9 * t10) / (t11 + t12) - (t13 - t14) + (t15 * t16);
    
    /* Use complex1 in multiple statements */
    int use1 = complex1 * 2;
    int use2 = complex1 + t17;
    int use3 = complex1 ^ t18;
    float use4 = complex1 * complex2;
    
    /* Recomputation in slightly different form */
    int complex1_alt = (t1 * t2) + (t3 / t4) - (t5 ^ t6) | (t7 & t8) | 0x1;
    float complex2_alt = (t9 * t10) / (t11 + t12) - (t13 - t14) + (t15 * t16) + 1.0f;
    
    /* More operations with vectors */
    v4si vec3 = vec1 * complex1;
    v4sf vec4 = vecf1 + complex2;
    
    /* Control flow to split basic blocks */
    if (complex1 > complex1_alt) {
        t30 = helper_complex(complex1, complex1_alt, t19);
        vec3 = vec3 + (v4si){1, 2, 3, 4};
    } else {
        t30 = helper_complex(complex1_alt, complex1, t20);
        vec4 = vec4 * (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
    }
    
    /* Another basic block split */
    for (int iter = 0; iter < 3; iter++) {
        double temp = helper_fp_compute(t23, t24, iter);
        if (iter == 1) {
            t28 += temp;
            vecd1 = vecd1 * (v2df){temp, temp/2};
        } else {
            t29 -= temp;
            vecd2 = vecd2 + (v2df){temp, temp*2};
        }
    }
    
    /* Final computation using all major temporaries */
    volatile long result = 
        (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
         t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30 +
         vec1[0] + vec1[1] + vec1[2] + vec1[3] +
         vecf1[0] + vecf1[1] + vecf1[2] + vecf1[3] +
         vecd1[0] + vecd1[1] +
         complex1 + complex1_alt + use1 + use2 + use3 + use4 +
         complex2 + complex2_alt);
    
    return result;
}

/* Additional helper for control flow */
static long __attribute__((noinline, noclone))
helper_complex(int x, int y, float z) {
    if (z > 0) {
        return (long)x * y + (long)(z * 100);
    } else {
        return (long)x / (y + 1) - (long)(z * 50);
    }
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        long result = test_remat(
            input1 + (i % 7),
            input2 - (i % 5),
            input3 * (1.0f + i * 0.01f),
            input4 / (1.0 + i * 0.005)
        );
        total += result;
        
        /* Additional computation to prevent loop optimization */
        if (i % 13 == 0) {
            total ^= result;
        }
    }
    
    printf("Final result: %ld\n", total);
    return 0;
}
