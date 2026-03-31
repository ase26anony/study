/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* Forward declarations */
extern int test_function(void);
extern struct MultiReg helper1(int a, int b, int c, int d);
extern struct MultiReg helper2(struct MultiReg x, struct MultiReg y);
extern double helper3(float a, float b, double c, double d);

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for parameter passing */
struct MultiReg {
    long long a;
    double b;
    v4si c;
    float d;
};

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static int create_pseudo_pressure(int base) {
    /* Many interdependent operations */
    int t1 = base * 3;
    int t2 = t1 + 7;
    int t3 = t2 ^ 0x55AA55AA;
    int t4 = t3 * t1;
    int t5 = t4 - t2;
    int t6 = t5 >> 3;
    int t7 = t6 | t3;
    int t8 = t7 & 0x00FF00FF;
    int t9 = t8 * t4;
    int t10 = t9 / (t1 + 1);
    
    /* Mix with floating point */
    float f1 = t1 * 0.5f;
    float f2 = t2 * 0.25f;
    float f3 = f1 + f2;
    float f4 = f3 * f1;
    float f5 = f4 - f2;
    
    /* Double precision */
    double d1 = t3 * 0.333;
    double d2 = t4 * 0.666;
    double d3 = d1 + d2;
    double d4 = d3 * d1;
    double d5 = d4 - d2;
    
    /* Vector operations */
    v4si v1 = {t1, t2, t3, t4};
    v4si v2 = {t5, t6, t7, t8};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    
    /* Artificial register clobbering */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* Complex return mixing all types */
    return t10 + (int)f5 + (int)d5 + v5[0] + v5[1] + v5[2] + v5[3];
}

/* Main test function with register pressure */
__attribute__((noinline))
int test_function(void) {
    /* Many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f;
    double da = 1.11, db = 2.22, dc = 3.33, dd = 4.44;
    long la = 1000L, lb = 2000L, lc = 3000L, ld = 4000L;
    
    /* Chain of interdependent computations */
    a = b + c;          /* Creates pseudo-reg for a */
    d = a * e;          /* Uses a, creates pseudo-reg for d */
    f = d - g;          /* Uses d, creates pseudo-reg for f */
    h = f ^ a;          /* Uses f and a, creates pseudo-reg for h */
    
    /* Floating point chain */
    fa = fb * fc;
    fd = fa + fb;
    fc = fd - fa;
    fb = fc * fd;
    
    /* Double precision chain */
    da = db + dc;
    dd = da * db;
    dc = dd - da;
    db = dc / dd;
    
    /* Long integer chain */
    la = lb * lc;
    ld = la + lb;
    lc = ld - la;
    lb = lc ^ ld;
    
    /* Vector operations */
    v4si v1 = {a, b, c, d};
    v4si v2 = {e, f, g, h};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    v4si v6 = v5 | v1;
    v4si v7 = v6 ^ v2;
    
    v4sf vf1 = {fa, fb, fc, fd};
    v4sf vf2 = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf vf3 = vf1 * vf2;
    v4sf vf4 = vf1 + vf2;
    v4sf vf5 = vf3 - vf4;
    
    v2df vd1 = {da, db};
    v2df vd2 = {dc, dd};
    v2df vd3 = vd1 * vd2;
    v2df vd4 = vd1 + vd2;
    v2df vd5 = vd3 - vd4;
    
    /* More complex expressions with mixing */
    int t1 = a + (int)fa;
    int t2 = t1 * (int)da;
    int t3 = t2 + la;
    int t4 = t3 ^ v5[0];
    int t5 = t4 * v6[1];
    int t6 = t5 + (int)vf3[0];
    int t7 = t6 * (int)vd3[0];
    int t8 = t7 - v7[2];
    int t9 = t8 ^ v5[3];
    int t10 = t9 * a;
    
    /* Function calls for inter-procedural pressure */
    struct MultiReg mr1 = helper1(a, b, c, d);
    struct MultiReg mr2 = helper1(e, f, g, h);
    struct MultiReg mr3 = helper2(mr1, mr2);
    
    double complex_result = helper3(fa, fb, da, db);
    
    /* Artificial register pressure */
    asm volatile("" : : : 
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "memory");
    
    /* Use all variables in final computation */
    int result = t10 + 
                 (int)(fa + fb + fc + fd) +
                 (int)(da + db + dc + dd) +
                 la + lb + lc + ld +
                 v5[0] + v5[1] + v5[2] + v5[3] +
                 v6[0] + v6[1] + v6[2] + v6[3] +
                 v7[0] + v7[1] + v7[2] + v7[3] +
                 (int)vf3[0] + (int)vf3[1] + (int)vf3[2] + (int)vf3[3] +
                 (int)vd3[0] + (int)vd3[1] +
                 (int)mr3.a + (int)mr3.b +
                 (int)complex_result;
    
    /* Call pseudo-register pressure function */
    result += create_pseudo_pressure(result);
    
    return result;
}

int main(void) {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation unit pressure */
    for (int i = 0; i < iterations; i++) {
        total += test_function();
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
