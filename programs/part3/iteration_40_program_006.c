/* early_remat_test.c - Test case for GCC early rematerialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GCC vector extensions for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
static int __attribute__((noinline, noclone)) 
helper_compute(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

static double __attribute__((noinline, noclone))
helper_fp(double x, double y, int scale) {
    switch (scale & 3) {
        case 0: return x * y + 1.0;
        case 1: return x / y - 2.0;
        case 2: return x + y * 3.0;
        default: return y - x / 4.0;
    }
}

static v4si __attribute__((noinline, noclone))
helper_vector(v4si a, v4si b, v4si mask) {
    v4si result;
    if ((a[0] + b[0]) > 1000) {
        result = a + b * mask;
    } else {
        result = a - b & mask;
    }
    return result;
}

/* Main test function with high register pressure */
static volatile long __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Many local variables of mixed types */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a * 5;
    long f = b >> 1;
    float g = c + 10.5f;
    double h = d - 20.25;
    int i = e & 0xFF;
    long j = f | 0xFFFF;
    float k = g * 2.0f;
    double l = h / 3.0;
    int m = i ^ 0xAA;
    long n = j << 3;
    float o = k - 5.0f;
    double p = l + 7.0;
    int q = m + 100;
    long r = n - 200;
    float s = o * 1.5f;
    double t = p / 2.5;
    int u = q % 17;
    long v = r ^ 0x1234;
    float w = s + 3.14159f;
    double x = t - 2.71828;
    int y = u * 3;
    long z = v >> 2;
    float aa = w / 2.0f;
    double bb = x * 1.5;
    int cc = y | 0x55;
    long dd = z & 0xFF00;
    float ee = aa + bb;
    double ff = bb - aa;
    
    /* Vector variables */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {b & 0xFF, f & 0xFF, j & 0xFF, n & 0xFF};
    v4sf vec3 = {c, g, k, o};
    v4sf vec4 = {d, h, l, p};
    v2df vec5 = {t, x};
    v2df vec6 = {ff, ee};
    
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
    double t12 = t11 - m;
    int t13 = t12 * n;
    long t14 = t13 ^ o;
    float t15 = t14 / p;
    double t16 = t15 + q;
    int t17 = t16 - r;
    long t18 = t17 * s;
    float t19 = t18 + t;
    double t20 = t19 - u;
    int t21 = t20 * v;
    long t22 = t21 ^ w;
    float t23 = t22 / x;
    double t24 = t23 + y;
    int t25 = t24 - z;
    long t26 = t25 * aa;
    float t27 = t26 + bb;
    double t28 = t27 - cc;
    int t29 = t28 * dd;
    long t30 = t29 ^ ee;
    
    /* Vector operations mixed with scalars */
    v4si vec7 = vec1 + vec2;
    v4si vec8 = vec7 * (v4si){t1, t5, t9, t13};
    v4sf vec9 = vec3 + vec4;
    v4sf vec10 = vec9 * (v4sf){t3, t7, t11, t15};
    v2df vec11 = vec5 + vec6;
    v2df vec12 = vec11 * (v2df){t8, t16};
    
    /* Inline assembly to clobber physical registers */
    asm volatile (
#if defined(__x86_64__)
        "movq $0, %%rax\n\t"
        "movq $0, %%rbx\n\t"
        "movq $0, %%rcx\n\t"
        "movq $0, %%rdx\n\t"
        "movq $0, %%rsi\n\t"
        "movq $0, %%rdi\n\t"
        "movq $0, %%r8\n\t"
        "movq $0, %%r9\n\t"
        "movq $0, %%r10\n\t"
        "movq $0, %%r11\n\t"
        "movq $0, %%r12\n\t"
        "movq $0, %%r13\n\t"
        "movq $0, %%r14\n\t"
        "movq $0, %%r15\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
#elif defined(__arm__) || defined(__aarch64__)
        "mov x0, #0\n\t"
        "mov x1, #0\n\t"
        "mov x2, #0\n\t"
        "mov x3, #0\n\t"
        "mov x4, #0\n\t"
        "mov x5, #0\n\t"
        "mov x6, #0\n\t"
        "mov x7, #0\n\t"
        "mov x8, #0\n\t"
        "mov x9, #0\n\t"
        "mov x10, #0\n\t"
        "mov x11, #0\n\t"
        "mov x12, #0\n\t"
        "mov x13, #0\n\t"
        "mov x14, #0\n\t"
        "mov x15, #0\n\t"
        "eor v0.16b, v0.16b, v0.16b\n\t"
        "eor v1.16b, v1.16b, v1.16b\n\t"
        "eor v2.16b, v2.16b, v2.16b\n\t"
        "eor v3.16b, v3.16b, v3.16b\n\t"
        "eor v4.16b, v4.16b, v4.16b\n\t"
        "eor v5.16b, v5.16b, v5.16b\n\t"
        "eor v6.16b, v6.16b, v6.16b\n\t"
        "eor v7.16b, v7.16b, v7.16b\n\t"
        "eor v8.16b, v8.16b, v8.16b\n\t"
        "eor v9.16b, v9.16b, v9.16b\n\t"
        "eor v10.16b, v10.16b, v10.16b\n\t"
        "eor v11.16b, v11.16b, v11.16b\n\t"
        "eor v12.16b, v12.16b, v12.16b\n\t"
        "eor v13.16b, v13.16b, v13.16b\n\t"
        "eor v14.16b, v14.16b, v14.16b\n\t"
        "eor v15.16b, v15.16b, v15.16b\n\t"
        : /* no outputs */
        : /* no inputs */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "memory"
#else
        "#error Unsupported architecture"
#endif
    );
    
    /* Complex expression recomputed in different form (for remat) */
    int complex1 = (t1 * t5) + (t9 >> 2) - (t13 & 0xF);
    long complex2 = (t2 ^ t6) | (t10 << 1) + (t14 % 7);
    
    /* Use complex1 in multiple statements */
    int use1 = complex1 * 2;
    int use2 = complex1 + 100;
    int use3 = complex1 - 50;
    
    /* Recomputation in slightly different form */
    int complex1_remat = (t1 * t5) + (t9 >> 2) - (t13 & 0xF) + 1;
    
    /* More operations using recomputed value */
    long use4 = complex1_remat * 3L;
    float use5 = complex1_remat / 2.0f;
    
    /* Call helper functions to create basic blocks */
    int helper1 = helper_compute(use1, use2, use3);
    double helper2 = helper_fp(use5, d, helper1);
    v4si helper3 = helper_vector(vec7, vec8, (v4si){complex1, complex1_remat, use1, use2});
    
    /* More vector operations */
    v4si vec13 = vec8 + helper3;
    v4sf vec14 = vec10 * (v4sf){helper2, helper2/2, helper2*2, helper2+1};
    v2df vec15 = vec12 + (v2df){use4, use4 * 2.0};
    
    /* Final computation using all temporaries */
    volatile long result = 
        (t30 + use4) ^ 
        (helper1 * 1000) + 
        ((long)(helper2 * 10000)) + 
        vec13[0] + vec13[1] + vec13[2] + vec13[3] +
        (long)(vec14[0] + vec14[1] + vec14[2] + vec14[3]) +
        (long)(vec15[0] + vec15[1]);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 2 - i;
        volatile float in3 = (input_seed + i) * 0.5f;
        volatile double in4 = (input_seed - i) * 0.25;
        
        long result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Modify seed to change computation pattern */
        input_seed = (input_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
