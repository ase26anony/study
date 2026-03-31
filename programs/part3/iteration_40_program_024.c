/* test-early-remat.c */
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
    return (a * b) ^ c;
}

__attribute__((noinline, noclone))
static double helper2(double a, double b, double c) {
    if (a > b) return a * c;
    return b * c;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    return a + b * 3;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a = input1;
    long b = input2;
    float c = input3;
    double d = input4;
    
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5, f6;
    double d1, d2, d3, d4, d5, d6, d7;
    
    /* Vector variables */
    v4si v1 = {a, a+1, a+2, a+3};
    v4si v2 = {b%100, b%101, b%102, b%103};
    v4sf vf1 = {c, c*2, c*3, c*4};
    v4sf vf2 = {d, d/2, d/3, d/4};
    v2df vd1 = {d, d*2};
    v2df vd2 = {d/3, d/4};
    
    /* Complex interdependent computation chain */
    t1 = a + (int)b;
    t2 = t1 * (int)(b >> 4);
    t3 = t2 ^ (int)(input1 * 3);
    t4 = t3 - (int)c;
    
    /* Use helper to create basic block boundary */
    if (t4 > 1000) {
        t5 = helper1(t1, t2, t3);
    } else {
        t5 = helper1(t3, t2, t1);
    }
    
    l1 = b * 37;
    l2 = l1 + (long)t1 * 41;
    l3 = l2 ^ (long)t2 * 43;
    
    f1 = c * 2.5f;
    f2 = f1 + (float)t3 / 7.0f;
    f3 = f2 * (float)l1;
    
    d1 = d * 3.14159;
    d2 = d1 + (double)f1 * 2.71828;
    d3 = helper2(d1, d2, d);
    
    /* Vector operations */
    v1 = v1 + v2 * 2;
    v2 = helper3(v1, v2);
    vf1 = vf1 + vf2;
    vf2 = vf1 * 2.5f;
    vd1 = vd1 + vd2;
    vd2 = vd1 * 1.618034;
    
    /* Inline assembly to clobber registers */
    asm volatile(
        "# Clobber many registers\n\t"
        :
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* More computations after clobber */
    t6 = t5 + (int)(l3 % 100);
    t7 = t6 * (int)f3;
    t8 = t7 ^ (int)d3;
    
    l4 = l3 + (long)t6 * 47;
    l5 = l4 ^ (long)t7 * 53;
    
    f4 = f3 + (float)t8 / 11.0f;
    f5 = f4 * (float)l4;
    f6 = helper2(d2, d3, f5);
    
    d4 = d3 + (double)f4 * 1.41421;
    d5 = d4 * (double)l5;
    d6 = helper2(d4, d5, d);
    
    /* Second computation of similar expression (for remat) */
    int t1_again = a + (int)b;  /* Same as t1 earlier */
    int t2_again = t1_again * (int)(b >> 4);  /* Same as t2 */
    
    /* Use in different way */
    t9 = t2_again + t8;
    t10 = t9 * (int)d6;
    
    /* More vector ops */
    v4si v3 = v1 + v2;
    v4sf vf3 = vf1 + vf2 * 0.5f;
    v2df vd3 = vd1 + vd2 * 0.7071;
    
    /* Switch statement for control flow */
    switch (t10 % 7) {
        case 0:
            d7 = d6 * 1.1;
            break;
        case 1:
            d7 = d6 * 1.2;
            break;
        case 2:
            d7 = d6 * 1.3;
            break;
        default:
            d7 = d6 * 1.4;
            break;
    }
    
    /* Final combination of all values */
    int result = t10 + (int)l5 + (int)f6 + (int)d7;
    result += v1[0] + v2[1] + (int)vf3[2] + (int)vd3[0];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile long total = 0;
    
    /* Volatile inputs to prevent constant propagation */
    volatile int in1 = 42;
    volatile long in2 = 123456789;
    volatile float in3 = 3.14159f;
    volatile double in4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Modify inputs slightly each iteration */
        in1 = (in1 * 1103515245 + 12345) & 0x7fffffff;
        in2 = in2 * 6364136223846793005UL + 1442695040888963407UL;
        in3 = in3 * 1.1f + 0.5f;
        in4 = in4 * 1.05 + 0.25;
        
        int result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", total);
    return (int)(total % 1000);
}
