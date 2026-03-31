/* Primary test file with hot loop and register pressure */
#include <stdint.h>
#include <stdlib.h>

/* Force pseudo-register creation through complex expressions */
#define FORCE_REG(N) asm volatile("" : : "r"(N) : "memory")

/* External helper functions to increase inter-procedural pressure */
extern struct Vec4 { float x, y, z, w; } helper1(struct Vec4 a, struct Vec4 b);
extern struct Vec4 helper2(struct Vec4 a, struct Vec4 b, struct Vec4 c);
extern struct Double2 { double a, b; } helper3(struct Double2 x, struct Double2 y);
extern int helper4(int a, int b, int c, int d, int e, int f);

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.71828;

/* NOINLINE test function with massive register pressure */
__attribute__((noinline, optimize("O3")))
long test_function(int seed) {
    /* Declare many variables of different types */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed % 7;
    int e = seed ^ 0x55AA55AA;
    int f = seed | 0x12345678;
    
    float fa = seed * 1.1f;
    float fb = seed * 2.2f;
    float fc = seed * 3.3f;
    float fd = seed * 4.4f;
    float fe = seed * 5.5f;
    float ff = seed * 6.6f;
    
    double da = seed * 1.111;
    double db = seed * 2.222;
    double dc = seed * 3.333;
    double dd = seed * 4.444;
    double de = seed * 5.555;
    double df = seed * 6.666;
    
    long la = seed * 1000L;
    long lb = seed * 2000L;
    long lc = seed * 3000L;
    long ld = seed * 4000L;
    
    /* Vector variables for wide register pressure */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4sf vf1 = {fa, fb, fc, fd};
    v4sf vf2 = {fe, ff, fa * 2, fb * 2};
    v2df vd1 = {da, db};
    v2df vd2 = {dc, dd};
    
    /* Complex interdependent computations to force pseudo-registers */
    /* Chain 1: Integer operations with many intermediate values */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 ^ e;
    int t5 = t4 | f;
    int t6 = t5 & a;
    int t7 = t6 + b;
    int t8 = t7 * c;
    int t9 = t8 - d;
    int t10 = t9 ^ e;
    
    /* Chain 2: Float operations with dependencies */
    float ft1 = fa + fb;
    float ft2 = ft1 * fc;
    float ft3 = ft2 - fd;
    float ft4 = ft3 / fe;
    float ft5 = ft4 + ff;
    float ft6 = ft5 * fa;
    float ft7 = ft6 - fb;
    float ft8 = ft7 / fc;
    float ft9 = ft8 + fd;
    float ft10 = ft9 * fe;
    
    /* Chain 3: Double operations */
    double dt1 = da + db;
    double dt2 = dt1 * dc;
    double dt3 = dt2 - dd;
    double dt4 = dt3 / de;
    double dt5 = dt4 + df;
    double dt6 = dt5 * da;
    double dt7 = dt6 - db;
    double dt8 = dt7 / dc;
    double dt9 = dt8 + dd;
    double dt10 = dt9 * de;
    
    /* Chain 4: Long operations */
    long lt1 = la + lb;
    long lt2 = lt1 * lc;
    long lt3 = lt2 - ld;
    long lt4 = lt3 + la;
    long lt5 = lt4 * lb;
    long lt6 = lt5 - lc;
    long lt7 = lt6 + ld;
    long lt8 = lt7 * la;
    long lt9 = lt8 - lb;
    long lt10 = lt9 + lc;
    
    /* Vector operations - each creates multiple pseudo-registers */
    v4si vt1 = v1 + v2;
    v4si vt2 = vt1 * v1;
    v4si vt3 = vt2 - v2;
    v4si vt4 = vt3 + v1;
    
    v4sf vft1 = vf1 + vf2;
    v4sf vft2 = vft1 * vf1;
    v4sf vft3 = vft2 - vf2;
    v4sf vft4 = vft3 / vf1;
    
    v2df vdt1 = vd1 + vd2;
    v2df vdt2 = vdt1 * vd1;
    v2df vdt3 = vdt2 - vd2;
    v2df vdt4 = vdt3 / vd1;
    
    /* Artificial register pressure through inline assembly */
    /* Clobber many physical registers to force pseudo-register usage */
    asm volatile(
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        :
        : "r"(t10), "r"(ft10)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after clobbering - forces reloading */
    int t11 = t10 + ft10;
    float ft11 = ft10 + t10;
    double dt11 = dt10 + ft11;
    long lt11 = lt10 + t11;
    
    /* Use vector results */
    int vs1 = vt4[0] + vt4[1] + vt4[2] + vt4[3];
    float vfs1 = vft4[0] + vft4[1] + vft4[2] + vft4[3];
    double vds1 = vdt4[0] + vdt4[1];
    
    /* Create structs for helper calls */
    struct Vec4 vec1 = {ft1, ft2, ft3, ft4};
    struct Vec4 vec2 = {ft5, ft6, ft7, ft8};
    struct Vec4 vec3 = {ft9, ft10, ft11, fa};
    
    /* Call helpers to increase inter-procedural pressure */
    struct Vec4 res1 = helper1(vec1, vec2);
    struct Vec4 res2 = helper2(vec1, vec2, vec3);
    
    struct Double2 dbl1 = {dt1, dt2};
    struct Double2 dbl2 = {dt3, dt4};
    struct Double2 dbl3 = helper3(dbl1, dbl2);
    
    int helper_res = helper4(t1, t2, t3, t4, t5, t6);
    
    /* Final computation using all temporaries */
    long final_result = 
        (long)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 +
        (long)ft1 + ft2 + ft3 + ft4 + ft5 + ft6 + ft7 + ft8 + ft9 + ft10 + ft11 +
        (long)dt1 + dt2 + dt3 + dt4 + dt5 + dt6 + dt7 + dt8 + dt9 + dt10 + dt11 +
        lt1 + lt2 + lt3 + lt4 + lt5 + lt6 + lt7 + lt8 + lt9 + lt10 + lt11 +
        vs1 + vfs1 + vds1 +
        res1.x + res1.y + res1.z + res1.w +
        res2.x + res2.y + res2.z + res2.w +
        dbl3.a + dbl3.b +
        helper_res;
    
    /* Force register usage before return */
    FORCE_REG(final_result);
    
    return final_result;
}

int main() {
    long total = 0;
    int iterations = g_volatile_counter;
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < iterations; i++) {
        /* Mix different seeds to prevent constant propagation */
        int seed = i ^ (i >> 3) ^ (i << 5);
        seed += g_volatile_float;
        seed ^= g_volatile_double;
        
        total += test_function(seed);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile long* sink = (volatile long*)malloc(sizeof(long));
    *sink = total;
    
    return (*sink > 0) ? 0 : 1;
}
