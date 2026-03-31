/* test_early_remat.c */
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
static float helper2(float a, float b, float c) {
    if (a < b) return a * b + c;
    return b * c - a;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 2, 3, 4};
    return a * b + mask;
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
    v4sf vf2 = {d, d/2, d/3, d/4};
    v2df vd1 = {d, d*2};
    v2df vd2 = {input4/3, input4/4};
    
    /* Complex serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)(b >> 4);
    t3 = helper1(t1, t2, a);
    t4 = t2 - t3 + (int)(b & 0xFF);
    
    l1 = b * t4;
    l2 = l1 + (t1 << 3);
    l3 = l2 - (t3 * 7);
    l4 = helper1(t4, t3, t2) + l3;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = helper2(f1, f2, c);
    f4 = f3 - f2 + t3;
    
    d1 = d + l1;
    d2 = d1 * f1;
    d3 = d2 - f3 * 2.5;
    d4 = d3 + helper2(f4, f3, f2);
    
    /* Vector operations */
    v1 = v1 + v2;
    v2 = v1 * (v4si){t1, t2, t3, t4};
    v1 = helper3(v1, v2);
    
    vf1 = vf1 + vf2;
    vf2 = vf1 * (v4sf){f1, f2, f3, f4};
    
    vd1 = vd1 + vd2;
    vd2 = vd1 * (v2df){d1, d2};
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        "pxor %%xmm0, %%xmm0\n"
        "pxor %%xmm1, %%xmm1\n"
        "pxor %%xmm2, %%xmm2\n"
        "pxor %%xmm3, %%xmm3\n"
        "pxor %%xmm4, %%xmm4\n"
        "pxor %%xmm5, %%xmm5\n"
        "pxor %%xmm6, %%xmm6\n"
        "pxor %%xmm7, %%xmm7\n"
        "pxor %%xmm8, %%xmm8\n"
        "pxor %%xmm9, %%xmm9\n"
        "pxor %%xmm10, %%xmm10\n"
        "pxor %%xmm11, %%xmm11\n"
        "pxor %%xmm12, %%xmm12\n"
        "pxor %%xmm13, %%xmm13\n"
        "pxor %%xmm14, %%xmm14\n"
        "pxor %%xmm15, %%xmm15\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* More operations after clobber - forcing recomputation */
    t5 = t4 * 3 + t3;  /* Complex expression that might need rematerialization */
    t6 = t5 - t2;
    t7 = helper1(t5, t6, t4);
    
    /* Use t5 in multiple places to increase liveness */
    t8 = t5 + t7;
    t9 = t5 * 2 - t8;
    
    /* Recompute similar expression to t5 but slightly different */
    t10 = t4 * 3 + t3 + 1;  /* Slightly different from t5 */
    
    l5 = l4 + t5;
    l6 = l5 * t10;
    l7 = helper1(t9, t10, t8) + l6;
    l8 = l7 - t5 * 2;
    
    f5 = f4 + t5;
    f6 = f5 * t10;
    f7 = helper2(f5, f6, f4);
    f8 = f7 - t5 / 2.0f;
    
    d5 = d4 + t5;
    d6 = d5 * t10;
    
    /* More vector operations */
    v1 = v1 + (v4si){t5, t6, t7, t8};
    v2 = v2 * (v4si){t9, t10, t5, t6};
    
    vf1 = vf1 + (v4sf){f5, f6, f7, f8};
    vf2 = vf2 * (v4sf){f5, f6, f7, f8};
    
    vd1 = vd1 + (v2df){d5, d6};
    vd2 = vd2 * (v2df){d5, d6};
    
    /* Control flow to split basic blocks */
    int result = t5;
    if (t5 > 1000) {
        result = helper1(t5, t6, t7) + v1[0] + (int)vf1[0];
    } else if (t5 > 500) {
        result = helper1(t6, t7, t8) + v2[1] + (int)vf2[1];
    } else {
        switch (t5 % 4) {
            case 0: result = l5 + v1[2]; break;
            case 1: result = l6 + v2[3]; break;
            case 2: result = (int)f5 + v1[0]; break;
            case 3: result = (int)d5 + v2[1]; break;
        }
    }
    
    /* Use all major temporaries in final computation */
    volatile int final_result = result 
        + t1 + t2 + t3 + t4 + t6 + t7 + t8 + t9 + t10
        + (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 + (int)l6 + (int)l7 + (int)l8
        + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8
        + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6
        + v1[0] + v1[1] + v1[2] + v1[3]
        + v2[0] + v2[1] + v2[2] + v2[3];
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile int total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        int result = test_remat(input1 + i, input2 + i, 
                               input3 + i * 0.1f, input4 + i * 0.01);
        total += result;
        
        /* Modify inputs to prevent optimization */
        input1 = (input1 * 13 + 17) % 1000;
        input2 = (input2 * 17 + 13) % 1000000;
        input3 = input3 * 1.01f;
        input4 = input4 * 0.99;
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
