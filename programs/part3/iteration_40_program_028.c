/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
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
static float float_complex(float a, float b, float c, float d) {
    float t1 = a * b + c;
    float t2 = b * c - d;
    float t3 = t1 / (t2 + 1.0f);
    return t3 * (a + b + c + d);
}

__attribute__((noinline, noclone))
static v4si vector_op(v4si a, v4si b, v4si c) {
    v4si t1 = a + b;
    v4si t2 = b * c;
    v4si t3 = t1 - t2;
    v4si t4 = t3 & a;
    return t4 | b;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* Integer variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize vectors */
    v1 = (v4si){a, a+1, a+2, a+3};
    v2 = (v4si){b%100, b%101, b%102, b%103};
    v3 = (v4si){1, 2, 3, 4};
    vf1 = (v4sf){c, c*2, c*3, c*4};
    vf2 = (v4sf){d, d/2, d/3, d/4};
    vd1 = (v2df){d, d*2};
    vd2 = (v2df){d/3, d/4};
    
    /* Start long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * a - (int)(c * 10);
    t3 = t2 / (a + 1) + (int)d;
    t4 = t3 ^ t1;
    t5 = (t4 << 3) | (t2 & 0xFF);
    
    /* Complex expression that will be recomputed later */
    int complex_expr = (t1 * t2 + t3 - t4) ^ (t5 << 2);
    
    /* Use complex_expr multiple times */
    t6 = complex_expr + a;
    t7 = complex_expr * b;
    t8 = complex_expr - t1;
    
    /* Floating point chain */
    f1 = c + (float)a;
    f2 = f1 * (float)b;
    f3 = f2 / c;
    f4 = f3 - (float)t1;
    f5 = f4 * f1;
    
    /* Double precision chain */
    d1 = d + (double)b;
    d2 = d1 * (double)a;
    d3 = d2 / d;
    d4 = d3 - (double)t2;
    d5 = d4 * d1;
    
    /* Vector operations - consume wide registers */
    v4 = vector_op(v1, v2, v3);
    v5 = v1 + v2 * v3 - v4;
    
    vf3 = vf1 * vf2 + (v4sf){f1, f2, f3, f4};
    vd2 = vd1 * (v2df){d1, d2} - (v2df){d3, d4};
    
    /* Control flow to create multiple basic blocks */
    if (t1 > t2) {
        t9 = compute_branch(t1, t2, t3);
        f6 = float_complex(f1, f2, f3, f4);
    } else {
        t9 = compute_branch(t2, t1, t3);
        f6 = float_complex(f4, f3, f2, f1);
    }
    
    /* Switch statement for more basic blocks */
    switch (t3 & 0x3) {
        case 0:
            t10 = t4 + t5;
            f7 = f5 * 2.0f;
            break;
        case 1:
            t10 = t4 - t5;
            f7 = f5 / 2.0f;
            break;
        case 2:
            t10 = t4 * t5;
            f7 = f5 + 2.0f;
            break;
        default:
            t10 = t4 / (t5 ? t5 : 1);
            f7 = f5 - 2.0f;
            break;
    }
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
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
    
    /* For ARM (commented out, use appropriate for target)
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
          "memory"
    );
    */
    
    /* Recomputation of complex_expr in slightly different form */
    /* This increases likelihood of early rematerialization */
    int complex_expr2 = (t1 * t2 + t3 - t4) ^ (t5 << 1);  /* <<1 instead of <<2 */
    
    /* More operations using recomputed value */
    t11 = complex_expr2 + t6;
    t12 = complex_expr2 * t7;
    t13 = complex_expr2 - t8;
    
    /* Continue the long chain */
    t14 = t9 + t10 + t11;
    t15 = t12 * t13 - t14;
    t16 = (t15 << 2) | (t14 & 0xF);
    t17 = t16 ^ t13;
    t18 = t17 + t11 - t10;
    t19 = t18 * t9 / (t8 ? t8 : 1);
    t20 = t19 & 0xFFFF;
    
    f8 = f6 + f7;
    f9 = f8 * f5 - f4;
    f10 = f9 / (f3 + 1.0f);
    
    d6 = d5 + (double)f10;
    d7 = d6 * d4 - d3;
    d8 = d7 / (d2 + 1.0);
    d9 = d8 * d1;
    d10 = d9 - d;
    
    /* More vector operations */
    v4si v6 = v4 + v5 * v1;
    v4sf vf4 = vf3 * (v4sf){f8, f9, f10, 1.0f};
    v2df vd3 = vd2 + (v2df){d6, d7};
    
    /* Final computation using all major temporaries */
    long result = (long)t20 + (long)(f10 * 1000) + (long)(d10 * 1000)
                + v6[0] + v6[1] + v6[2] + v6[3]
                + (long)(vf4[0] + vf4[1] + vf4[2] + vf4[3])
                + (long)(vd3[0] + vd3[1]);
    
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
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input1 + (i % 100);
        volatile long in2 = input2 + i;
        volatile float in3 = input3 + (i * 0.01f);
        volatile double in4 = input4 + (i * 0.001);
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i));
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
