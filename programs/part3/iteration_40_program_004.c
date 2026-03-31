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
        return a * c + b;
    } else if (a < b) {
        return b * c - a;
    } else {
        return (a + b) * c;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    double sum = v[0] + v[1];
    if (sum > 100.0) sum *= 0.5;
    else if (sum < -100.0) sum *= -0.5;
    return sum;
}

/* Main test function with high register pressure */
__attribute__((noinline))
static volatile double test_remat(volatile int loop_count) {
    /* Declare many local variables - at least 30 */
    volatile int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    volatile long b1 = 10, b2 = 20, b3 = 30, b4 = 40, b5 = 50;
    volatile float c1 = 1.1f, c2 = 2.2f, c3 = 3.3f, c4 = 4.4f, c5 = 5.5f;
    volatile double d1 = 10.1, d2 = 20.2, d3 = 30.3, d4 = 40.4, d5 = 50.5;
    
    /* Additional variables for more pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long u1, u2, u3, u4, u5;
    float v1, v2, v3, v4, v5;
    double w1, w2, w3, w4, w5;
    
    /* Vector variables */
    v4si vec1 = {1, 2, 3, 4}, vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f}, fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {10.0, 20.0}, dvec2 = {30.0, 40.0};
    
    /* Long serial chain of interdependent operations */
    t1 = a1 + a2 * a3 - a4;
    t2 = t1 * a5 + b1 % 7;
    t3 = t2 - b2 / 3 + t1;
    t4 = t3 * 2 + b3 - t2;
    t5 = t4 / 2 + b4 * t3;
    
    u1 = b1 * t1 + b2;
    u2 = u1 - b3 * t2 + b4;
    u3 = u2 / 2 + b5 * t3 - u1;
    u4 = u3 * 3 - t4 + u2;
    u5 = u4 + t5 * u3 - b5;
    
    v1 = c1 * t1 + c2;
    v2 = v1 - c3 * t2 + c4;
    v3 = v2 / 2.0f + c5 * t3 - v1;
    v4 = v3 * 3.0f - t4 + v2;
    v5 = v4 + t5 * v3 - c5;
    
    w1 = d1 * u1 + d2;
    w2 = w1 - d3 * u2 + d4;
    w3 = w2 / 2.0 + d5 * u3 - w1;
    w4 = w3 * 3.0 - u4 + w2;
    w5 = w4 + u5 * w3 - d5;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2 * 2;
    v4si vec4 = vec3 - vec1 + vec2;
    v4sf fvec3 = fvec1 * 1.5f + fvec2;
    v4sf fvec4 = fvec3 - fvec1 * 0.5f;
    v2df dvec3 = dvec1 * 2.0 + dvec2;
    v2df dvec4 = dvec3 - dvec1 + dvec2;
    
    /* Control flow to create multiple basic blocks */
    if (t1 > t2) {
        t6 = compute_branch(t1, t2, t3);
        vec3 = vec3 + (v4si){t6, t6, t6, t6};
    } else {
        t6 = compute_branch(t2, t1, t3);
        vec3 = vec3 - (v4si){t6, t6, t6, t6};
    }
    
    switch (t3 % 4) {
        case 0:
            t7 = t4 * 2 + t5;
            fvec3 = fvec3 * 2.0f;
            break;
        case 1:
            t7 = t4 / 2 + t5;
            fvec3 = fvec3 / 2.0f;
            break;
        case 2:
            t7 = t4 - t5;
            fvec3 = fvec3 + fvec4;
            break;
        default:
            t7 = t4 + t5 * 3;
            fvec3 = fvec3 - fvec4;
            break;
    }
    
    /* Inline assembly to clobber registers */
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
          "x16", "x17", "x18", "x19", "x20", "x21", "x22",
          "x23", "x24", "x25", "x26", "x27", "x28", "memory"
#else
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "memory"
#endif
    );
    
    /* Loop with recomputation pattern to trigger early remat */
    volatile int i;
    volatile double accumulator = 0.0;
    
    for (i = 0; i < loop_count; i++) {
        /* Complex expression computed and used multiple times */
        int complex_val = (t1 * t2 + t3 - t4) * (i % 10 + 1);
        complex_val = complex_val + (t5 << 2) - (t6 >> 1);
        
        /* Use complex_val in multiple statements */
        t8 = complex_val * 2 + t7;
        t9 = complex_val / 3 - t8;
        u1 = complex_val + b1 * 2;
        
        /* Recomputation in slightly different form */
        int complex_val2 = (t1 * t2 + t3 - t4) * (i % 10 + 2);  /* Slightly different multiplier */
        complex_val2 = complex_val2 + (t5 << 3) - (t6 >> 2);    /* Different shifts */
        
        /* Use the recomputed value */
        t10 = complex_val2 * 3 - t9;
        u2 = complex_val2 - b2 / 2;
        
        /* More vector operations in loop */
        vec4 = vec3 + vec4 * (i + 1);
        fvec4 = fvec3 * (float)(i + 1) - fvec4;
        
        /* Vector reduction creates more register pressure */
        double vec_sum = vector_reduce(dvec3);
        accumulator += vec_sum + complex_val + complex_val2;
        
        /* Modify some values for next iteration */
        t1 = t1 + i;
        t2 = t2 - i % 3;
        dvec3[0] += 0.1 * i;
        dvec3[1] -= 0.05 * i;
    }
    
    /* Final computation using all temporaries */
    double result = w5 + accumulator;
    result += (double)(t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10);
    result += (double)(u1 + u2 + u3 + u4 + u5);
    result += (double)(v1 + v2 + v3 + v4 + v5);
    
    /* Use vector elements */
    result += vec3[0] + vec3[1] + vec3[2] + vec3[3];
    result += fvec3[0] + fvec3[1] + fvec3[2] + fvec3[3];
    result += dvec4[0] + dvec4[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile double total = 0.0;
    
    /* Call test function multiple times */
    for (int outer = 0; outer < 10; outer++) {
        total += test_remat(iterations);
    }
    
    printf("Result: %f\n", total);
    return 0;
}
