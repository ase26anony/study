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

__attribute__((noinline, noclone))
static double vector_compute(v4sf vec1, v4sf vec2) {
    v4sf result = vec1 + vec2 * 2.5f;
    float sum = result[0] + result[1] + result[2] + result[3];
    return (double)sum;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1 + 1;
    volatile long b = input2 - 2;
    volatile float c = input3 * 3.14f;
    volatile double d = input4 / 2.71828;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    
    /* Vector variables */
    v4si vec_int1 = {a, a+1, a+2, a+3};
    v4si vec_int2 = {b%100, (b+1)%100, (b+2)%100, (b+3)%100};
    v4sf vec_float1 = {c, c*2.0f, c*3.0f, c*4.0f};
    v4sf vec_float2 = {input3, input3*0.5f, input3*1.5f, input3*2.5f};
    v2df vec_double1 = {d, d*1.1};
    v2df vec_double2 = {input4, input4*0.9};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)(c * 10.0f);
    t3 = t2 - (int)(d * 100.0);
    t4 = t3 ^ (t1 << 2);
    t5 = (t4 & 0xFF) | (t2 & 0xFF00);
    t6 = t5 * t3 / (t1 + 1);
    t7 = t6 + compute_partial(t1, t2, t3, t4);
    t8 = t7 - compute_partial(t5, t6, t7, t8);
    t9 = (t8 * 37) % 101;
    t10 = t9 ^ t8 ^ t7 ^ t6;
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ t4;
    l5 = l4 | t5;
    l6 = l5 & t6;
    l7 = l6 + t7;
    l8 = l7 - t8;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - t3;
    f4 = f3 / (t4 + 1.0f);
    f5 = f4 * f3 - f2;
    f6 = f5 + compute_partial((int)f1, (int)f2, (int)f3, (int)f4);
    f7 = f6 * 1.414f;
    f8 = f7 / 3.141f;
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - l3;
    d4 = d3 / (l4 + 1.0);
    d5 = d4 * vector_compute(vec_float1, vec_float2);
    d6 = d5 - vector_compute(vec_float2, vec_float1);
    
    /* Vector operations */
    vec_int1 = vec_int1 + vec_int2 * 3;
    vec_int2 = vec_int1 - vec_int2;
    vec_float1 = vec_float1 + vec_float2 * 2.0f;
    vec_float2 = vec_float1 - vec_float2;
    vec_double1 = vec_double1 + vec_double2;
    vec_double2 = vec_double1 * 1.5 - vec_double2;
    
    /* Inline assembly to clobber registers */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* More computations after assembly clobber */
    int t11 = t10 + (vec_int1[0] + vec_int1[1] + vec_int1[2] + vec_int1[3]);
    long l9 = l8 * (vec_int2[0] | vec_int2[1] | vec_int2[2] | vec_int2[3]);
    float f9 = f8 + (vec_float1[0] + vec_float1[1] + vec_float1[2] + vec_float1[3]);
    double d7 = d6 * (vec_double1[0] + vec_double2[1]);
    
    /* Control flow to split basic blocks */
    volatile long result = 0;
    if (t11 > 1000) {
        result = l9 + t11 * 2;
    } else if (t11 > 500) {
        result = l9 + t11;
    } else {
        result = l9 - t11;
    }
    
    switch (t11 % 5) {
        case 0:
            result += (long)(f9 * 100.0f);
            break;
        case 1:
            result += (long)(d7 * 50.0);
            break;
        case 2:
            result += vec_int1[0] + vec_int1[2];
            break;
        case 3:
            result += (long)(vec_float1[1] * 10.0f);
            break;
        default:
            result += (long)(vec_double2[0] * 20.0);
            break;
    }
    
    /* Complex expression recomputed in slightly different form */
    int complex_expr = (t1 * t2 + t3 * t4 - t5 * t6) / (t7 + 1);
    /* Use it multiple times */
    result += complex_expr * 2;
    result += complex_expr / 3;
    result += complex_expr % 17;
    /* Recompute similar expression later */
    int recomputed_expr = (t1 * t3 + t2 * t4 - t5 * t7) / (t6 + 1);
    result += recomputed_expr;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005ULL + 1442695040888963407ULL;
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.01 - 0.3;
        
        long result = test_remat(seed1 % 1000, 
                                 seed2 % 10000,
                                 seed3,
                                 seed4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i));
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
