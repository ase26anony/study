/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Force register pressure with wide types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct to force register pressure across calls */
struct MultiRegStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long int64;
    double fp64;
    int int32;
    float fp32;
};

/* Function declarations from helper file */
struct MultiRegStruct __attribute__((noinline)) helper_func1(struct MultiRegStruct a, struct MultiRegStruct b);
struct MultiRegStruct __attribute__((noinline)) helper_func2(struct MultiRegStruct a, struct MultiRegStruct b, struct MultiRegStruct c);
v4si __attribute__((noinline)) vector_op1(v4si a, v4si b, v4si c);
v4sf __attribute__((noinline)) vector_op2(v4sf a, v4sf b, v4sf c);

/* Inline assembly to clobber registers and force pseudo-register usage */
#define CLOBBER_REGS() \
    asm volatile("" : : : \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", \
        "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", \
        "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", \
        "memory")

/* Complex test function with maximum register pressure */
__attribute__((noinline, optimize("O3")))
long long test_function(int base_val) {
    /* Declare many variables of different types to force pseudo-registers */
    v4si v1 = {base_val, base_val + 1, base_val + 2, base_val + 3};
    v4si v2 = {base_val * 2, base_val * 3, base_val * 4, base_val * 5};
    v4si v3 = {base_val + 10, base_val + 11, base_val + 12, base_val + 13};
    
    v4sf f1 = {base_val * 1.1f, base_val * 1.2f, base_val * 1.3f, base_val * 1.4f};
    v4sf f2 = {base_val * 2.1f, base_val * 2.2f, base_val * 2.3f, base_val * 2.4f};
    v4sf f3 = {base_val * 3.1f, base_val * 3.2f, base_val * 3.3f, base_val * 3.4f};
    
    v2df d1 = {base_val * 1.01, base_val * 1.02};
    v2df d2 = {base_val * 2.01, base_val * 2.02};
    v2df d3 = {base_val * 3.01, base_val * 3.02};
    
    /* Scalar variables - each will need a pseudo-register */
    int a = base_val;
    int b = base_val + 1;
    int c = base_val + 2;
    int d = base_val + 3;
    int e = base_val + 4;
    int f = base_val + 5;
    int g = base_val + 6;
    int h = base_val + 7;
    
    long long la = base_val * 100LL;
    long long lb = base_val * 200LL;
    long long lc = base_val * 300LL;
    long long ld = base_val * 400LL;
    
    float fa = base_val * 1.5f;
    float fb = base_val * 2.5f;
    float fc = base_val * 3.5f;
    float fd = base_val * 4.5f;
    
    double da = base_val * 1.25;
    double db = base_val * 2.25;
    double dc = base_val * 3.25;
    double dd = base_val * 4.25;
    
    /* Create artificial register pressure with interdependent computations */
    /* Chain 1: Integer operations with many intermediate values */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 ^ e;
    int t5 = t4 | f;
    int t6 = t5 & g;
    int t7 = t6 << 2;
    int t8 = t7 >> 1;
    int t9 = t8 + h;
    int t10 = t9 * a;
    
    /* Chain 2: Floating point operations */
    float ft1 = fa + fb;
    float ft2 = ft1 * fc;
    float ft3 = ft2 - fd;
    float ft4 = ft3 / fa;
    float ft5 = ft4 * 2.0f;
    float ft6 = ft5 + fb;
    float ft7 = ft6 - fc;
    float ft8 = ft7 * fd;
    
    /* Chain 3: Double precision */
    double dt1 = da + db;
    double dt2 = dt1 * dc;
    double dt3 = dt2 - dd;
    double dt4 = dt3 / da;
    double dt5 = dt4 * 2.0;
    double dt6 = dt5 + db;
    double dt7 = dt6 - dc;
    double dt8 = dt7 * dd;
    
    /* Chain 4: Long long operations */
    long long lt1 = la + lb;
    long long lt2 = lt1 * lc;
    long long lt3 = lt2 - ld;
    long long lt4 = lt3 ^ la;
    long long lt5 = lt4 | lb;
    long long lt6 = lt5 & lc;
    long long lt7 = lt6 << 3;
    long long lt8 = lt7 >> 2;
    long long lt9 = lt8 + ld;
    long long lt10 = lt9 * la;
    
    /* Vector operations - these use wide registers */
    v4si vt1 = v1 + v2;
    v4si vt2 = vt1 * v3;
    v4si vt3 = vt2 - v1;
    v4si vt4 = vt3 & v2;
    
    v4sf vft1 = f1 + f2;
    v4sf vft2 = vft1 * f3;
    v4sf vft3 = vft2 - f1;
    v4sf vft4 = vft3 / f2;
    
    v2df vdt1 = d1 + d2;
    v2df vdt2 = vdt1 * d3;
    v2df vdt3 = vdt2 - d1;
    v2df vdt4 = vdt3 / d2;
    
    /* Force register spilling by clobbering physical registers */
    CLOBBER_REGS();
    
    /* More computations after clobber - forces reloading from stack */
    int t11 = t10 + t9;
    int t12 = t11 * t8;
    int t13 = t12 - t7;
    int t14 = t13 ^ t6;
    int t15 = t14 | t5;
    
    float ft9 = ft8 + ft7;
    float ft10 = ft9 * ft6;
    float ft11 = ft10 - ft5;
    float ft12 = ft11 / ft4;
    
    double dt9 = dt8 + dt7;
    double dt10 = dt9 * dt6;
    double dt11 = dt10 - dt5;
    double dt12 = dt11 / dt4;
    
    /* Create structs for cross-function pressure */
    struct MultiRegStruct s1 = {
        .vec_int = vt4,
        .vec_float = vft4,
        .vec_double = vdt4,
        .int64 = lt10,
        .fp64 = dt12,
        .int32 = t15,
        .fp32 = ft12
    };
    
    struct MultiRegStruct s2 = {
        .vec_int = vt3,
        .vec_float = vft3,
        .vec_double = vdt3,
        .int64 = lt9,
        .fp64 = dt11,
        .int32 = t14,
        .fp32 = ft11
    };
    
    struct MultiRegStruct s3 = {
        .vec_int = vt2,
        .vec_float = vft2,
        .vec_double = vdt2,
        .int64 = lt8,
        .fp64 = dt10,
        .int32 = t13,
        .fp32 = ft10
    };
    
    /* Call helper functions to increase inter-procedural register pressure */
    struct MultiRegStruct r1 = helper_func1(s1, s2);
    struct MultiRegStruct r2 = helper_func2(s1, s2, s3);
    
    /* More vector operations */
    v4si vr1 = vector_op1(vt4, vt3, vt2);
    v4sf vr2 = vector_op2(vft4, vft3, vft2);
    
    /* Final computation using all intermediate values */
    long long result = 
        (long long)t15 + 
        (long long)(ft12 * 1000) + 
        (long long)(dt12 * 10000) + 
        lt10 + 
        vr1[0] + vr1[1] + vr1[2] + vr1[3] +
        (long long)(vr2[0] * 100) +
        r1.int64 + r2.int64;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

int main() {
    volatile int iterations = g_volatile_counter;
    long long total = 0;
    
    /* Hot loop to trigger aggressive compilation */
    for (int i = 0; i < iterations; i++) {
        /* Vary input to prevent constant propagation */
        int input = i + (iterations & 0xFF);
        total += test_function(input);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
