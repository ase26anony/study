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
static int helper1(int a, int b, int c) {
    return (a * b) ^ c;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    return (a + b) * c;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    return a + b * 3;
}

__attribute__((noinline, noclone))
static double helper4(double a, double b, double c) {
    if (a > b) return a * c;
    return b * c;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    int a = input1;
    long b = input2;
    float c = input3;
    double d = input4;
    
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    
    /* Vector variables */
    v4si v1 = {a, a+1, a+2, a+3};
    v4si v2 = {b%10, b%20, b%30, b%40};
    v4sf vf1 = {c, c*2, c*3, c*4};
    v4sf vf2 = {c/2, c/3, c/4, c/5};
    v2df vd1 = {d, d*2};
    v2df vd2 = {d/2, d/3};
    
    /* Complex interdependent arithmetic chain */
    t1 = a + (int)b;
    t2 = t1 * (int)(b >> 4);
    t3 = t2 ^ (t1 << 3);
    t4 = helper1(t1, t2, t3);
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ t4;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = helper2(f1, f2, c);
    f4 = f3 - f2;
    
    d1 = d + l1;
    d2 = d1 * f1;
    d3 = helper4(d1, d2, d);
    d4 = d3 - d2;
    
    /* Vector operations */
    v1 = v1 + v2;
    v2 = helper3(v1, v2);
    vf1 = vf1 * vf2;
    vd1 = vd1 + vd2;
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Continue with more computations after clobber */
    t5 = t4 + l4;
    t6 = t5 * (int)f4;
    t7 = t6 ^ (int)d4;
    t8 = helper1(t5, t6, t7);
    
    l5 = l4 + (long)t5;
    l6 = l5 * (long)t6;
    l7 = l6 - (long)t7;
    l8 = l7 ^ (long)t8;
    
    f5 = f4 + t5;
    f6 = f5 * t6;
    f7 = helper2(f5, f6, f4);
    f8 = f7 - f6;
    
    d5 = d4 + l5;
    d6 = helper4(d5, d4, d3);
    
    /* More vector operations */
    v4si v3 = v1 * 2;
    v4si v4 = helper3(v2, v3);
    v4sf vf3 = vf1 + vf2;
    v2df vd3 = vd1 * 2.0;
    
    /* Control flow to create multiple basic blocks */
    if (t8 > 1000) {
        t9 = t8 * 2;
        t10 = helper1(t9, t8, t7);
        
        /* Recompute a complex expression in slightly different form */
        /* This may trigger early rematerialization decisions */
        int recomputed_t1 = a + (int)b + 1;  /* Similar to t1 but different */
        int recomputed_t2 = recomputed_t1 * (int)(b >> 4) + 5;
        
        t9 = recomputed_t2 ^ t10;
    } else {
        t9 = t8 / 2;
        t10 = helper1(t9, t7, t6);
        
        /* Another recomputation opportunity */
        long recomputed_l1 = b + t1 + 2;
        long recomputed_l2 = recomputed_l1 * t2 - 3;
        
        t10 += (int)recomputed_l2;
    }
    
    /* Switch statement for more control flow complexity */
    switch (t9 % 4) {
        case 0:
            t9 = v1[0] + v2[1];
            break;
        case 1:
            t9 = v1[1] * v2[2];
            break;
        case 2:
            t9 = (int)vf1[0] + (int)vf2[1];
            break;
        case 3:
            t9 = (int)vd1[0] * 2;
            break;
    }
    
    /* Final computation using all temporaries */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         (int)l1 + (int)l2 + (int)l3 + (int)l4 +
                         (int)l5 + (int)l6 + (int)l7 + (int)l8 +
                         (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                         (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                         (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                         (int)d5 + (int)d6 +
                         v1[0] + v1[1] + v1[2] + v1[3] +
                         v2[0] + v2[1] + v2[2] + v2[3] +
                         (int)vf1[0] + (int)vf1[1] + (int)vf1[2] + (int)vf1[3] +
                         (int)vd1[0] + (int)vd1[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile long total = 0;
    
    /* Main loop to increase register pressure across iterations */
    for (volatile int i = 0; i < loop_count; i++) {
        /* Use different inputs each iteration to prevent optimization */
        volatile int input1 = i * 3 + 1;
        volatile long input2 = i * 5 + 2;
        volatile float input3 = i * 1.5f + 3.0f;
        volatile double input4 = i * 2.7 + 4.0;
        
        int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Small computation between calls to create inter-procedural pressure */
        if (i % 10 == 0) {
            /* Another inline assembly to clobber registers between calls */
            asm volatile (
                "# Clobber between iterations\n\t"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "memory"
            );
        }
    }
    
    printf("Final result: %ld\n", total);
    return (int)total;
}
