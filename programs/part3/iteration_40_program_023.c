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

__attribute__((noinline, noclone))
static double vector_reduce(v4sf v) {
    double sum = 0.0;
    sum += v[0] + v[1] + v[2] + v[3];
    return sum;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* More integer variables */
    int e = a * 2;
    int f = b % 100;
    int g = e + f;
    int h = g - a;
    int i = h * 3;
    int j = i / 2;
    int k = j | 0xFF;
    int l = k & 0x0F;
    int m = l ^ 0x55;
    int n = m << 2;
    int o = n >> 1;
    
    /* More long variables */
    long p = b + 1000;
    long q = p - 500;
    long r = q * 2;
    long s = r / 3;
    long t = s % 100;
    long u = t | 0xFFFF;
    long v = u & 0xFF00;
    
    /* Floating point variables */
    float w = c * 2.0f;
    float x = w + 1.5f;
    float y = x / 3.0f;
    float z = y - 0.5f;
    float aa = z * c;
    float ab = aa + w;
    
    /* Double variables */
    double ac = d * 1.5;
    double ad = ac + 2.0;
    double ae = ad / 3.0;
    double af = ae - 1.0;
    double ag = af * d;
    double ah = ag + ac;
    
    /* Vector variables */
    v4si vec1 = {a, e, g, i};
    v4si vec2 = {f, h, j, k};
    v4sf vec3 = {c, w, x, y};
    v4sf vec4 = {z, aa, ab, 1.0f};
    v2df vec5 = {d, ac};
    v2df vec6 = {ad, ae};
    
    /* Long serial chain of interdependent operations */
    int t1 = a + e;
    int t2 = t1 * f;
    int t3 = t2 - g;
    int t4 = t3 / h;
    int t5 = t4 | i;
    int t6 = t5 & j;
    int t7 = t6 ^ k;
    int t8 = t7 << l;
    int t9 = t8 >> m;
    
    long t10 = b + p;
    long t11 = t10 * q;
    long t12 = t11 - r;
    long t13 = t12 / s;
    long t14 = t13 | t;
    long t15 = t14 & u;
    
    float t16 = c + w;
    float t17 = t16 * x;
    float t18 = t17 - y;
    float t19 = t18 / z;
    
    double t20 = d + ac;
    double t21 = t20 * ad;
    double t22 = t21 - ae;
    double t23 = t22 / af;
    
    /* Vector operations */
    v4si t24 = vec1 + vec2;
    v4si t25 = t24 * vec1;
    v4sf t26 = vec3 + vec4;
    v4sf t27 = t26 * vec3;
    v2df t28 = vec5 + vec6;
    v2df t29 = t28 * vec5;
    
    /* Inline assembly to clobber registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Control flow to create multiple basic blocks */
    int result;
    if (t9 > 1000) {
        result = compute_branch(t1, t2, t3);
        
        /* Recompute similar expression later in same block */
        int t9_recomputed = (t8 >> m) + 1;  /* Slightly different recomputation */
        result += t9_recomputed;
    } else if (t9 > 500) {
        switch (t9 % 4) {
            case 0:
                result = t4 + t5;
                break;
            case 1:
                result = t6 - t7;
                break;
            case 2:
                result = t8 * 2;
                break;
            default:
                result = t9 / 2;
                break;
        }
        
        /* Another recomputation */
        int t5_recomputed = (t4 | i) ^ 1;
        result ^= t5_recomputed;
    } else {
        result = t1 + t2 + t3 + t4;
        
        /* Complex expression recomputation */
        int t3_recomputed = (t1 * f) - g + 1;
        result *= t3_recomputed;
    }
    
    /* More operations after control flow */
    double t30 = t23 + ah;
    float t31 = t19 + ab;
    long t32 = t15 + v;
    
    /* Vector reduction */
    double vec_sum = vector_reduce(t27);
    
    /* Final computation using many temporaries */
    volatile long final_result = 
        (long)(result + t9 + t32) + 
        (long)(t30 * 100) + 
        (long)(t31 * 10) + 
        (long)vec_sum;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile long total = 0;
    
    /* Create varying inputs to prevent constant propagation */
    volatile int base1 = 42;
    volatile long base2 = 123456789;
    volatile float base3 = 3.14159f;
    volatile double base4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int input1 = base1 + (i % 100);
        volatile long input2 = base2 + i;
        volatile float input3 = base3 + (i * 0.01f);
        volatile double input4 = base4 + (i * 0.001);
        
        /* Call the test function */
        total += test_remat(input1, input2, input3, input4);
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i));
    }
    
    printf("Final result: %ld\n", total);
    return (int)(total % 1000);
}
