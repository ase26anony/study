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
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c - b;
    } else if (a < b) {
        return b * c + a;
    } else {
        return (a + b) * c;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v4sf v) {
    double sum = v[0] + v[1] + v[2] + v[3];
    return sum * 0.25;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.14f;
    double d = input4 / 2.71828;
    
    int e = a * 2;
    long f = b / 3;
    float g = c + 1.0f;
    double h = d - 0.5;
    
    int i = e ^ 0xFF;
    long j = f | 0xFFFF;
    float k = g * 2.0f;
    double l = h / 3.0;
    
    int m = i << 2;
    long n = j >> 1;
    float o = k + 3.14f;
    double p = l - 1.618;
    
    int q = m & 0x0F0F;
    long r = n ^ 0xAAAA;
    float s = o / 2.0f;
    double t = p * 1.5;
    
    int u = q + 100;
    long v = r - 200;
    float w = s * 1.1f;
    double x = t / 1.1;
    
    int y = u % 17;
    long z = v % 31;
    float aa = w + 2.0f;
    double bb = x - 2.0;
    
    int cc = y | z;
    long dd = z & y;
    float ee = aa * bb;
    double ff = bb / aa;
    
    int gg = cc ^ dd;
    long hh = dd | cc;
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
    int t1 = a + b;
    long t2 = t1 * f;
    float t3 = c + g;
    double t4 = d - h;
    
    int t5 = t1 ^ e;
    long t6 = t2 | j;
    float t7 = t3 * k;
    double t8 = t4 / l;
    
    int t9 = t5 + m;
    long t10 = t6 - n;
    float t11 = t7 + o;
    double t12 = t8 - p;
    
    /* Complex expression computed and used multiple times */
    int complex_expr = (t1 * t5 + t9) / (t1 + 1);
    long complex_expr2 = (t2 ^ t6) | (t10 & 0xFF);
    
    /* Use complex expression multiple times */
    int use1 = complex_expr + t9;
    long use2 = complex_expr2 * t10;
    int use3 = complex_expr - t5;
    long use4 = complex_expr2 ^ t6;
    
    /* Inline assembly to clobber physical registers */
    asm volatile (
#if defined(__x86_64__)
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
#elif defined(__arm__) || defined(__aarch64__)
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "memory"
#else
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "memory"
#endif
    );
    
    /* Control flow to create multiple basic blocks */
    int branch_result;
    if (complex_expr > 1000) {
        branch_result = compute_branch(use1, use3, t1);
        
        /* Recomputation of similar expression */
        int complex_expr_recomp = (t1 * t5 + t9) / (t1 + 2);  /* Slightly different */
        vec1 += (v4si){complex_expr_recomp, 0, 0, 0};
    } else if (complex_expr < 500) {
        branch_result = compute_branch(use3, use1, t5);
        
        /* Another recomputation */
        int complex_expr_recomp = (t1 * t5 + t9) / (t1 + 3);  /* Slightly different */
        vec2 += (v4si){0, complex_expr_recomp, 0, 0};
    } else {
        branch_result = compute_branch(t1, t5, t9);
        
        /* Yet another recomputation */
        int complex_expr_recomp = (t1 * t5 + t9) / (t1 + 4);  /* Slightly different */
        vec1 *= (v4si){complex_expr_recomp, 1, 1, 1};
    }
    
    /* More operations after control flow */
    float t13 = t11 * aa;
    double t14 = t12 + bb;
    
    int t15 = use1 ^ branch_result;
    long t16 = use2 | branch_result;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4sf vecf3 = vecf1 * vecf2;
    v2df vecd3 = vecd1 - vecd2;
    
    /* Reduce vectors */
    double vec_sum = vector_reduce(vecf3);
    
    /* Switch statement for additional basic blocks */
    int final_result;
    switch (t15 & 0x7) {
        case 0:
            final_result = t1 + t5 + t9;
            break;
        case 1:
            final_result = t5 - t9 + t15;
            break;
        case 2:
            final_result = t9 * t15 / (t1 + 1);
            break;
        case 3:
            final_result = (t1 ^ t5) | t9;
            break;
        case 4:
            final_result = t15 + branch_result;
            break;
        case 5:
            final_result = t1 * 2 - t5;
            break;
        case 6:
            final_result = t9 + t15 * 3;
            break;
        default:
            final_result = t1 + t5 + t9 + t15;
            break;
    }
    
    /* Final computation using many variables to keep them live */
    volatile int result = final_result 
        + (int)t2 + (int)t6 + (int)t10 + (int)t16
        + (int)t3 + (int)t7 + (int)t11 + (int)t13
        + (int)vec_sum + (int)vecd3[0] + (int)vecd3[1]
        + vec3[0] + vec3[1] + vec3[2] + vec3[3];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long accumulator = 0;
    
    /* Volatile inputs to prevent constant propagation */
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Modify inputs slightly each iteration */
        volatile int result = test_remat(
            input1 + (i % 7),
            input2 - (i % 11),
            input3 * (1.0f + (i % 5) * 0.1f),
            input4 / (1.0 + (i % 3) * 0.05)
        );
        
        accumulator += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", accumulator);
    return 0;
}
