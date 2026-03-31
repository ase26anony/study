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
static int compute_complex(int a, int b, int c, int d) {
    volatile int barrier = a;
    if (barrier > 0) {
        return (a * b) + (c / (d ? d : 1));
    } else {
        return (a + b) * (c - d);
    }
}

__attribute__((noinline, noclone))
static double fp_complex(double a, double b, double c) {
    volatile double barrier = a;
    if (barrier > 0.0) {
        return (a * b) / (c + 1.0);
    } else {
        return (a + b) * c;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1;
    volatile long b = input2 + 1;
    volatile float c = input3 * 2.0f;
    volatile double d = input4 / 3.0;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize vectors */
    v1 = (v4si){a, a+1, a+2, a+3};
    v2 = (v4si){b%100, (b+1)%100, (b+2)%100, (b+3)%100};
    vf1 = (v4sf){c, c*2.0f, c*3.0f, c*4.0f};
    vd1 = (v2df){d, d*1.5};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)(b >> 4);
    t3 = t2 - (int)(input1 * 2);
    t4 = t3 / (t1 ? t1 : 1);
    t5 = t4 | (t2 & 0xFF);
    t6 = t5 ^ t3;
    t7 = t6 << 2;
    t8 = t7 >> 1;
    t9 = t8 + t1 - t2;
    t10 = t9 * t3 / (t4 ? t4 : 1);
    
    l1 = b + a;
    l2 = l1 * (b % 37);
    l3 = l2 - (input2 >> 3);
    l4 = l3 / (l1 ? l1 : 1);
    l5 = l4 | (l2 & 0xFFFF);
    l6 = l5 ^ l3;
    l7 = l6 << 3;
    l8 = l7 >> 2;
    l9 = l8 + l1 - l2;
    l10 = l9 * l3 / (l4 ? l4 : 1);
    
    f1 = c + (float)a;
    f2 = f1 * (float)(b % 19);
    f3 = f2 - input3;
    f4 = f3 / (f1 + 0.001f);
    f5 = f4 * 1.5f;
    f6 = f5 - f2;
    f7 = f6 + f3;
    f8 = f7 * 0.75f;
    f9 = f8 / (f4 + 0.001f);
    f10 = f9 * f1 - f2;
    
    d1 = d + (double)a;
    d2 = d1 * (double)(b % 23);
    d3 = d2 - input4;
    d4 = d3 / (d1 + 0.0001);
    d5 = d4 * 1.25;
    d6 = d5 - d2;
    d7 = d6 + d3;
    d8 = d7 * 0.8;
    d9 = d8 / (d4 + 0.0001);
    d10 = d9 * d1 - d2;
    
    /* Vector operations */
    v3 = v1 + v2;
    v4 = v3 * (v4si){2, 3, 4, 5};
    v5 = v4 - v1;
    
    vf2 = vf1 * (v4sf){1.5f, 2.0f, 2.5f, 3.0f};
    vf3 = vf2 + vf1;
    
    vd2 = vd1 * (v2df){0.5, 1.5};
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    /* For ARM (commented out, use appropriate target)
    asm volatile (
        "# Clobber many registers\n\t"
        "mov r0, #0\n\t"
        "mov r1, #0\n\t"
        "mov r2, #0\n\t"
        "mov r3, #0\n\t"
        "mov r4, #0\n\t"
        "mov r5, #0\n\t"
        "mov r6, #0\n\t"
        "mov r7, #0\n\t"
        "mov r8, #0\n\t"
        "mov r9, #0\n\t"
        "mov r10, #0\n\t"
        "vmov.i32 q0, #0\n\t"
        "vmov.i32 q1, #0\n\t"
        "vmov.i32 q2, #0\n\t"
        "vmov.i32 q3, #0\n\t"
        "vmov.i32 q4, #0\n\t"
        "vmov.i32 q5, #0\n\t"
        "vmov.i32 q6, #0\n\t"
        "vmov.i32 q7, #0\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "memory"
    );
    */
    
    /* Control flow to split basic blocks */
    volatile int selector = t1 & 0x3;
    long result = 0;
    
    switch (selector) {
        case 0:
            result = compute_complex(t1, t2, t3, t4) + l1;
            result += (long)(f1 * 100.0f) + (long)(d1 * 100.0);
            break;
        case 1:
            result = compute_complex(t5, t6, t7, t8) + l2;
            result += (long)(f2 * 100.0f) + (long)(d2 * 100.0);
            break;
        case 2:
            result = compute_complex(t9, t10, t1, t2) + l3;
            result += (long)(f3 * 100.0f) + (long)(d3 * 100.0);
            break;
        default:
            result = compute_complex(t3, t4, t5, t6) + l4;
            result += (long)(f4 * 100.0f) + (long)(d4 * 100.0);
            break;
    }
    
    /* More computations after control flow */
    double fp_result = fp_complex(d5, d6, d7);
    fp_result += fp_complex(d8, d9, d10);
    
    /* Vector reductions */
    int vsum = v3[0] + v3[1] + v3[2] + v3[3];
    vsum += v4[0] + v4[1] + v4[2] + v4[3];
    vsum += v5[0] + v5[1] + v5[2] + v5[3];
    
    float vfsum = vf2[0] + vf2[1] + vf2[2] + vf2[3];
    vfsum += vf3[0] + vf3[1] + vf3[2] + vf3[3];
    
    double vdsum = vd2[0] + vd2[1];
    
    /* Final complex expression that might trigger rematerialization */
    /* This value is computed, used, and then recomputed in different form */
    volatile long final_complex = 
        (result * 2) + 
        (long)(fp_result * 1000.0) +
        vsum * 3 +
        (long)(vfsum * 100.0f) +
        (long)(vdsum * 1000.0);
    
    /* Use the value multiple times */
    if (final_complex > 1000000) {
        final_complex = final_complex / 2 + result;
    } else {
        final_complex = final_complex * 2 - result;
    }
    
    /* Recomputation in slightly different form */
    volatile long final_complex2 = 
        (result * 3) + 
        (long)(fp_result * 500.0) +
        vsum * 2 +
        (long)(vfsum * 50.0f) +
        (long)(vdsum * 500.0);
    
    /* Force use of all major temporaries */
    return final_complex + final_complex2 + 
           t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
           (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5 +
           (long)f6 + (long)f7 + (long)f8 + (long)f9 + (long)f10 +
           (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5 +
           (long)d6 + (long)d7 + (long)d8 + (long)d9 + (long)d10;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 1000L + i * 37L;
        volatile float in3 = (float)(input_seed + i) * 1.5f;
        volatile double in4 = (double)(input_seed + i) * 2.5;
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", (long)total);
    return 0;
}
