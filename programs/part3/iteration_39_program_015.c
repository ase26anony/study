/* Main test driver with hot loop to induce register pressure */
#include <stdint.h>
#include <stdio.h>

/* External helper functions from second compilation unit */
extern int helper1(int a, int b, int c, int d, int e);
extern float helper2(float a, float b, float c, float d);
extern double helper3(double a, double b, double c);
extern long helper4(long a, long b, long c, long d, long e, long f);

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static int complex_expression(int a, int b, int c, int d, int e) {
    /* Create many intermediate values with dependencies */
    int t1 = a + b;
    int t2 = b * c;
    int t3 = t1 - t2;
    int t4 = c ^ d;
    int t5 = t3 | t4;
    int t6 = t2 & t5;
    int t7 = t6 << 2;
    int t8 = t7 >> 1;
    int t9 = t8 + e;
    int t10 = t9 * 3;
    int t11 = t10 / 2;
    int t12 = t11 % 7;
    int t13 = t12 ^ t5;
    int t14 = t13 | t4;
    int t15 = t14 & t3;
    int t16 = t15 + t1;
    int t17 = t16 - t2;
    int t18 = t17 * t3;
    int t19 = t18 / 4;
    int t20 = t19 % 11;
    
    /* Force serial evaluation with dependencies */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory");
    
    return t20;
}

/* Test function with maximum register pressure */
__attribute__((noinline))
static int test_function(int seed) {
    /* Declare many variables of different types */
    int a = seed;
    int b = seed + 1;
    int c = seed * 2;
    int d = seed - 3;
    int e = seed ^ 0xFF;
    int f = seed | 0xAA;
    int g = seed & 0x55;
    int h = seed << 2;
    int i = seed >> 1;
    int j = ~seed;
    
    float fa = seed * 1.5f;
    float fb = seed * 2.5f;
    float fc = seed * 3.5f;
    float fd = seed * 4.5f;
    
    double da = seed * 1.25;
    double db = seed * 2.25;
    double dc = seed * 3.25;
    
    long la = seed * 100L;
    long lb = seed * 200L;
    long lc = seed * 300L;
    long ld = seed * 400L;
    
    /* Vector types for additional pressure */
    typedef int v4si __attribute__((vector_size(16)));
    v4si va = {seed, seed + 1, seed + 2, seed + 3};
    v4si vb = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si vc = {seed * 6, seed * 7, seed * 8, seed * 9};
    
    /* Chain of interdependent computations */
    a = b + c;
    d = a * e;
    f = d - g;
    h = f ^ i;
    j = h | a;
    
    /* Mixed-type operations */
    fa = fb * fc;
    fd = fa / fb;
    
    da = db + dc;
    db = da * 1.1;
    
    la = lb + lc;
    ld = la * 2L;
    
    /* Vector operations */
    va = vb + vc;
    vb = va * 2;
    vc = vb - va;
    
    /* More complex chains */
    int t1 = complex_expression(a, b, c, d, e);
    int t2 = complex_expression(f, g, h, i, j);
    int t3 = complex_expression(t1, t2, a, b, c);
    
    /* Use helper functions for inter-procedural pressure */
    int h1 = helper1(a, b, c, d, e);
    float h2 = helper2(fa, fb, fc, fd);
    double h3 = helper3(da, db, dc);
    long h4 = helper4(la, lb, lc, ld, t1, t2);
    
    /* Final computation using all temporaries */
    int result = t1 + t2 + t3 + h1 + (int)h2 + (int)h3 + (int)h4;
    
    /* Extract vector elements */
    int vsum = va[0] + va[1] + va[2] + va[3] +
               vb[0] + vb[1] + vb[2] + vb[3] +
               vc[0] + vc[1] + vc[2] + vc[3];
    
    result += vsum;
    
    /* Artificial clobber to reduce available registers */
    asm volatile("" : : : 
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "memory");
    
    return result;
}

int main() {
    int total = 0;
    int iterations = loop_counter;
    
    /* Hot loop to trigger early rematerialization */
    for (int i = 0; i < iterations; i++) {
        total += test_function(i);
        total += test_function(i * 2);
        total += test_function(i * 3);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
