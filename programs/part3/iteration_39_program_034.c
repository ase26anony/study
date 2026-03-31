/* Test to trigger early rematerialization pseudo-register replacement logic */
/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -c early-remat-test.c */
/* Link with: gcc -O2 -fearly-remat early-remat-test.o early-remat-helper.o */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization of critical variables */
volatile int g_volatile_counter = 1000;

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* External helper functions from second compilation unit */
extern struct MultiArg __attribute__((noinline)) 
helper_func1(int a, double b, long c, float d);
extern v4si __attribute__((noinline)) 
helper_func2(v4si a, v4si b, v4sf c);
extern struct LargeStruct __attribute__((noinline))
helper_func3(struct LargeStruct s1, struct LargeStruct s2);

/* Structs for by-value passing to increase register pressure */
struct MultiArg {
    int x;
    double y;
    long z;
    float w;
};

struct LargeStruct {
    v4si vec1;
    v2df vec2;
    int scalar1;
    double scalar2;
    long scalar3;
};

/* Force register clobbering */
#define CLOBBER_REGS() \
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                 "r8", "r9", "r10", "r11", "r12", "memory")

/* Complex test function with massive register pressure */
__attribute__((noinline))
static long test_function(int seed) {
    /* Declare many variables of different types to force pseudo-registers */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 4;
    int e = seed % 5;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed ^ 0xFF;
    int i = seed | 0xAA;
    int j = seed & 0x55;
    
    float fa = seed * 1.1f;
    float fb = seed * 2.2f;
    float fc = seed * 3.3f;
    float fd = seed * 4.4f;
    float fe = seed * 5.5f;
    
    double da = seed * 1.111;
    double db = seed * 2.222;
    double dc = seed * 3.333;
    double dd = seed * 4.444;
    double de = seed * 5.555;
    
    long la = seed * 1000L;
    long lb = seed * 2000L;
    long lc = seed * 3000L;
    long ld = seed * 4000L;
    long le = seed * 5000L;
    
    /* Vector variables - use wide registers */
    v4si va = {a, b, c, d};
    v4si vb = {e, f, g, h};
    v4si vc = {i, j, a, b};
    
    v4sf vfa = {fa, fb, fc, fd};
    v4sf vfb = {fe, fa * 2, fb * 2, fc * 2};
    
    v2df vda = {da, db};
    v2df vdb = {dc, dd};
    
    /* Create complex interdependencies to force serial evaluation */
    /* Chain 1: Integer operations with pseudo-register reuse */
    int t1 = a + b;      /* Pseudo-reg for t1 created */
    int t2 = t1 * c;     /* t1 used here, new pseudo-reg for t2 */
    int t3 = t2 - d;     /* t2 used here, t1 potentially dead */
    int t4 = t3 / e;     /* t3 used here, t2 potentially dead */
    int t5 = t4 ^ f;     /* Chain continues... */
    int t6 = t5 | g;
    int t7 = t6 & h;
    int t8 = t7 << 2;
    int t9 = t8 >> 1;
    int t10 = t9 % 13;
    
    /* Chain 2: Floating point with mixed operations */
    float ft1 = fa + fb;
    float ft2 = ft1 * fc;
    float ft3 = ft2 - fd;
    float ft4 = ft3 / fe;
    float ft5 = ft4 * fa;
    
    /* Chain 3: Double precision */
    double dt1 = da + db;
    double dt2 = dt1 * dc;
    double dt3 = dt2 - dd;
    double dt4 = dt3 / de;
    double dt5 = dt4 * da;
    
    /* Chain 4: Long integers */
    long lt1 = la + lb;
    long lt2 = lt1 * lc;
    long lt3 = lt2 - ld;
    long lt4 = lt3 / le;
    long lt5 = lt4 % 1000;
    
    /* Vector operations - consume many registers */
    v4si vt1 = va + vb;
    v4si vt2 = vt1 * vc;
    v4si vt3 = vt2 - va;
    v4si vt4 = vt3 / vb;
    
    v4sf vft1 = vfa + vfb;
    v4sf vft2 = vft1 * vfa;
    v4sf vft3 = vft2 - vfb;
    
    v2df vdt1 = vda + vdb;
    v2df vdt2 = vdt1 * vda;
    v2df vdt3 = vdt2 - vdb;
    
    /* Artificial register pressure - clobber physical registers */
    CLOBBER_REGS();
    
    /* More operations after clobbering - forces reloads */
    int t11 = t10 + a;
    int t12 = t11 * b;
    int t13 = t12 - c;
    int t14 = t13 / d;
    int t15 = t14 ^ e;
    
    /* Call helper functions to increase inter-procedural pressure */
    struct MultiArg ma1 = helper_func1(t1, dt1, lt1, ft1);
    
    /* Use results immediately to create live ranges */
    int t16 = t15 + ma1.x;
    float ft6 = ft5 + ma1.w;
    double dt6 = dt5 + ma1.y;
    long lt6 = lt5 + ma1.z;
    
    /* More vector operations */
    v4si vt5 = helper_func2(vt1, vt2, vft1);
    v4si vt6 = vt4 + vt5;
    v4si vt7 = vt6 * vt3;
    
    /* Create structs for by-value passing */
    struct LargeStruct ls1 = {
        .vec1 = vt1,
        .vec2 = vdt1,
        .scalar1 = t1,
        .scalar2 = dt1,
        .scalar3 = lt1
    };
    
    struct LargeStruct ls2 = {
        .vec1 = vt2,
        .vec2 = vdt2,
        .scalar1 = t2,
        .scalar2 = dt2,
        .scalar3 = lt2
    };
    
    /* Another helper call with large structs */
    struct LargeStruct ls3 = helper_func3(ls1, ls2);
    
    /* Final computation using all temporaries */
    long result = (long)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                  (long)t11 + t12 + t13 + t14 + t15 + t16 +
                  (long)(ft1 + ft2 + ft3 + ft4 + ft5 + ft6) +
                  (long)(dt1 + dt2 + dt3 + dt4 + dt5 + dt6) +
                  lt1 + lt2 + lt3 + lt4 + lt5 + lt6 +
                  ls3.scalar3 +
                  vt1[0] + vt2[1] + vt3[2] + vt4[3] + vt5[0] + vt6[1] + vt7[2] +
                  (long)(vft1[0] + vft2[1] + vft3[2]) +
                  (long)(vdt1[0] + vdt2[1] + vdt3[0]);
    
    return result;
}

int main(void) {
    long total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to create hot code region */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent complete optimization */
        int seed = i + g_volatile_counter;
        total += test_function(seed);
        
        /* Prevent loop unrolling from reducing pressure */
        if (i % 7 == 0) {
            CLOBBER_REGS();
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    return (int)(total % 1000);
}
