/* early-remat-test.c */
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
    if (a > b) {
        return a * c - b;
    } else {
        return b * c + a;
    }
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    switch ((int)a % 3) {
        case 0: return a * b + c;
        case 1: return a / b - c;
        case 2: return a + b * c;
        default: return a - b + c;
    }
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si result;
    for (int i = 0; i < 4; i++) {
        if (a[i] > 0) {
            result[i] = a[i] * b[i];
        } else {
            result[i] = a[i] + b[i];
        }
    }
    return result;
}

/* Main test function implementing all requirements */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Requirement 1: Many local variables of mixed types */
    int a = input1 + 1;
    long b = input2 - 2;
    float c = input3 * 3.0f;
    double d = input4 / 4.0;
    int e = a * 5;
    long f = b / 6;
    float g = c + 7.0f;
    double h = d - 8.0;
    int i = e ^ 0x55;
    long j = f | 0xAA;
    float k = g * 2.5f;
    double l = h / 1.5;
    int m = i << 2;
    long n = j >> 1;
    float o = k + 10.0f;
    double p = l - 20.0;
    int q = m & 0xFF;
    long r = n ^ 0xFFFF;
    float s = o / 3.0f;
    double t = p * 4.0;
    int u = q % 17;
    long v = r % 23;
    float w = s * s;
    double x = t * t;
    int y = u * u;
    long z = v * v;
    float aa = w + w;
    double bb = x + x;
    int cc = y | z;
    long dd = z & y;
    float ee = aa - bb;
    double ff = bb - aa;
    
    /* Vector variables for Requirement 4 */
    v4si vec1 = {a, e, i, m};
    v4si vec2 = {b & 0xFF, f & 0xFF, j & 0xFF, n & 0xFF};
    v4sf vec3 = {c, g, k, o};
    v4sf vec4 = {d, h, l, p};
    v2df vec5 = {d * 2.0, h * 2.0};
    v2df vec6 = {l / 2.0, p / 2.0};
    
    /* Requirement 2: Complex expression computed and reused */
    int complex_expr = ((a * b) + (e * f) - (i * j)) / ((m + n) ? (m + n) : 1);
    float float_expr = (c * d) + (g * h) - (k * l) / ((o + p) ? (o + p) : 1.0f);
    
    /* Requirement 5: Control flow splitting */
    if (complex_expr > 1000) {
        complex_expr = helper1(a, e, i);
        float_expr = helper2(c, g, k);
    } else if (complex_expr > 500) {
        complex_expr = helper1(e, i, m);
        float_expr = helper2(g, k, o);
    } else {
        complex_expr = helper1(i, m, q);
        float_expr = helper2(k, o, s);
    }
    
    /* Vector operations */
    vec1 = helper3(vec1, vec2);
    vec3 = vec3 + vec4;
    vec5 = vec5 - vec6;
    
    /* Requirement 3: Inline assembly clobbering registers */
    /* x86_64 version */
    asm volatile (
        "nop\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* ARM version (commented out, use appropriate one for target)
    asm volatile (
        "nop\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
          "memory"
    );
    */
    
    /* Requirement 2 continued: Recomputation in slightly different form */
    int complex_expr2 = ((a * b) - (e * f) + (i * j)) / ((m - n) ? (m - n) : 1);
    float float_expr2 = (c / d) + (g / h) - (k / l) * ((o - p) ? (o - p) : 1.0f);
    
    /* More interdependent operations */
    int t1 = complex_expr + complex_expr2;
    long t2 = t1 * b;
    float t3 = float_expr + float_expr2;
    double t4 = t3 * d;
    int t5 = t1 ^ t2;
    long t6 = t2 | t5;
    float t7 = t3 / t4;
    double t8 = t4 * t7;
    
    /* More vector operations */
    v4si vec7 = vec1 * 2;
    v4sf vec8 = vec3 * 2.0f;
    v2df vec9 = vec5 + vec6;
    
    /* Use all variables to ensure they're live */
    int result = a + e + i + m + q + u + y + cc + t1 + t5;
    result += vec1[0] + vec1[1] + vec1[2] + vec1[3];
    result += (int)vec3[0] + (int)vec3[1] + (int)vec3[2] + (int)vec3[3];
    result += (int)vec5[0] + (int)vec5[1];
    result += vec7[0] + vec7[1];
    result += (int)vec8[0] + (int)vec8[1];
    result += (int)vec9[0] + (int)vec9[1];
    
    result += (int)(c + g + k + o + s + w + aa + ee + t3 + t7);
    result += (int)(b + f + j + n + r + v + z + dd + t2 + t6);
    result += (int)(d + h + l + p + t + x + bb + ff + t4 + t8);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile int total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int input1 = input_seed + i;
        volatile long input2 = input_seed * 3L - i;
        volatile float input3 = (float)input_seed / (i + 1) + i;
        volatile double input4 = (double)input_seed * 2.0 / (i + 2) - i;
        
        total += test_remat(input1, input2, input3, input4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return 0;
}
