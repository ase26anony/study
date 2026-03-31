/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
__attribute__((noinline, noclone))
static int complex_condition(int a, int b, float c, double d) {
    if (a > b) {
        return (int)(c * 2.0f) + (int)d;
    } else if (a < b) {
        return (int)(c / 2.0f) - (int)d;
    } else {
        return (int)c * (int)d;
    }
}

__attribute__((noinline, noclone))
static v4si vector_transform(v4si v, int scalar) {
    switch (scalar & 3) {
        case 0: return v + (v4si){scalar, scalar, scalar, scalar};
        case 1: return v * (v4si){scalar, scalar, scalar, scalar};
        case 2: return v - (v4si){scalar, scalar, scalar, scalar};
        default: return v & (v4si){scalar, scalar, scalar, scalar};
    }
}

/* Main test function with high register pressure */
__attribute__((noinline))
static volatile long test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* More scalar variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Start long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)c;
    t3 = t2 - (int)d;
    t4 = t3 ^ t1;
    t5 = t4 | t2;
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ l1;
    l5 = l4 | l2;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - t3;
    f4 = f3 / f1;
    f5 = f4 * f2;
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - l3;
    d4 = d3 / d1;
    d5 = d4 * d2;
    
    /* Vector operations */
    v1 = (v4si){t1, t2, t3, t4};
    v2 = (v4si){t5, t1, t2, t3};
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f1, f2, f3};
    vf3 = vf1 * vf2 - vf1 / vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 + vd2 * (v2df){d5, d1};
    
    /* Control flow to create multiple basic blocks */
    if (t1 > 0) {
        t6 = complex_condition(t1, t2, f1, d1);
        t7 = t6 * t5 + t4;
    } else {
        t6 = complex_condition(t2, t1, f2, d2);
        t7 = t6 / t5 - t4;
    }
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version */
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
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* ARM version (commented out, use for ARM targets) */
    /*
    asm volatile (
        "# Clobber ARM registers\n\t"
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
        "vmov.i32 q8, #0\n\t"
        "vmov.i32 q9, #0\n\t"
        "vmov.i32 q10, #0\n\t"
        "vmov.i32 q11, #0\n\t"
        "vmov.i32 q12, #0\n\t"
        "vmov.i32 q13, #0\n\t"
        "vmov.i32 q14, #0\n\t"
        "vmov.i32 q15, #0"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
          "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "memory"
    );
    */
    
    /* Continue computation after clobber */
    t8 = t7 + t6;
    t9 = t8 * t5;
    t10 = t9 - t4;
    
    l6 = l5 + t8;
    l7 = l6 * t9;
    l8 = l7 - t10;
    
    f6 = f5 + t8;
    f7 = f6 * t9;
    f8 = f7 - t10;
    
    d6 = d5 + l6;
    d7 = d6 * l7;
    d8 = d7 - l8;
    
    /* Vector operations after clobber */
    v1 = vector_transform(v5, t8);
    v2 = vector_transform(v1, t9);
    v3 = v1 + v2;
    v4 = v1 * v2;
    v5 = v3 - v4;
    
    /* Recompute similar expressions to force rematerialization decisions */
    /* This value is computed, used, then recomputed in slightly different form */
    int complex_val = (t1 * t2 + t3) / (t4 | 1);
    t8 = complex_val + t5;
    t9 = t8 * l1;
    
    /* Recomputation in slightly different form */
    int complex_val2 = (t1 * t2 + t3) / (t4 | 2);  /* Slightly different */
    t10 = complex_val2 - t5;
    
    /* More control flow */
    switch (t10 & 7) {
        case 0: l6 = l5 + complex_val; break;
        case 1: l6 = l5 - complex_val; break;
        case 2: l6 = l5 * complex_val; break;
        case 3: l6 = l5 / (complex_val | 1); break;
        case 4: l6 = l5 ^ complex_val; break;
        case 5: l6 = l5 | complex_val; break;
        case 6: l6 = l5 & complex_val; break;
        default: l6 = l5 % (complex_val | 1); break;
    }
    
    /* Final computation using all major temporaries */
    volatile long result = (long)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 +
                         (long)f1 + (long)f2 + (long)f3 + (long)f4 + 
                         (long)f5 + (long)f6 + (long)f7 + (long)f8 +
                         (long)d1 + (long)d2 + (long)d3 + (long)d4 +
                         (long)d5 + (long)d6 + (long)d7 + (long)d8 +
                         v1[0] + v1[1] + v1[2] + v1[3] +
                         v5[0] + v5[1] + v5[2] + v5[3];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input1 = 12345;
    volatile long input2 = 6789012345L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Modify inputs slightly each iteration */
        input1 += i & 0xFF;
        input2 -= i * 3;
        input3 *= 1.0001f;
        input4 /= 1.0000001;
        
        long result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Additional computation to prevent loop optimization */
        if (total < 0) {
            total = -total;
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
