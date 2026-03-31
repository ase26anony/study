/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to split basic blocks */
__attribute__((noinline, noclone))
static int compute_complex(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

/* Helper function with control flow */
__attribute__((noinline, noclone))
static double vector_decision(v4si vec, double threshold) {
    int sum = vec[0] + vec[1] + vec[2] + vec[3];
    if (sum > threshold) {
        return sum * 1.5;
    } else {
        return sum * 0.75;
    }
}

/* Main test function implementing all requirements */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2,
                                volatile float input3, volatile double input4) {
    /* Requirement 1: Many local variables of mixed types */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a * 5;
    long f = b + 6;
    float g = c - 7.0f;
    double h = d * 8.0;
    int i = e >> 1;
    long j = f << 2;
    float k = g * 9.0f;
    double l = h / 10.0;
    int m = i | 0xFF;
    long n = j & 0xFFFF;
    float o = k + 11.0f;
    double p = l - 12.0;
    int q = m ^ 0xAA;
    long r = n | 0xBB;
    float s = o * 13.0f;
    double t = p / 14.0;
    int u = q << 3;
    long v = r >> 2;
    float w = s + 15.0f;
    double x = t * 16.0;
    int y = u % 17;
    long z = v + 18;
    float aa = w - 19.0f;
    double bb = x / 20.0;
    int cc = y & z;
    long dd = z | y;
    float ee = aa * bb;
    double ff = bb + aa;
    
    /* Vector variables for Requirement 4 */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {b & 0xFF, f & 0xFF, j & 0xFF, n & 0xFF};
    v4sf vecf1 = {c, g, k, o};
    v4sf vecf2 = {d, h, l, p};
    v2df vecd1 = {x, t};
    v2df vecd2 = {bb, ff};
    
    /* Requirement 2: Complex interdependent computations */
    int t1 = a + b;
    long t2 = t1 * c;
    float t3 = t2 - d;
    double t4 = t3 * e;
    int t5 = t4 + f;
    long t6 = t5 * g;
    float t7 = t6 - h;
    double t8 = t7 * i;
    int t9 = t8 + j;
    long t10 = t9 * k;
    float t11 = t10 - l;
    double t12 = t11 * m;
    int t13 = t12 + n;
    long t14 = t13 * o;
    float t15 = t14 - p;
    double t16 = t15 * q;
    int t17 = t16 + r;
    long t18 = t17 * s;
    float t19 = t18 - t;
    double t20 = t19 * u;
    int t21 = t20 + v;
    long t22 = t21 * w;
    float t23 = t22 - x;
    double t24 = t23 * y;
    int t25 = t24 + z;
    long t26 = t25 * aa;
    float t27 = t26 - bb;
    double t28 = t27 * cc;
    int t29 = t28 + dd;
    long t30 = t29 * ee;
    
    /* Vector operations mixed with scalars */
    vec1 = vec1 + vec2;
    vecf1 = vecf1 * vecf2;
    vecd1 = vecd1 - vecd2;
    
    /* Requirement 5: Control flow splitting */
    if (t30 > 1000) {
        t1 = compute_complex(t1, t5, t9);
        vec1 = vec1 << 1;
    } else {
        t1 = compute_complex(t13, t17, t21);
        vec1 = vec1 >> 1;
    }
    
    switch (t1 % 4) {
        case 0:
            t2 = t2 * 2;
            vecf1 = vecf1 + vecf2;
            break;
        case 1:
            t2 = t2 / 2;
            vecf1 = vecf1 - vecf2;
            break;
        case 2:
            t2 = t2 << 1;
            vecf1 = vecf1 * 2.0f;
            break;
        default:
            t2 = t2 >> 1;
            vecf1 = vecf1 / 2.0f;
            break;
    }
    
    /* Requirement 3: Inline assembly clobbering registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* ARM64 version (commented out, use one or the other)
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "nop"
        : 
        : 
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
          "x24", "x25", "x26", "x27", "x28",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
          "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
          "memory"
    );
    */
    
    /* More computations after assembly clobber */
    double t31 = vector_decision(vec1, t30);
    float t32 = vecf1[0] + vecf1[1] + vecf1[2] + vecf1[3];
    double t33 = vecd1[0] * vecd1[1];
    
    /* Recomputation of earlier values (Requirement 2) */
    int t1_recomp = a + b + (t1 % 100);  /* Slightly different recomputation */
    long t2_recomp = t1_recomp * c * 2;
    
    /* Final complex expression using many temporaries */
    volatile long result = 
        (t1 + t2 + t5 + t9 + t13 + t17 + t21 + t25 + t29) +
        (t1_recomp * t2_recomp) +
        (long)(t3 + t7 + t11 + t15 + t19 + t23 + t27) +
        (long)(t31 + t32 + t33) +
        vec1[0] + vec1[1] + vec1[2] + vec1[3] +
        (long)(ee * ff);
    
    return result;
}

int main(int argc, char **argv) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile long result = test_remat(
            input1 + (i % 10),
            input2 - (i % 20),
            input3 * (1.0f + (i % 5) * 0.1f),
            input4 / (1.0 + (i % 3) * 0.05)
        );
        total += result;
        
        /* Additional control flow in main loop */
        if (i % 100 == 0) {
            total = total / 2;
        }
    }
    
    printf("Final result: %ld\n", total);
    return 0;
}
