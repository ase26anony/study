/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
__attribute__((noinline, noclone))
static int compute_complex(int a, int b, int c, int d) {
    if (a > b) {
        return (a * c) - (b * d);
    } else {
        return (b * d) - (a * c);
    }
}

__attribute__((noinline, noclone))
static double fp_complex(double x, double y, double z) {
    switch ((int)x % 3) {
        case 0: return (x * y) + z;
        case 1: return (x + y) * z;
        case 2: return (x - y) / (z + 1.0);
        default: return x;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2,
                                  volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 1;
    float c = input3 * 2.0f;
    double d = input4 / 2.0;
    
    int e = a * 3;
    long f = b / 2;
    float g = c + 1.5f;
    double h = d - 0.5;
    
    int i = e ^ 0x55AA;
    long j = f | 0xFFFF;
    float k = g * 3.14159f;
    double l = h / 2.71828;
    
    int m = i << 2;
    long n = j >> 1;
    float o = k + k;
    double p = l * l;
    
    int q = m % 17;
    long r = n & 0xFF00FF;
    float s = o - 1.0f;
    double t = p + 1.0;
    
    int u = q * q;
    long v = r + r;
    float w = s / 2.0f;
    double x = t * 0.5;
    
    int y = u | v;
    long z = v ^ u;
    float aa = w * w;
    double bb = x + x;
    
    /* Vector variables for additional pressure */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {b & 0xFF, f & 0xFF, j & 0xFF, n & 0xFF};
    v4sf vec3 = {c, g, k, o};
    v4sf vec4 = {d, h, l, p};
    v2df vec5 = {t, bb};
    v2df vec6 = {x, bb/2};
    
    /* Long serial chain of interdependent operations */
    double t1 = a + b + c + d;
    double t2 = t1 * e * f;
    float t3 = t2 / g + h;
    long t4 = (long)(t3) * i * j;
    int t5 = t4 % m + n;
    double t6 = t5 * o * p;
    
    /* Complex expression that will be reused */
    double complex_expr = (t1 * t2) / (t3 + 1.0) + (t4 % 1000) - (t5 * t6);
    
    /* First use of complex_expr */
    double result1 = complex_expr * 2.0;
    double result2 = complex_expr / 3.0;
    double result3 = complex_expr + complex_expr;
    
    /* Inline assembly to clobber registers */
    asm volatile (
#if defined(__x86_64__)
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
        "pxor %%xmm0, %%xmm0\n"
        "pxor %%xmm1, %%xmm1\n"
        "pxor %%xmm2, %%xmm2\n"
        "pxor %%xmm3, %%xmm3\n"
        "pxor %%xmm4, %%xmm4\n"
        "pxor %%xmm5, %%xmm5\n"
        "pxor %%xmm6, %%xmm6\n"
        "pxor %%xmm7, %%xmm7\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
#elif defined(__arm__) || defined(__aarch64__)
        "mov x0, #0\n"
        "mov x1, #0\n"
        "mov x2, #0\n"
        "mov x3, #0\n"
        "mov x4, #0\n"
        "mov x5, #0\n"
        "mov x6, #0\n"
        "mov x7, #0\n"
        "mov x8, #0\n"
        "mov x9, #0\n"
        "mov x10, #0\n"
        "mov x11, #0\n"
        "mov x12, #0\n"
        "mov x13, #0\n"
        "mov x14, #0\n"
        "mov x15, #0\n"
        "eor v0.16b, v0.16b, v0.16b\n"
        "eor v1.16b, v1.16b, v1.16b\n"
        "eor v2.16b, v2.16b, v2.16b\n"
        "eor v3.16b, v3.16b, v3.16b\n"
        "eor v4.16b, v4.16b, v4.16b\n"
        "eor v5.16b, v5.16b, v5.16b\n"
        "eor v6.16b, v6.16b, v6.16b\n"
        "eor v7.16b, v7.16b, v7.16b\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "memory"
#else
        "nop\n"
        :
        :
        : "memory"
#endif
    );
    
    /* Vector operations */
    v4si vec7 = vec1 + vec2;
    v4si vec8 = vec1 * vec2;
    v4sf vec9 = vec3 + vec4;
    v4sf vec10 = vec3 * vec4;
    v2df vec11 = vec5 + vec6;
    v2df vec12 = vec5 * vec6;
    
    /* Recomputation of complex_expr in slightly different form */
    double complex_expr2 = (t1 * t2) / (t3 + 1.0) + (t4 % 1000) - (t5 * t6) + 1.0;
    
    /* More operations using both original and recomputed values */
    double t7 = result1 + result2 + result3;
    double t8 = complex_expr2 * t7;
    double t9 = complex_expr / complex_expr2;
    
    /* Control flow to split basic blocks */
    if (t8 > t9) {
        t8 = compute_complex(a, q, u, y);
    } else {
        t9 = fp_complex(t8, t9, complex_expr);
    }
    
    switch ((int)t8 % 4) {
        case 0:
            t8 += vec7[0] + vec9[0] + vec11[0];
            break;
        case 1:
            t9 += vec8[1] + vec10[1] + vec12[1];
            break;
        case 2:
            t8 *= 1.5;
            t9 /= 1.5;
            break;
        case 3:
            t8 = t9 * 0.75;
            break;
    }
    
    /* Final computation using all major temporaries */
    volatile double final_result = 
        t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 +
        complex_expr + complex_expr2 +
        vec7[0] + vec8[1] + vec9[2] + vec10[3] +
        vec11[0] + vec12[1] +
        a + b + c + d + e + f + g + h +
        i + j + k + l + m + n + o + p +
        q + r + s + t + u + v + w + x +
        y + z + aa + bb;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 6364136223846793005UL + 1) & 0x7FFFFFFFFFFFFFFFUL;
        seed3 = seed3 * 1.01f + 0.5f;
        seed4 = seed4 * 1.001 + 0.1;
        
        double result = test_remat(seed1, seed2, seed3, seed4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i) : : "memory");
    }
    
    printf("Final result: %f\n", (double)total);
    return 0;
}
