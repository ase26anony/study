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
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c - b;
    } else if (a < b) {
        return b * c + a;
    } else {
        return (a + b) * c;
    }
}

/* Another helper with switch statement */
__attribute__((noinline, noclone))
static double vector_select(int idx, double a, double b, double c, double d) {
    switch (idx & 3) {
        case 0: return a * 1.5;
        case 1: return b * 2.5;
        case 2: return c * 3.5;
        case 3: return d * 4.5;
        default: return a + b + c + d;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1 + 1;
    volatile long b = input2 - 1;
    volatile float c = input3 * 2.0f;
    volatile double d = input4 / 2.0;
    
    /* More variables for the chain */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3, vf4;
    v2df vd1, vd2, vd3;
    
    /* Initialize vectors */
    v1 = (v4si){a, a+1, a+2, a+3};
    v2 = (v4si){b&0xFF, (b>>8)&0xFF, (b>>16)&0xFF, (b>>24)&0xFF};
    vf1 = (v4sf){c, c*2, c*3, c*4};
    vd1 = (v2df){d, d*1.5};
    
    /* Start long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)c;
    t3 = t2 - (int)d;
    t4 = t3 | t1;
    t5 = t4 ^ t2;
    t6 = t5 & t3;
    t7 = t6 << 2;
    t8 = t7 >> 1;
    t9 = t8 + t4;
    t10 = t9 - t5;
    
    l1 = b + a;
    l2 = l1 * t1;
    l3 = l2 - t2;
    l4 = l3 | l1;
    l5 = l4 ^ l2;
    l6 = l5 & l3;
    l7 = l6 << 3;
    l8 = l7 >> 2;
    l9 = l8 + l4;
    l10 = l9 - l5;
    
    f1 = c + a;
    f2 = f1 * t3;
    f3 = f2 - b;
    f4 = f3 / f1;
    f5 = f4 * f2;
    f6 = f5 - f3;
    f7 = f6 + f4;
    f8 = f7 * f5;
    f9 = f8 / f6;
    f10 = f9 - f7;
    
    d1 = d + a;
    d2 = d1 * l1;
    d3 = d2 - f1;
    d4 = d3 / d1;
    d5 = d4 * d2;
    d6 = d5 - d3;
    d7 = d6 + d4;
    d8 = d7 * d5;
    d9 = d8 / d6;
    d10 = d9 - d7;
    
    /* Vector operations consuming more registers */
    v3 = v1 + v2;
    v4 = v3 * v1;
    v5 = v4 - v2;
    
    vf2 = vf1 * (v4sf){f1, f2, f3, f4};
    vf3 = vf2 + vf1;
    vf4 = vf3 - (v4sf){f5, f6, f7, f8};
    
    vd2 = vd1 * (v2df){d1, d2};
    vd3 = vd2 + (v2df){d3, d4};
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version */
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
    
    /* ARM version (commented out, use appropriate one for your target)
    asm volatile (
        "# Clobber ARM registers\n"
        "mov r0, #0\n"
        "mov r1, #0\n"
        "mov r2, #0\n"
        "mov r3, #0\n"
        "mov r4, #0\n"
        "mov r5, #0\n"
        "mov r6, #0\n"
        "mov r7, #0\n"
        "mov r8, #0\n"
        "mov r9, #0\n"
        "mov r10, #0\n"
        "vmov.i32 q0, #0\n"
        "vmov.i32 q1, #0\n"
        "vmov.i32 q2, #0\n"
        "vmov.i32 q3, #0\n"
        "vmov.i32 q4, #0\n"
        "vmov.i32 q5, #0\n"
        "vmov.i32 q6, #0\n"
        "vmov.i32 q7, #0\n"
        "vmov.i32 q8, #0\n"
        "vmov.i32 q9, #0\n"
        "vmov.i32 q10, #0\n"
        "vmov.i32 q11, #0\n"
        "vmov.i32 q12, #0\n"
        "vmov.i32 q13, #0\n"
        "vmov.i32 q14, #0\n"
        "vmov.i32 q15, #0\n"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
          "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15",
          "memory"
    );
    */
    
    /* Control flow to split basic blocks */
    int branch_result;
    if (t10 > 1000) {
        branch_result = compute_branch(t1, t2, t3);
        v3 = v3 + (v4si){branch_result, branch_result>>1, branch_result>>2, branch_result>>3};
    } else {
        branch_result = compute_branch(t4, t5, t6);
        v4 = v4 - (v4si){branch_result, branch_result>>1, branch_result>>2, branch_result>>3};
    }
    
    /* Recompute some values in slightly different forms (for rematerialization) */
    /* This expression is complex and used multiple times */
    int complex_expr = (t1 * t2 + t3 * t4 - t5 * t6) | (t7 & t8) ^ (t9 | t10);
    
    /* Use complex_expr multiple times */
    l1 = l1 + complex_expr;
    l2 = l2 - complex_expr;
    l3 = l3 * complex_expr;
    
    /* Recompute similar but not identical expression later */
    int complex_expr2 = (t2 * t3 + t4 * t5 - t6 * t7) | (t8 & t9) ^ (t10 | t1);
    
    /* Use the recomputed value */
    l4 = l4 + complex_expr2;
    l5 = l5 - complex_expr2;
    
    /* More vector operations */
    vf2 = vf2 * (v4sf){f9, f10, f1, f2};
    vf3 = vf3 + (v4sf){f3, f4, f5, f6};
    
    /* Switch statement for more control flow */
    double selected = vector_select(complex_expr & 0xF, d1, d2, d3, d4);
    d10 = d10 * selected;
    
    /* Final computation using all major temporaries */
    volatile long result = 
        (long)t10 + l10 + (long)f10 + (long)d10 +
        (long)v3[0] + (long)v3[1] + (long)v3[2] + (long)v3[3] +
        (long)vf4[0] + (long)vf4[1] + (long)vf4[2] + (long)vf4[3] +
        (long)vd3[0] + (long)vd3[1] +
        (long)branch_result + (long)complex_expr + (long)complex_expr2;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        input1 = input1 + (i & 0xF);
        input2 = input2 - (i & 0xFF);
        input3 = input3 * (1.0f + (i % 10) * 0.01f);
        input4 = input4 / (1.0 + (i % 5) * 0.01);
        
        long result = test_remat(input1, input2, input3, input4);
        total = total + result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total & 0x7FFFFFFF);
}
