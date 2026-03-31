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
static int complex_transform(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

__attribute__((noinline, noclone))
static float float_transform(float a, float b, float c) {
    switch ((int)a % 4) {
        case 0: return a * b + c;
        case 1: return a / b - c;
        case 2: return a + b * c;
        default: return a - b / c;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare at least 30 distinct local variables */
    volatile int a = input1 + 1;
    volatile long b = input2 - 2;
    volatile float c = input3 * 3.0f;
    volatile double d = input4 / 4.0;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    
    /* Vector variables for wider register pressure */
    v4si vec1 = {a, a+1, a+2, a+3};
    v4si vec2 = {b%100, (b+1)%100, (b+2)%100, (b+3)%100};
    v4sf vecf1 = {c, c+1.0f, c+2.0f, c+3.0f};
    v4sf vecf2 = {d, d+1.0, d+2.0, d+3.0};
    v2df vecd1 = {d, d*2.0};
    v2df vecd2 = {d/2.0, d/3.0};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (a % 17);
    t3 = t2 - (b % 23);
    t4 = t3 ^ (t1 << 2);
    t5 = t4 | (t2 >> 1);
    t6 = complex_transform(t5, t3, t4);
    t7 = t6 + (t1 & t2);
    t8 = t7 * (t3 | t4);
    t9 = t8 - (t5 ^ t6);
    t10 = t9 >> (t7 % 8);
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ t4;
    l5 = l4 | t5;
    l6 = l5 + t6;
    l7 = l6 * t7;
    l8 = l7 - t8;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = float_transform(f2, c, f1);
    f4 = f3 - t3;
    f5 = f4 / (t4 + 1.0f);
    f6 = float_transform(f5, f3, f4);
    f7 = f6 * t5;
    f8 = f7 - t6;
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - l3;
    d4 = d3 / (l4 + 1.0);
    d5 = d4 * l5;
    d6 = d5 - l6;
    
    /* Vector operations consuming wide registers */
    vec1 = vec1 + vec2;
    vec2 = vec1 * vec2;
    vec1 = vec2 - vec1;
    
    vecf1 = vecf1 + vecf2;
    vecf2 = vecf1 * vecf2;
    vecf1 = vecf2 - vecf1;
    
    vecd1 = vecd1 + vecd2;
    vecd2 = vecd1 * vecd2;
    vecd1 = vecd2 - vecd1;
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version - clobber many registers */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
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
        "pxor %%xmm15, %%xmm15"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* ARM version (commented out - use appropriate for target)
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
        "vmov.i32 q7, #0"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "memory"
    );
    */
    
    /* Continue with more operations after clobber */
    /* Force recomputation of earlier values in different forms */
    int t1_alt = (a * 2) + (int)(b / 2);  /* Similar to t1 but different */
    int t2_alt = t1_alt * (a % 19);       /* Similar to t2 but different */
    
    /* Use both original and recomputed values */
    int combined1 = t1 + t1_alt;
    int combined2 = t2 * t2_alt;
    
    /* More complex expressions that might trigger rematerialization */
    long l9 = (l8 * 3) + (t9 << 2);
    float f9 = (f8 * 2.0f) - (t10 / 100.0f);
    double d7 = (d6 / 3.0) + (l7 % 1000);
    
    /* Vector operations continue */
    v4si vec3 = vec1 + (v4si){t1, t2, t3, t4};
    v4sf vecf3 = vecf1 * (v4sf){f1, f2, f3, f4};
    v2df vecd3 = vecd1 + (v2df){d1, d2};
    
    /* Conditional block to split control flow */
    if (combined1 > combined2) {
        vec3 = vec3 * 2;
        vecf3 = vecf3 / 2.0f;
        vecd3 = vecd3 * 1.5;
    } else {
        vec3 = vec3 / 2;
        vecf3 = vecf3 * 2.0f;
        vecd3 = vecd3 / 1.5;
    }
    
    /* Switch statement for more control flow complexity */
    switch (t10 % 5) {
        case 0:
            l9 = l9 + vec3[0];
            break;
        case 1:
            f9 = f9 + vecf3[1];
            break;
        case 2:
            d7 = d7 + vecd3[0];
            break;
        case 3:
            l9 = l9 - vec3[2];
            break;
        default:
            f9 = f9 - vecf3[3];
            break;
    }
    
    /* Final computation using all major temporaries */
    volatile long result = (long)t10 + l9 + (long)f9 + (long)d7 +
                          vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                          (long)vecf3[0] + (long)vecf3[1] +
                          (long)vecd3[0] + (long)vecd3[1];
    
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
        volatile long result = test_remat(
            input1 + i,
            input2 - i * 2,
            input3 + i * 0.1f,
            input4 - i * 0.01
        );
        total += result;
        
        /* Modify inputs for next iteration */
        input1 = (input1 * 13 + 17) % 1000;
        input2 = (input2 * 17 + 13) % 1000000;
        input3 = input3 * 1.1f;
        input4 = input4 / 1.01;
    }
    
    printf("Final result: %ld\n", (long)total);
    return 0;
}
