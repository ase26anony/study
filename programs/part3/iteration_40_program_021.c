/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
static int __attribute__((noinline, noclone)) 
helper_compute(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

static double __attribute__((noinline, noclone))
helper_fp(double x, double y, int sel) {
    switch (sel & 3) {
        case 0: return x + y * 2.0;
        case 1: return x - y / 1.5;
        case 2: return x * y + 3.14159;
        default: return y * x - 2.71828;
    }
}

/* Main test function with high register pressure */
static volatile long __attribute__((noinline))
test_remat(volatile int input1, volatile long input2, 
           volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 | 0xFF;
    int a4 = input1 & 0x7F;
    int a5 = input1 ^ 0x55;
    int a6 = a1 + a2;
    int a7 = a3 - a4;
    int a8 = a5 * a6;
    int a9 = a7 / (a1 ? a1 : 1);
    int a10 = a8 ^ a9;
    
    long b1 = input2 + 1000;
    long b2 = input2 - 500;
    long b3 = b1 * 3;
    long b4 = b2 / 2;
    long b5 = b3 + b4;
    long b6 = b5 << 2;
    long b7 = b6 >> 1;
    long b8 = b7 | 0xFFFF;
    long b9 = b8 & 0xAAAAAAAA;
    long b10 = b9 ^ b10;
    
    float c1 = input3 * 1.5f;
    float c2 = input3 / 2.0f;
    float c3 = c1 + c2;
    float c4 = c1 - c2;
    float c5 = c3 * c4;
    float c6 = c5 + 123.456f;
    float c7 = c6 - 78.9f;
    float c8 = c7 * 2.71828f;
    float c9 = c8 / 3.14159f;
    float c10 = c9 + c1;
    
    double d1 = input4 * 1.234;
    double d2 = input4 / 5.678;
    double d3 = d1 + d2;
    double d4 = d1 - d2;
    double d5 = d3 * d4;
    double d6 = helper_fp(d5, d4, a1);
    
    /* Vector variables for wide register pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c7, c8};
    v4sf vf3 = vf1 + vf2;
    v4sf vf4 = vf1 * vf2;
    
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    v2df vd3 = vd1 + vd2;
    v2df vd4 = vd1 * vd2;
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 | a5;
    int t5 = t4 ^ a6;
    int t6 = t5 + a7;
    int t7 = t6 * a8;
    int t8 = t7 - a9;
    int t9 = t8 | a10;
    int t10 = t9 ^ t1;
    
    long u1 = b1 + b2;
    long u2 = u1 * b3;
    long u3 = u2 - b4;
    long u4 = u3 | b5;
    long u5 = u4 ^ b6;
    long u6 = u5 + b7;
    long u7 = u6 * b8;
    long u8 = u7 - b9;
    long u9 = u8 | b10;
    long u10 = u9 ^ u1;
    
    float f1 = c1 + c2;
    float f2 = f1 * c3;
    float f3 = f2 - c4;
    float f4 = f3 + c5;
    float f5 = f4 * c6;
    float f6 = f5 - c7;
    float f7 = f6 + c8;
    float f8 = f7 * c9;
    float f9 = f8 - c10;
    float f10 = f9 + f1;
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version - adjust for your architecture */
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
    
    /* Complex expression computed and used multiple times */
    /* This increases chances for early rematerialization */
    int complex_expr = (t10 * 3) + (u10 & 0xFFF) + ((int)f10 * 2);
    
    /* Use complex_expr in multiple statements */
    int use1 = complex_expr + a1;
    int use2 = complex_expr - a2;
    int use3 = complex_expr | a3;
    int use4 = complex_expr ^ a4;
    
    /* Control flow to split basic blocks */
    if (complex_expr > 1000) {
        t10 = helper_compute(use1, use2, use3);
        u10 = (long)t10 * b1;
    } else {
        t10 = helper_compute(use4, use1, use2);
        u10 = (long)t10 * b2;
    }
    
    /* Recomputation of similar expression */
    /* This pattern encourages rematerialization decisions */
    int complex_expr2 = (t10 * 3) + (u10 & 0xFFF) + ((int)f10 * 2) + 1;
    
    /* More vector operations */
    v4si v6 = v5 + v3;
    v4sf vf5 = vf3 * vf4;
    v2df vd5 = vd3 + vd4;
    
    /* Final computation using all major temporaries */
    long result = (long)t10 + u10 + (long)complex_expr2 + 
                  (long)(f10 * 100.0f) + (long)d6 +
                  v6[0] + v6[1] + v6[2] + v6[3] +
                  (long)vf5[0] + (long)vd5[0];
    
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
        volatile long in2 = input_seed * 1000L + i * 37L;
        volatile float in3 = (float)input_seed / 3.14f + i * 0.1f;
        volatile double in4 = (double)input_seed * 1.618 + i * 0.01;
        
        long result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(total));
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
