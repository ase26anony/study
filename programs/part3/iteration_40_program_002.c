/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
__attribute__((noinline, noclone))
static int helper1(int a, int b, int c) {
    if (a > b) return a * c - b;
    return b * c + a;
}

__attribute__((noinline, noclone))
static float helper2(float x, float y, float z) {
    if (x < y) return x * y + z;
    return y * z - x;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 0, 1, 0};
    return (a + b) * mask;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 1.5f;
    double d = input4 / 2.0;
    int e = a * 3;
    long f = b + 1000;
    float g = c - 0.5f;
    double h = d * 3.14159;
    int i = e ^ 0x55;
    long j = f | 0xFF;
    float k = g * 2.0f;
    double l = h + 1.618;
    int m = i << 2;
    long n = j >> 1;
    float o = k / 3.0f;
    double p = l - 2.718;
    int q = m & 0x0F;
    long r = n % 17;
    float s = o + 4.2f;
    double t = p * 0.707;
    
    /* Vector variables */
    v4si v1 = {a, e, i, m};
    v4si v2 = {q, 2, 4, 8};
    v4sf v3 = {c, g, k, o};
    v4sf v4 = {1.1f, 2.2f, 3.3f, 4.4f};
    v2df v5 = {d, h};
    v2df v6 = {l, p};
    
    /* Additional scalars */
    int u = q * 2;
    long v = r + 500;
    float w = s * 0.9f;
    double x = t / 1.414;
    int y = u | 0xAA;
    long z = v ^ 0x5555;
    float aa = w + 3.14f;
    double bb = x - 1.0;
    int cc = y & 0x33;
    long dd = z * 3;
    float ee = aa * 2.0f;
    double ff = bb + 0.5;
    
    /* Long serial chain of interdependent operations */
    int t1 = a + e;
    long t2 = b * f;
    float t3 = c - g;
    double t4 = d + h;
    
    t1 = t1 * i;
    t2 = t2 + j;
    t3 = t3 * k;
    t4 = t4 - l;
    
    /* Complex expression computed and reused */
    int complex_expr = (t1 * m + q) / (u & 0x7F);
    long complex_expr2 = (t2 ^ n) | (r << 2);
    
    /* Use helper functions to create basic blocks */
    if (complex_expr > 100) {
        t1 = helper1(t1, complex_expr, m);
        v1 = helper3(v1, v2);
    } else {
        t1 = helper1(complex_expr, t1, q);
        v1 = v1 + v2;
    }
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile(
        "# Clobber many registers\n\t"
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
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    /* Continue computation after clobber */
    float t5 = helper2(t3, aa, ee);
    double t6 = t4 + ff;
    
    /* Recomputation of complex expression in slightly different form */
    int complex_expr_recomp = (t1 * u + cc) / ((y & 0x3F) | 1);
    long complex_expr2_recomp = (t2 ^ dd) | (v << 1);
    
    /* More vector operations */
    v3 = v3 * v4;
    v5 = v5 + v6;
    
    /* Switch statement for additional control flow */
    switch (complex_expr_recomp & 0x3) {
        case 0:
            t1 = t1 + complex_expr_recomp;
            t3 = t5 * 0.5f;
            break;
        case 1:
            t1 = t1 - complex_expr_recomp;
            t3 = t5 / 0.5f;
            break;
        case 2:
            t1 = t1 * complex_expr_recomp;
            t3 = t5 + 0.5f;
            break;
        default:
            t1 = complex_expr_recomp;
            t3 = t5 - 0.5f;
            break;
    }
    
    /* Final computation using all major temporaries */
    volatile long result = (long)t1 + t2 + (long)t3 + (long)t4 
                         + (long)t5 + (long)t6 + complex_expr2 
                         + complex_expr2_recomp + v1[0] + v1[1] 
                         + v1[2] + v1[3] + (long)v3[0] + (long)v3[1]
                         + (long)v5[0] + (long)v5[1];
    
    return result;
}

int main(int argc, char *argv[]) {
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
            input2 - i,
            input3 * (1.0f + i * 0.001f),
            input4 / (1.0 + i * 0.0001)
        );
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
