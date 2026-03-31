/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
static int __attribute__((noinline, noclone))
complex_transform(int a, int b, int c) {
    if (a > b) {
        return (a * c) ^ (b << 3);
    } else {
        return (b * c) ^ (a << 2);
    }
}

static double __attribute__((noinline, noclone))
fp_transform(double x, double y, int scale) {
    switch (scale & 3) {
        case 0: return x * y + 1.0;
        case 1: return x / y - 2.0;
        case 2: return x + y * 3.0;
        default: return y - x / 4.0;
    }
}

/* Main test function with high register pressure */
static volatile int __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a ^ (int)b;
    long f = b | (long)a;
    float g = c + (float)d;
    double h = d - (double)c;
    int i = complex_transform(a, e, 5);
    long j = f * 7L;
    float k = g * 2.5f;
    double l = h / 1.5;
    int m = i << 2;
    long n = j >> 1;
    float o = k + 10.0f;
    double p = l - 20.0;
    int q = m ^ n;
    long r = n & m;
    float s = o * p;
    double t = p / o;
    int u = q * 3;
    long v = r / 2;
    float w = s + t;
    double x = t - s;
    int y = u | v;
    long z = v ^ u;
    float aa = w * 1.1f;
    double bb = x * 1.2;
    int cc = y & 0xFF;
    long dd = z | 0xFFFF;
    float ee = aa / 2.0f;
    double ff = bb * 2.0;
    
    /* Vector variables for additional pressure */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {b & 0xFF, f & 0xFF, j & 0xFF, n & 0xFF};
    v4sf vec3 = {c, g, k, o};
    v4sf vec4 = {(float)d, (float)h, (float)l, (float)p};
    v2df vec5 = {d, h};
    v2df vec6 = {l, p};
    
    /* Long serial chain of interdependent operations */
    int t1 = a + (int)b;
    long t2 = b * t1;
    float t3 = c + (float)t2;
    double t4 = d * t3;
    int t5 = t1 ^ (int)t4;
    long t6 = t2 | t5;
    float t7 = t3 * (float)t6;
    double t8 = t4 / (double)t7;
    int t9 = t5 + t6;
    long t10 = t6 - t9;
    float t11 = t7 + (float)t8;
    double t12 = t8 - (double)t11;
    
    /* Vector operations */
    vec1 = vec1 + vec2;
    vec2 = vec1 * vec2;
    vec3 = vec3 + vec4;
    vec4 = vec3 * vec4;
    vec5 = vec5 + vec6;
    vec6 = vec5 * vec6;
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* More computations after clobber */
    int t13 = t9 * 2;
    long t14 = t10 / 3;
    float t15 = t11 * 1.5f;
    double t16 = t12 / 2.5;
    
    /* Complex expression computed, used multiple times, then recomputed */
    int complex_val = (t13 * t5) + (t6 >> 3) ^ (t9 & 0xF);
    
    /* Use complex_val multiple times */
    int use1 = complex_val + t13;
    int use2 = complex_val - t14;
    float use3 = (float)complex_val * t15;
    double use4 = (double)complex_val / t16;
    
    /* Recompute similar expression later in same iteration */
    int complex_val2 = (t13 * t5) + (t6 >> 3) ^ (t9 & 0xF) + 1;  /* Slightly different */
    
    /* More vector operations */
    v4si vec7 = vec1 + (v4si){t13, t5, t9, complex_val};
    v4sf vec8 = vec3 * (v4sf){t15, t11, t7, t3};
    
    /* Control flow to split basic blocks */
    if (complex_val > complex_val2) {
        t13 = complex_transform(t13, complex_val, complex_val2);
        t14 = t13 * 7L;
    } else {
        t13 = complex_transform(complex_val2, complex_val, t13);
        t14 = t13 * 3L;
    }
    
    /* Switch statement for more control flow */
    switch (t13 & 7) {
        case 0: t15 = fp_transform(t15, t16, 0); break;
        case 1: t16 = fp_transform(t16, t15, 1); break;
        case 2: t15 = t15 * 2.0f; break;
        case 3: t16 = t16 / 2.0; break;
        case 4: t15 = t15 + (float)t16; break;
        case 5: t16 = t16 - (double)t15; break;
        default: t15 = t15 * t16; break;
    }
    
    /* Final combination of all values */
    volatile int result = 
        a + e + i + m + q + u + y + cc +
        (int)(b % 100) + (int)(f % 100) + (int)(j % 100) +
        (int)(c * 10) + (int)(g * 10) + (int)(k * 10) +
        (int)d + (int)h + (int)l +
        t1 + t5 + t9 + t13 +
        vec1[0] + vec2[1] + vec3[2] + vec4[3] +
        (int)vec5[0] + (int)vec6[1] +
        use1 + use2 + (int)use3 + (int)use4 +
        complex_val + complex_val2;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile long total = 0;
    
    /* Volatile inputs to prevent constant propagation */
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        input1 += (i & 1) ? 1 : -1;
        input2 += (i & 2) ? 10L : -10L;
        input3 *= (i % 3 == 0) ? 1.01f : 0.99f;
        input4 /= (i % 5 == 0) ? 1.02 : 0.98;
        
        total += test_remat(input1, input2, input3, input4);
    }
    
    printf("Result: %ld\n", total);
    return (int)(total % 1000);
}
