/* main.c - Primary test file to induce register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* Force pseudo-register creation through complex expressions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Noinline functions to prevent optimization and increase register pressure */
__attribute__((noinline)) 
int helper1(int a, int b, int c, int d, int e, int f) {
    return ((a * b) + (c * d) - (e * f)) ^ (a + b + c + d + e + f);
}

__attribute__((noinline))
float helper2(float a, float b, float c, float d, float e, float f) {
    return (a * b) + (c * d) - (e * f) + (a + b + c + d + e + f);
}

__attribute__((noinline))
double helper3(double a, double b, double c, double d) {
    return (a * b * c * d) + (a + b + c + d) - (a * b) + (c * d);
}

__attribute__((noinline))
long helper4(long a, long b, long c, long d, long e) {
    return (a << 3) | (b << 2) | (c << 1) | d | e;
}

__attribute__((noinline))
v4si vector_op1(v4si a, v4si b, v4si c) {
    return a + b * c - (a & b) | (c << 1);
}

__attribute__((noinline))
v4sf vector_op2(v4sf a, v4sf b, v4sf c) {
    return a * b + c / a - b * c;
}

/* Complex struct to force register pressure across function boundaries */
struct MultiRegStruct {
    int a, b, c, d;
    float e, f;
    double g;
    long h;
};

__attribute__((noinline))
struct MultiRegStruct create_struct(int a, float b, double c, long d) {
    struct MultiRegStruct s;
    s.a = a * 2;
    s.b = a + 5;
    s.c = a ^ 0xFF;
    s.d = a >> 3;
    s.e = b * 2.0f;
    s.f = b + 3.14f;
    s.g = c * 1.5;
    s.h = d << 2;
    return s;
}

__attribute__((noinline))
int use_struct(struct MultiRegStruct s1, struct MultiRegStruct s2) {
    return s1.a + s1.b + s1.c + s1.d + 
           (int)s1.e + (int)s1.f + (int)s1.g + (int)s1.h +
           s2.a + s2.b + s2.c + s2.d;
}

/* Main test function with dense register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
int test_function(int seed) {
    /* Declare many variables of different types to use pseudo-registers */
    int t1 = seed;
    int t2 = t1 + 1;
    int t3 = t2 * 2;
    int t4 = t3 ^ 0xABCD;
    int t5 = t4 >> 3;
    int t6 = t5 << 2;
    int t7 = t6 | 0xFF;
    int t8 = t7 & 0x7F;
    int t9 = t8 + t1;
    int t10 = t9 - t2;
    
    float f1 = (float)t1 * 1.1f;
    float f2 = f1 + 2.2f;
    float f3 = f2 * 3.3f;
    float f4 = f3 / 4.4f;
    float f5 = f4 - f1;
    float f6 = f5 + f2;
    
    double d1 = (double)t2 * 1.111;
    double d2 = d1 + 2.222;
    double d3 = d2 * 3.333;
    double d4 = d3 / 4.444;
    double d5 = d4 - d1;
    double d6 = d5 + d2;
    
    long l1 = (long)t3 * 1000L;
    long l2 = l1 + 2000L;
    long l3 = l2 * 3000L;
    long l4 = l3 / 4000L;
    long l5 = l4 - l1;
    long l6 = l5 | l2;
    
    /* Vector operations for wide register pressure */
    v4si vec1 = {t1, t2, t3, t4};
    v4si vec2 = {t5, t6, t7, t8};
    v4si vec3 = {t9, t10, t1, t2};
    
    v4sf vecf1 = {f1, f2, f3, f4};
    v4sf vecf2 = {f5, f6, f1, f2};
    v4sf vecf3 = {f3, f4, f5, f6};
    
    /* Complex interdependent computations */
    int r1 = helper1(t1, t2, t3, t4, t5, t6);
    float r2 = helper2(f1, f2, f3, f4, f5, f6);
    double r3 = helper3(d1, d2, d3, d4);
    long r4 = helper4(l1, l2, l3, l4, l5);
    
    /* Force pseudo-register to be used as both operand and destination */
    int chain1 = r1 + t1;
    int chain2 = chain1 * t2;  /* chain1 used as operand, chain2 as destination */
    int chain3 = chain2 - t3;
    int chain4 = chain3 ^ t4;
    int chain5 = chain4 | t5;
    int chain6 = chain5 & t6;
    int chain7 = chain6 + chain1;  /* Multiple uses of chain variables */
    int chain8 = chain7 * chain2;
    int chain9 = chain8 - chain3;
    int chain10 = chain9 ^ chain4;
    
    /* More chaining with different types */
    float fchain1 = r2 + f1;
    float fchain2 = fchain1 * f2;
    float fchain3 = fchain2 - f3;
    float fchain4 = fchain3 / f4;
    float fchain5 = fchain4 + fchain1;
    float fchain6 = fchain5 * fchain2;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile (
        "/* Clobber many registers to force pseudo-register usage */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        : 
        : "r" (chain1), "r" (chain2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Vector operations mixed with scalar */
    v4si vec_result1 = vector_op1(vec1, vec2, vec3);
    v4sf vec_result2 = vector_op2(vecf1, vecf2, vecf3);
    
    /* Struct operations for inter-procedural pressure */
    struct MultiRegStruct s1 = create_struct(chain1, fchain1, d1, l1);
    struct MultiRegStruct s2 = create_struct(chain2, fchain2, d2, l2);
    int struct_result = use_struct(s1, s2);
    
    /* Final computation using all temporaries */
    int final_result = 
        chain1 + chain2 + chain3 + chain4 + chain5 +
        chain6 + chain7 + chain8 + chain9 + chain10 +
        (int)fchain1 + (int)fchain2 + (int)fchain3 +
        (int)fchain4 + (int)fchain5 + (int)fchain6 +
        (int)r2 + (int)r3 + (int)r4 +
        vec_result1[0] + vec_result1[1] + vec_result1[2] + vec_result1[3] +
        (int)vec_result2[0] + (int)vec_result2[1] +
        struct_result;
    
    /* Force serial evaluation with volatile memory barrier */
    asm volatile ("" ::: "memory");
    
    return final_result ^ seed;  /* Ensure all computations are used */
}

int main() {
    int total = 0;
    volatile int iterations = g_iterations;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        /* Mix different seeds to prevent pattern recognition */
        int seed = i ^ 0xDEADBEEF;
        total += test_function(seed);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile ("nop" :::);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
