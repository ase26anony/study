/* Main test driver with register pressure */
#include <stdint.h>
#include <stdio.h>

/* Force external linkage for helper functions */
extern int helper1(int a, int b, int c, int d, int e);
extern float helper2(float a, float b, float c);
extern double helper3(double a, double b, double c, double d);
extern long helper4(long a, long b, long c, long d, long e, long f);

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

/* Noinline to prevent optimization */
__attribute__((noinline))
struct MultiReg complex_calculation(int x, float y, double z, long w) {
    struct MultiReg result;
    
    /* Create many pseudo-registers through complex expressions */
    int t1 = x * 2;
    int t2 = t1 + x;
    int t3 = t2 * 3;
    int t4 = t3 - t1;
    int t5 = t4 / 2;
    int t6 = t5 | t3;
    int t7 = t6 & t4;
    int t8 = t7 ^ t5;
    int t9 = t8 << 2;
    int t10 = t9 >> 1;
    
    float f1 = y * 2.0f;
    float f2 = f1 + y;
    float f3 = f2 * 3.0f;
    float f4 = f3 - f1;
    float f5 = f4 / 2.0f;
    float f6 = f5 * f3;
    float f7 = f6 - f4;
    float f8 = f7 + f5;
    float f9 = f8 * 1.5f;
    float f10 = f9 / 0.5f;
    
    double d1 = z * 2.0;
    double d2 = d1 + z;
    double d3 = d2 * 3.0;
    double d4 = d3 - d1;
    double d5 = d4 / 2.0;
    double d6 = d5 * d3;
    double d7 = d6 - d4;
    double d8 = d7 + d5;
    double d9 = d8 * 1.5;
    double d10 = d9 / 0.5;
    
    long l1 = w * 2;
    long l2 = l1 + w;
    long l3 = l2 * 3;
    long l4 = l3 - l1;
    long l5 = l4 / 2;
    long l6 = l5 | l3;
    long l7 = l6 & l4;
    long l8 = l7 ^ l5;
    long l9 = l8 << 2;
    long l10 = l9 >> 1;
    
    /* Vector operations for wide register pressure */
    v4si vec1 = {t1, t2, t3, t4};
    v4si vec2 = {t5, t6, t7, t8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec3 - vec4;
    
    v4sf vecf1 = {f1, f2, f3, f4};
    v4sf vecf2 = {f5, f6, f7, f8};
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf1 * vecf2;
    v4sf vecf5 = vecf3 - vecf4;
    
    /* Artificial register pressure with inline assembly */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", 
                 "r6", "r7", "r8", "r9", "r10", "memory");
    
    /* Interdependent computations forcing serial evaluation */
    int a1 = t10 + (int)f10 + (int)d10 + (int)l10;
    int a2 = a1 * t9;
    int a3 = a2 + (int)f9;
    int a4 = a3 - (int)d9;
    int a5 = a4 | (int)l9;
    int a6 = a5 & t8;
    int a7 = a6 ^ (int)f8;
    int a8 = a7 + (int)d8;
    int a9 = a8 - (int)l8;
    int a10 = a9 * t7;
    
    float b1 = (float)a10 + f7;
    float b2 = b1 * (float)t6;
    float b3 = b2 + (float)d7;
    float b4 = b3 - (float)l7;
    float b5 = b4 / f6;
    float b6 = b5 * (float)t5;
    float b7 = b6 + (float)d6;
    float b8 = b7 - (float)l6;
    float b9 = b8 / f5;
    float b10 = b9 * (float)t4;
    
    double c1 = (double)b10 + d5;
    double c2 = c1 * (double)t3;
    double c3 = c2 + (double)l5;
    double c4 = c3 - (double)f4;
    double c5 = c4 / d4;
    double c6 = c5 * (double)t2;
    double c7 = c6 + (double)l4;
    double c8 = c7 - (double)f3;
    double c9 = c8 / d3;
    double c10 = c9 * (double)t1;
    
    long d1l = (long)c10 + l3;
    long d2l = d1l * (long)f2;
    long d3l = d2l + (long)t10;
    long d4l = d3l - (long)d2;
    long d5l = d4l / l2;
    long d6l = d5l * (long)f1;
    long d7l = d6l + (long)t9;
    long d8l = d7l - (long)d1;
    long d9l = d8l / l1;
    long d10l = d9l * (long)x;
    
    /* More artificial register pressure */
    asm volatile("" : : : "r11", "r12", "r13", "r14", "r15", 
                 "d0", "d1", "d2", "d3", "d4", "d5", "memory");
    
    /* Final computation using all temporaries */
    result.a = a10 + (int)b10 + (int)c10 + (int)d10l;
    result.b = t10 + f10 + d10 + l10;
    result.c = vec5[0] + vec5[1] + vec5[2] + vec5[3];
    result.d = (int)vecf5[0] + (int)vecf5[1] + (int)vecf5[2] + (int)vecf5[3];
    result.e = b10 + f10;
    result.f = (float)c10 + f10;
    result.g = c10 + d10;
    result.h = (double)d10l + d10;
    
    return result;
}

__attribute__((noinline))
int test_function(int seed) {
    /* Create register pressure with many local variables */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v2 + 1;
    int v4 = v3 * 3;
    int v5 = v4 - 2;
    int v6 = v5 / 2;
    int v7 = v6 | 0xFF;
    int v8 = v7 & 0x0F;
    int v9 = v8 ^ 0xAA;
    int v10 = v9 << 3;
    
    float fv1 = (float)seed * 1.5f;
    float fv2 = fv1 + 2.0f;
    float fv3 = fv2 * 2.0f;
    float fv4 = fv3 - 1.0f;
    float fv5 = fv4 / 3.0f;
    float fv6 = fv5 * 1.2f;
    float fv7 = fv6 + 0.5f;
    float fv8 = fv7 - 0.2f;
    float fv9 = fv8 * 1.1f;
    float fv10 = fv9 / 0.9f;
    
    double dv1 = (double)seed * 1.7;
    double dv2 = dv1 + 3.0;
    double dv3 = dv2 * 2.5;
    double dv4 = dv3 - 1.5;
    double dv5 = dv4 / 2.0;
    double dv6 = dv5 * 1.8;
    double dv7 = dv6 + 0.7;
    double dv8 = dv7 - 0.3;
    double dv9 = dv8 * 1.3;
    double dv10 = dv9 / 0.8;
    
    long lv1 = (long)seed * 3;
    long lv2 = lv1 + 5;
    long lv3 = lv2 * 4;
    long lv4 = lv3 - 3;
    long lv5 = lv4 / 2;
    long lv6 = lv5 | 0xFFFF;
    long lv7 = lv6 & 0xFF00;
    long lv8 = lv7 ^ 0xAAAA;
    long lv9 = lv8 << 2;
    long lv10 = lv9 >> 1;
    
    /* Critical pattern: pseudo-register used as both operand and destination */
    int temp = v10 + (int)fv10;
    int temp2 = temp * v9;
    temp = temp2 + (int)dv10;  // Reuse temp - creates pseudo-register pressure
    temp2 = temp - lv10;
    temp = temp2 | v8;
    temp2 = temp & (int)fv9;
    temp = temp2 ^ (int)dv9;
    temp2 = temp + lv9;
    temp = temp2 * v7;
    temp2 = temp - (int)fv8;
    
    /* Call helper functions for inter-procedural pressure */
    int h1 = helper1(v10, temp, temp2, v9, v8);
    float h2 = helper2(fv10, fv9, fv8);
    double h3 = helper3(dv10, dv9, dv8, dv7);
    long h4 = helper4(lv10, lv9, lv8, lv7, lv6, lv5);
    
    /* Complex expression chain */
    struct MultiReg mr = complex_calculation(temp, fv10, dv10, lv10);
    
    /* Final computation ensuring all values are used */
    int result = v10 + temp + temp2 + h1 + 
                 (int)fv10 + (int)h2 + 
                 (int)dv10 + (int)h3 + 
                 (int)lv10 + (int)h4 +
                 mr.a + mr.b + mr.c + mr.d +
                 (int)mr.e + (int)mr.f +
                 (int)mr.g + (int)mr.h;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

int main() {
    int total = 0;
    int iterations = loop_counter;
    
    /* Hot loop to trigger optimization */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
