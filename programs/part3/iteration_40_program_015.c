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
    if (a > b) return a * c - b;
    return b * c + a;
}

__attribute__((noinline, noclone))
static float helper2(float x, float y, float z) {
    if (x < y) return x * y + z;
    return y * z - x;
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
    /* Declare many local variables - at least 30 */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a * 5;
    long f = b + 6;
    float g = c - 7.0f;
    double h = d * 8.0;
    int i = e & 0xFF;
    long j = f | 0xFFFF;
    float k = g * 2.5f;
    double l = h / 1.5;
    int m = i ^ 0xAA;
    long n = j << 2;
    float o = k + 10.0f;
    double p = l - 5.0;
    int q = m * 3;
    long r = n >> 1;
    float s = o / 2.0f;
    double t = p * 3.0;
    
    /* Vector variables */
    v4si v1 = {a, e, i, m};
    v4si v2 = {q, 2, 4, 6};
    v4sf v3 = {c, g, k, o};
    v4sf v4 = {1.5f, 2.5f, 3.5f, 4.5f};
    v2df v5 = {d, h};
    v2df v6 = {l, t};
    
    /* More scalars */
    int u = q + r;
    long v = r * 2;
    float w = s * t;
    double x = t + w;
    int y = u & v;
    long z = v | u;
    float aa = w - x;
    double bb = x * 2.0;
    int cc = y ^ z;
    long dd = z << 1;
    float ee = aa / bb;
    double ff = bb + ee;
    
    /* Long serial chain of interdependent operations */
    int t1 = a + b;
    long t2 = t1 * c;
    float t3 = t2 - d;
    double t4 = t3 * e;
    int t5 = t4 + f;
    long t6 = t5 * g;
    float t7 = t6 - h;
    double t8 = t7 * i;
    int t9 = t8 + j;
    long t10 = t9 * k;
    float t11 = t10 - l;
    double t12 = t11 * m;
    int t13 = t12 + n;
    long t14 = t13 * o;
    float t15 = t14 - p;
    double t16 = t15 * q;
    int t17 = t16 + r;
    long t18 = t17 * s;
    float t19 = t18 - t;
    double t20 = t19 * u;
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = vt1 * helper3(v1, v2);
    v4sf vt3 = v3 * v4;
    v4sf vt4 = vt3 + v3;
    v2df vt5 = v5 * v6;
    v2df vt6 = vt5 + v5;
    
    /* Control flow to create multiple basic blocks */
    if (t1 > t2) {
        t3 = helper2(t1, t2, t3);
        vt1 = helper3(vt1, vt2);
    } else {
        t4 = helper2(t2, t3, t4);
        vt2 = helper3(vt2, vt1);
    }
    
    switch (t5 % 4) {
        case 0:
            t6 = helper1(t5, t6, t7);
            vt3 = vt3 * 2.0f;
            break;
        case 1:
            t7 = helper1(t6, t7, t8);
            vt4 = vt4 / 2.0f;
            break;
        case 2:
            t8 = helper1(t7, t8, t9);
            vt5 = vt5 * 1.5;
            break;
        default:
            t9 = helper1(t8, t9, t10);
            vt6 = vt6 / 1.5;
            break;
    }
    
    /* Inline assembly to clobber registers */
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
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    /* ARM version (commented out, use for ARM targets) */
    /*
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
    
    /* Recomputation of earlier values in different forms */
    /* This increases chances for early rematerialization */
    int t1_recomp = (a * 2) + (b / 2);
    long t2_recomp = t1_recomp * (c + 1.0f);
    float t3_recomp = t2_recomp - (d * 0.5);
    double t4_recomp = t3_recomp * (e ^ 0xF);
    
    /* Use recomputed values */
    if (t1_recomp > t2_recomp) {
        t5 = t1_recomp + t2_recomp;
    } else {
        t6 = t2_recomp - t1_recomp;
    }
    
    /* More operations with all variables to keep them live */
    int result1 = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    long result2 = t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    float result3 = vt1[0] + vt1[1] + vt1[2] + vt1[3] +
                    vt2[0] + vt2[1] + vt2[2] + vt2[3];
    double result4 = vt3[0] + vt3[1] + vt3[2] + vt3[3] +
                     vt4[0] + vt4[1] + vt4[2] + vt4[3] +
                     vt5[0] + vt5[1] + vt6[0] + vt6[1];
    
    /* Final volatile result to prevent optimization */
    volatile int final_result = result1 + result2 + result3 + result4 +
                                t1_recomp + t2_recomp + t3_recomp + t4_recomp +
                                a + b + c + d + e + f + g + h + i + j +
                                k + l + m + n + o + p + q + r + s + t +
                                u + v + w + x + y + z + aa + bb + cc + dd + ee + ff;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile int total = 0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 6364136223846793005UL + 1);
        seed3 = seed3 * 1.01f + 0.5f;
        seed4 = seed4 * 1.001 + 0.1;
        
        total += test_remat(seed1, seed2, seed3, seed4);
    }
    
    printf("Final result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to ensure execution */
}
