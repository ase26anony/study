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
static int complex_transform(int a, int b, float c, double d) {
    if (a > b) {
        return (int)(a * c + d);
    } else {
        return (int)(b * c - d);
    }
}

__attribute__((noinline, noclone))
static v4si vector_transform(v4si vec, int scalar) {
    switch (scalar & 3) {
        case 0: return vec + scalar;
        case 1: return vec - scalar;
        case 2: return vec * scalar;
        default: return vec / (scalar ? scalar : 1);
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables of mixed types */
    int a = input1;
    long b = input2;
    float c = input3;
    double d = input4;
    
    /* More integer variables */
    int e = a * 2;
    int f = b & 0xFF;
    int g = a + f;
    int h = e - g;
    int i = h * 3;
    int j = i / 2;
    int k = j | 0x0F;
    int l = k ^ 0x55;
    int m = l << 2;
    int n = m >> 1;
    int o = n % 17;
    int p = o + 100;
    int q = p * a;
    int r = q - b;
    int s = r & 0xFFF;
    int t = s | 0xF0F0;
    
    /* More floating point variables */
    float u = c * 1.5f;
    float v = u + 2.0f;
    float w = v * c;
    float x = w - u;
    float y = x / 2.0f;
    float z = y + 3.14159f;
    
    /* Double precision variables */
    double aa = d * 2.0;
    double bb = aa + 1.0;
    double cc = bb * d;
    double dd = cc - aa;
    double ee = dd / 3.0;
    double ff = ee + 2.71828;
    
    /* Vector variables */
    v4si vec1 = {a, e, g, h};
    v4si vec2 = {i, j, k, l};
    v4sf vecf1 = {c, u, v, w};
    v4sf vecf2 = {x, y, z, 1.0f};
    v2df vecd1 = {d, aa};
    v2df vecd2 = {bb, cc};
    
    /* Long serial chain of interdependent operations */
    int t1 = a + b;
    long t2 = t1 * e;
    float t3 = t2 * c;
    double t4 = t3 + d;
    int t5 = t4 * 2;
    float t6 = t5 * u;
    double t7 = t6 - aa;
    int t8 = t7 / 3;
    long t9 = t8 | f;
    float t10 = t9 * v;
    double t11 = t10 + bb;
    int t12 = t11 * g;
    float t13 = t12 * w;
    double t14 = t13 - cc;
    int t15 = t14 * h;
    long t16 = t15 & 0xFFFF;
    float t17 = t16 * x;
    double t18 = t17 + dd;
    int t19 = t18 * i;
    float t20 = t19 * y;
    double t21 = t20 - ee;
    int t22 = t21 * j;
    long t23 = t22 ^ 0xAAAA;
    float t24 = t23 * z;
    double t25 = t24 + ff;
    
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
    
    /* For ARM (commented out, use appropriate one for target)
    asm volatile (
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
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "memory"
    );
    */
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec3 * t1;
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf3 * t3;
    v2df vecd3 = vecd1 + vecd2;
    v2df vecd4 = vecd3 * t4;
    
    /* Complex expression computed and used multiple times */
    int complex_val = (t5 * t6) + (t7 / 2) - (t8 & 0xFF);
    float float_val = complex_val * c + u - v;
    double double_val = float_val * d + aa - bb;
    
    /* Use the complex value in multiple statements */
    int use1 = complex_val * t9;
    float use2 = float_val * t10;
    double use3 = double_val * t11;
    
    /* Recompute similar expression later */
    int complex_val2 = (t12 * t13) + (t14 / 3) - (t15 & 0x7F);
    float float_val2 = complex_val2 * w + x - y;
    double double_val2 = float_val2 * cc + dd - ee;
    
    /* More operations using both computations */
    int result1 = use1 + complex_val2;
    float result2 = use2 + float_val2;
    double result3 = use3 + double_val2;
    
    /* Split into different basic blocks */
    if (result1 > 0) {
        result1 = complex_transform(result1, t16, result2, result3);
        vec4 = vector_transform(vec4, result1);
    } else {
        result1 = complex_transform(t17, result1, result3, result2);
        vec4 = vector_transform(vec3, result1);
    }
    
    /* Final mixing of all values */
    volatile int final_result = 
        result1 + 
        (int)result2 + 
        (int)result3 + 
        vec4[0] + vec4[1] + vec4[2] + vec4[3] +
        (int)vecf4[0] + (int)vecf4[1] + (int)vecf4[2] + (int)vecf4[3] +
        (int)vecd4[0] + (int)vecd4[1] +
        t25;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile int total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        input1 += i & 1;
        input2 -= i & 3;
        input3 *= 1.0001f;
        input4 /= 1.00001;
        
        int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Additional control flow */
        if (i % 100 == 0) {
            total ^= result;
        }
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
