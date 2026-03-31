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
static int compute_partial(int a, int b, int c, int d) {
    if (a > b) {
        return (a * c) + (b * d);
    } else {
        return (a * d) - (b * c);
    }
}

/* Another helper with different control flow */
__attribute__((noinline, noclone))
static double vector_reduce(v4sf vec) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        if (vec[i] > 0.0f) {
            sum += vec[i];
        } else {
            sum -= vec[i] * 0.5;
        }
    }
    return sum;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2,
                                  volatile float input3, volatile double input4) {
    /* Declare at least 30 distinct local variables */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    
    int e = a * 5;
    long f = b / 6;
    float g = c + 7.0f;
    double h = d - 8.0;
    
    int i = e ^ 0xABCD;
    long j = f | 0x12345678;
    float k = g * 2.5f;
    double l = h / 1.5;
    
    int m = i << 3;
    long n = j >> 2;
    float o = k + 10.0f;
    double p = l * 0.75;
    
    int q = m % 17;
    long r = n & 0xFF00FF;
    float s = o - 3.14f;
    double t = p + 2.71828;
    
    int u = q * 11;
    long v = r / 9;
    float w = s * 4.0f;
    double x = t / 3.0;
    
    int y = u + 100;
    long z = v - 200;
    float aa = w + 50.0f;
    double bb = x - 25.0;
    
    int cc = y & z;
    long dd = v | u;
    float ee = aa * bb;
    double ff = bb / aa;
    
    int gg = cc ^ dd;
    long hh = dd << 1;
    float ii = ee + ff;
    double jj = ff - ee;
    
    /* Vector variables for additional pressure */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {q, u, y, cc};
    v4sf vecf1 = {c, g, k, o};
    v4sf vecf2 = {s, w, aa, ee};
    v2df vecd1 = {d, h};
    v2df vecd2 = {l, p};
    
    /* Long serial chain of interdependent operations */
    double t1 = a + b + c + d;
    double t2 = t1 * e * f;
    float t3 = t2 + g + h;
    long t4 = t3 * i * j;
    int t5 = t4 + k + l;
    double t6 = t5 * m * n;
    float t7 = t6 + o + p;
    long t8 = t7 * q * r;
    int t9 = t8 + s + t;
    double t10 = t9 * u * v;
    float t11 = t10 + w + x;
    long t12 = t11 * y * z;
    int t13 = t12 + aa + bb;
    double t14 = t13 * cc * dd;
    float t15 = t14 + ee + ff;
    long t16 = t15 * gg * hh;
    int t17 = t16 + ii + jj;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf1 * vecf2;
    v2df vecd3 = vecd1 + vecd2;
    v2df vecd4 = vecd1 * vecd2;
    
    /* Complex expression computed and used multiple times */
    double complex_expr = (t1 * t3 * t5 * t7 * t9 * t11 * t13 * t15 * t17) / 
                         (a * b * c * d * e * f * g * h);
    
    /* Use complex_expr in multiple statements */
    double use1 = complex_expr * 2.0;
    double use2 = complex_expr / 3.0;
    double use3 = complex_expr + 100.0;
    double use4 = complex_expr - 50.0;
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
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
        "pxor %%xmm15, %%xmm15"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* Control flow to split basic blocks */
    double result = 0.0;
    switch (a % 4) {
        case 0:
            result = use1 + use2 + compute_partial(a, e, i, m);
            break;
        case 1:
            result = use2 - use3 + compute_partial(q, u, y, cc);
            break;
        case 2:
            result = use3 * use4 + compute_partial(gg, hh, t17, t16);
            break;
        default:
            result = use4 / use1 + compute_partial(vec3[0], vec3[1], vec3[2], vec3[3]);
            break;
    }
    
    /* Recomputation of complex_expr in slightly different form */
    double complex_expr2 = (t2 * t4 * t6 * t8 * t10 * t12 * t14 * t16 * t17) /
                          (i * j * k * l * m * n * o * p);
    
    /* Vector reduction adds more computation */
    result += vector_reduce(vecf3);
    result += vector_reduce(vecf4);
    
    /* Final mixing of all values */
    result += complex_expr2;
    result += vec3[0] + vec3[1] + vec3[2] + vec3[3];
    result += vec4[0] * vec4[1] * vec4[2] * vec4[3];
    result += vecd3[0] + vecd3[1];
    result += vecd4[0] * vecd4[1];
    
    /* Ensure all temporaries contribute to result */
    result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    result += t10 + t11 + t12 + t13 + t14 + t15 + t16 + t17;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 6364136223846793005UL + 1);
        seed3 = seed3 * 1.01f + 0.5f;
        seed4 = seed4 * 1.001 + 0.25;
        
        total += test_remat(seed1, seed2, seed3, seed4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %f\n", total);
    return 0;
}
