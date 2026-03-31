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
    } else {
        return b * c + a;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    return v[0] + v[1];
}

/* Main test function with high register pressure */
__attribute__((noinline))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare at least 30 distinct local variables */
    int a = input1;
    long b = input2;
    float c = input3;
    double d = input4;
    
    /* More integer variables */
    int e = a * 2;
    int f = b & 0xFF;
    int g = a + f;
    int h = e - g;
    int i = h * 3;
    int j = i | 0x7F;
    int k = j ^ f;
    int l = k << 2;
    int m = l >> 1;
    int n = m % 17;
    int o = n + a;
    int p = o * b;
    
    /* More floating point variables */
    float q = c * 2.5f;
    float r = q + c;
    float s = r - 1.0f;
    float t = s * c;
    float u = t / 3.14f;
    
    double v = d * 1.618;
    double w = v + d;
    double x = w - 2.718;
    double y = x * d;
    double z = y / 1.414;
    
    /* Vector variables */
    v4si vec1 = {a, e, g, i};
    v4si vec2 = {f, h, j, k};
    v4sf vec3 = {c, q, r, s};
    v4sf vec4 = {t, u, 2.0f, 3.0f};
    v2df vec5 = {d, v};
    v2df vec6 = {w, x};
    
    /* Long serial chain of interdependent operations */
    int t1 = a + b;
    long t2 = t1 * e;
    float t3 = c + t2;
    double t4 = d * t3;
    int t5 = t1 ^ t2;
    float t6 = t3 * q;
    double t7 = t4 + w;
    int t8 = t5 & m;
    long t9 = t2 | t8;
    float t10 = t6 - u;
    double t11 = t7 * z;
    
    /* Complex expression computed and used multiple times */
    int complex_expr = (a * b + e * f - g * h) | (i * j ^ k * l);
    float complex_float = (c * q - r * s) / (t + u);
    double complex_double = (d * v + w * x) - (y * z);
    
    /* Use complex expression multiple times */
    int use1 = complex_expr + t1;
    int use2 = complex_expr * t5;
    int use3 = complex_expr & t8;
    
    float use4 = complex_float + t3;
    float use5 = complex_float * t6;
    
    double use6 = complex_double + t4;
    double use7 = complex_double - t11;
    
    /* Vector operations */
    v4si vec7 = vec1 + vec2;
    v4si vec8 = vec7 * vec1;
    v4sf vec9 = vec3 * vec4;
    v2df vec10 = vec5 + vec6;
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n"
        :
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Control flow to create multiple basic blocks */
    int branch_result;
    if (complex_expr > 1000) {
        branch_result = compute_branch(a, e, g);
        
        /* Recomputation of complex expression in different form */
        int recomputed_expr = (a * b) | (e * f) ^ (g * h) & (i * j);
        use1 = recomputed_expr + t1;
        
        /* More operations in this branch */
        vec7 = vec7 + (v4si){recomputed_expr, 0, 0, 0};
        vec9 = vec9 * (v4sf){complex_float, 0, 0, 0};
    } else {
        branch_result = compute_branch(e, a, f);
        
        /* Different recomputation */
        int recomputed_expr = (a + b) * (e - f) / (g + 1);
        use2 = recomputed_expr * t5;
        
        /* Different vector operations */
        vec8 = vec8 - (v4si){recomputed_expr, 0, 0, 0};
        vec10 = vec10 * (v2df){complex_double, 0};
    }
    
    /* Switch statement for more control flow */
    switch (branch_result % 4) {
        case 0:
            t1 = t1 * 2;
            complex_expr = complex_expr >> 1;
            break;
        case 1:
            t2 = t2 / 3;
            complex_float = complex_float + 1.0f;
            break;
        case 2:
            t3 = t3 * 1.5f;
            complex_double = complex_double - 0.5;
            break;
        case 3:
            t4 = t4 * 0.8;
            /* Another recomputation */
            complex_expr = (a ^ b) & (e | f) * (g ^ h);
            break;
    }
    
    /* More operations after control flow */
    int final1 = use1 + use2 + use3 + branch_result;
    float final2 = use4 * use5 + t10;
    double final3 = use6 - use7 + vector_reduce(vec10);
    
    /* Final complex expression that uses many values */
    volatile int result = final1 + (int)final2 + (int)final3 +
                         vec7[0] + vec8[1] + (int)vec9[2] +
                         (int)(complex_expr % 100);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    
    /* Volatile inputs to prevent constant propagation */
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Modify inputs slightly each iteration */
        input1 = (input1 * 1103515245 + 12345) & 0x7FFFFFFF;
        input2 = input2 * 6364136223846793005L + 1442695040888963407L;
        input3 = input3 * 1.01f + 0.5f;
        input4 = input4 * 1.001 + 0.1;
        
        int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
