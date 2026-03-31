/* Main test driver with high register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
volatile int g_iterations = 100;

/* Prevent optimization of results */
volatile int g_sink;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for parameter passing */
struct LargeStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long extra[4];
};

/* External helper functions */
struct LargeStruct __attribute__((noinline)) helper1(int a, float b, double c, v4si v);
struct LargeStruct __attribute__((noinline)) helper2(struct LargeStruct s1, struct LargeStruct s2);
int __attribute__((noinline)) helper3(struct LargeStruct s);

/* Main test function with extreme register pressure */
int __attribute__((noinline,optimize("O3"))) 
test_function(int seed) {
    /* Declare many variables of different types */
    int i1 = seed + 1;
    int i2 = seed * 2;
    int i3 = seed / 3;
    int i4 = seed - 4;
    int i5 = seed % 5;
    int i6 = seed << 2;
    int i7 = seed >> 1;
    int i8 = seed | 0xFF;
    int i9 = seed & 0x0F;
    int i10 = seed ^ 0x55;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    float f3 = seed * 3.3f;
    float f4 = seed * 4.4f;
    float f5 = seed * 5.5f;
    
    double d1 = seed * 1.11;
    double d2 = seed * 2.22;
    double d3 = seed * 3.33;
    double d4 = seed * 4.44;
    double d5 = seed * 5.55;
    
    long l1 = seed * 1000L;
    long l2 = seed * 2000L;
    long l3 = seed * 3000L;
    long l4 = seed * 4000L;
    
    /* Vector variables */
    v4si v1 = {i1, i2, i3, i4};
    v4si v2 = {i5, i6, i7, i8};
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = {f5, f1, f2, f3};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Complex interdependent computations */
    /* Chain 1: Integer operations with many pseudo-registers */
    i1 = i2 + i3;
    i2 = i1 * i4;      /* i1 used as operand and destination in chain */
    i3 = i2 - i5;
    i4 = i3 / i6;
    i5 = i4 | i7;
    i6 = i5 & i8;
    i7 = i6 ^ i9;
    i8 = i7 << 2;
    i9 = i8 >> 1;
    i10 = i9 % 11;
    
    /* Chain 2: Floating point operations */
    f1 = f2 + f3;
    f2 = f1 * f4;      /* f1 used as operand and destination */
    f3 = f2 - f5;
    f4 = f3 / f1;
    f5 = f4 * 2.0f;
    
    /* Chain 3: Mixed operations */
    d1 = d2 + f1;
    d2 = d1 * f2;
    d3 = d2 - f3;
    d4 = d3 / f4;
    d5 = d4 * 2.0;
    
    /* Vector operations */
    v1 = v1 + v2;
    v2 = v1 * v2;
    vf1 = vf1 + vf2;
    vf2 = vf1 * vf2;
    vd1 = vd1 + vd2;
    vd2 = vd1 * vd2;
    
    /* Long operations */
    l1 = l2 + l3;
    l2 = l1 * l4;
    l3 = l2 - l1;
    l4 = l3 / 1000L;
    
    /* Create artificial register pressure with inline assembly */
    /* Clobber many registers to force spilling */
    asm volatile (
        "/* Clobber physical registers */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r2, r0, r1\n\t"
        :
        : "r" (i1), "r" (i2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More complex chains with function calls */
    struct LargeStruct s1 = helper1(i1, f1, d1, v1);
    struct LargeStruct s2 = helper1(i2, f2, d2, v2);
    
    /* Use results immediately to create register pressure */
    i1 = helper3(s1);
    i2 = helper3(s2);
    
    struct LargeStruct s3 = helper2(s1, s2);
    
    /* Final computation using all variables */
    int result = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result += (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    /* Use vector results */
    int *vp = (int*)&v1;
    for (int i = 0; i < 4; i++) result += vp[i];
    
    vp = (int*)&v2;
    for (int i = 0; i < 4; i++) result += vp[i];
    
    result += helper3(s3);
    
    return result;
}

int main() {
    int total = 0;
    int iterations = g_iterations;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Vary input to prevent constant propagation */
        int result = test_function(i + total);
        total += result;
        
        /* Prevent dead code elimination */
        g_sink = total;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
