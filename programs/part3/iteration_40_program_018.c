/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
static int __attribute__((noinline, noclone))
helper_complex_calc(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a << 2);
    }
}

static double __attribute__((noinline, noclone))
helper_float_calc(double x, double y, int scale) {
    switch (scale & 3) {
        case 0: return x * y + 1.0;
        case 1: return x / y - 2.0;
        case 2: return x + y * 3.0;
        default: return y - x / 4.0;
    }
}

/* Main test function with high register pressure */
static volatile long __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a * 5;
    long f = b >> 1;
    float g = c + 7.0f;
    double h = d - 8.0;
    int i = e ^ 0x55;
    long j = f | 0xAA;
    float k = g * 1.5f;
    double l = h / 2.5;
    int m = i << 3;
    long n = j & 0xFF;
    float o = k - 10.0f;
    double p = l + 20.0;
    int q = m % 17;
    long r = n * 3;
    float s = o / 2.0f;
    double t = p * 3.0;
    
    /* Vector variables for wider register pressure */
    v4si v1 = {a, e, i, m};
    v4si v2 = {q, 2, 4, 8};
    v4sf v3 = {c, g, k, o};
    v4sf v4 = {1.0f, 2.0f, 3.0f, 4.0f};
    v2df v5 = {d, h};
    v2df v6 = {l, t};
    
    /* More scalar variables */
    int u = a + e + i;
    long v = b + f + j;
    float w = c * g * k;
    double x = d + h + l;
    int y = m ^ q ^ u;
    long z = n & r & v;
    float aa = o - s - w;
    double bb = p + t + x;
    int cc = helper_complex_calc(a, e, i);
    long dd = v * 2 + z;
    
    /* Long serial chain of interdependent operations */
    int t1 = a + b;
    long t2 = t1 * c;
    float t3 = t2 - d;
    double t4 = t3 * e;
    int t5 = t4 + f;
    long t6 = t5 ^ g;
    float t7 = t6 / h;
    double t8 = t7 - i;
    int t9 = t8 * j;
    long t10 = t9 | k;
    float t11 = t10 + l;
    double t12 = t11 * m;
    int t13 = t12 - n;
    long t14 = t13 & o;
    float t15 = t14 / p;
    double t16 = t15 + q;
    int t17 = t16 ^ r;
    long t18 = t17 | s;
    float t19 = t18 - t;
    double t20 = t19 * u;
    
    /* Vector operations */
    v4si v7 = v1 + v2;
    v4si v8 = v7 * v1;
    v4sf v9 = v3 + v4;
    v4sf v10 = v9 * v3;
    v2df v11 = v5 + v6;
    v2df v12 = v11 * v5;
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version */
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
    
    /* ARM version (commented out, choose based on target)
    asm volatile (
        "# Clobber many registers\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
          "memory"
    );
    */
    
    /* More computations after clobber */
    int t21 = t20 + v;
    long t22 = t21 * w;
    float t23 = t22 - x;
    double t24 = helper_float_calc(t23, bb, cc);
    int t25 = t24 + y;
    long t26 = t25 ^ z;
    float t27 = t26 * aa;
    double t28 = t27 - bb;
    
    /* Recompute a complex expression in slightly different form */
    /* This increases chances for early rematerialization */
    int recomputed1 = (a + b) * 3 - (c > 0 ? d : e);  /* Similar to t1 chain */
    long recomputed2 = recomputed1 * f + (g * 2);
    
    /* Use vector results */
    v4si v13 = v8 + v7;
    v4sf v14 = v10 - v9;
    v2df v15 = v12 / v11;
    
    /* Control flow to split basic blocks */
    if (recomputed1 > recomputed2) {
        t28 += v13[0] + v14[1] + v15[0];
    } else {
        t28 -= v13[2] * v14[3] / v15[1];
    }
    
    /* Switch statement for more control flow */
    switch (cc & 7) {
        case 0: t28 *= 1.1; break;
        case 1: t28 /= 1.2; break;
        case 2: t28 += 100.0; break;
        case 3: t28 -= 200.0; break;
        case 4: t28 = -t28; break;
        case 5: t28 = t28 * t28; break;
        case 6: t28 = helper_float_calc(t28, d, a); break;
        default: t28 = 0.0; break;
    }
    
    /* Final computation using all major temporaries */
    volatile long result = (long)(t1 + t5 + t9 + t13 + t17 + t21 + t25) +
                          (long)(t2 + t6 + t10 + t14 + t18 + t22 + t26) +
                          (long)(t3 + t7 + t11 + t15 + t19 + t23 + t27) +
                          (long)(t4 + t8 + t12 + t16 + t20 + t24 + t28) +
                          v13[0] + v13[1] + v13[2] + v13[3] +
                          (long)v14[0] + (long)v14[1] + (long)v14[2] + (long)v14[3] +
                          (long)v15[0] + (long)v15[1] +
                          recomputed1 + recomputed2;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        long result = test_remat(input1 + i, 
                                input2 - i * 2,
                                input3 * (1.0f + i * 0.01f),
                                input4 / (1.0 + i * 0.01));
        total += result;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(total));
    }
    
    printf("Final result: %ld\n", total);
    return (int)(total % 256);
}
