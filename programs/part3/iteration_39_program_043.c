/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations */
extern int test_function(void);
extern struct LargeStruct helper_func1(int a, int b, float c, double d);
extern struct LargeStruct helper_func2(long a, double b, int c, float d);
extern double helper_func3(struct LargeStruct s1, struct LargeStruct s2);

/* Volatile to prevent optimization */
volatile int g_iterations = 100;

/* Complex struct to force register pressure */
struct LargeStruct {
    double d1, d2;
    float f1, f2, f3;
    int i1, i2, i3;
    long l1;
};

/* Vector types for additional pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Noinline helper functions to prevent inlining */
__attribute__((noinline)) 
struct LargeStruct helper_func1(int a, int b, float c, double d) {
    struct LargeStruct s;
    s.d1 = d + a;
    s.d2 = d * b;
    s.f1 = c + a;
    s.f2 = c * b;
    s.f3 = s.f1 + s.f2;
    s.i1 = a + b;
    s.i2 = a * b;
    s.i3 = s.i1 - s.i2;
    s.l1 = (long)a * b;
    return s;
}

__attribute__((noinline))
struct LargeStruct helper_func2(long a, double b, int c, float d) {
    struct LargeStruct s;
    s.d1 = b + a;
    s.d2 = b * c;
    s.f1 = d + a;
    s.f2 = d * c;
    s.f3 = s.f1 - s.f2;
    s.i1 = c + (int)a;
    s.i2 = c * (int)a;
    s.i3 = s.i1 ^ s.i2;
    s.l1 = a << 2;
    return s;
}

__attribute__((noinline))
double helper_func3(struct LargeStruct s1, struct LargeStruct s2) {
    double sum = 0.0;
    sum += s1.d1 * s2.d1;
    sum += s1.d2 * s2.d2;
    sum += s1.f1 * s2.f1;
    sum += s1.f2 * s2.f2;
    sum += s1.f3 * s2.f3;
    sum += s1.i1 * s2.i1;
    sum += s1.i2 * s2.i2;
    sum += s1.i3 * s2.i3;
    sum += (double)s1.l1 * s2.l1;
    return sum;
}

/* Main test function with high register pressure */
__attribute__((noinline))
int test_function(void) {
    /* Many local variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    
    /* Vector variables */
    v4si vec1 = {v1, v2, v3, v4};
    v4si vec2 = {v2, v3, v4, v5};
    v4sf vecf1 = {f1, f2, f3, f4};
    v4sf vecf2 = {f2, f3, f4, f5};
    v2df vecd1 = {d1, d2};
    v2df vecd2 = {d2, d3};
    
    /* Complex interdependent computations */
    int t1 = v1 + v2;
    int t2 = t1 * v3;
    float t3 = f1 * f2 + f3;
    double t4 = d1 / d2 * d3;
    long t5 = l1 ^ l2 | l3;
    
    /* Chain of dependent operations */
    t1 = t2 + v4 - t1;
    t2 = t1 * v5 / 2;
    t3 = t3 * f4 - f5;
    t4 = t4 + d4 * d5;
    t5 = t5 & l4 | l5;
    
    /* More temporaries with cross-type operations */
    int t6 = (int)(t3 * 10) + t1;
    float t7 = (float)t2 / 3.14f + t3;
    double t8 = (double)t5 * 0.01 + t4;
    long t9 = (long)t6 * t2 + t5;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4sf vecf3 = vecf1 * vecf2;
    v4sf vecf4 = vecf1 + vecf2;
    v2df vecd3 = vecd1 * vecd2;
    v2df vecd4 = vecd1 + vecd2;
    
    /* Extract elements from vectors */
    int t10 = vec3[0] + vec3[1];
    int t11 = vec4[2] * vec4[3];
    float t12 = vecf3[0] + vecf3[1];
    float t13 = vecf4[2] * vecf4[3];
    double t14 = vecd3[0] / vecd3[1];
    double t15 = vecd4[0] + vecd4[1];
    
    /* Call helper functions for inter-procedural pressure */
    struct LargeStruct s1 = helper_func1(t1, t2, t3, t4);
    struct LargeStruct s2 = helper_func2(t5, t8, t6, t7);
    
    double t16 = helper_func3(s1, s2);
    
    /* Artificial register pressure with inline asm */
    asm volatile (
        "/* Clobber many registers to force pseudo-reg usage */\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* More dependent computations after asm */
    int t17 = t10 + t11 - t6;
    float t18 = t12 * t13 / t7;
    double t19 = t14 + t15 + t16;
    long t20 = t9 ^ t17;
    
    /* Final complex expression using all temporaries */
    int result = (int)((t1 + t2 + t6 + t10 + t11 + t17) * 
                      (t3 + t7 + t12 + t13 + t18) / 
                      (t4 + t8 + t14 + t15 + t16 + t19) + 
                      (t5 + t9 + t20));
    
    /* Ensure all variables are used */
    asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                      "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
                      "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5),
                      "r"(l1), "r"(l2), "r"(l3), "r"(l4), "r"(l5));
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < g_iterations; i++) {
        total += test_function();
        
        /* Volatile store to prevent loop optimization */
        *(volatile int*)&g_iterations = g_iterations;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
