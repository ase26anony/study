/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>

/* Force compiler to use many pseudo-registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 1000;
volatile int g_volatile_seed = 12345;

/* Noinline helper functions to increase register pressure across calls */
struct LargeStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long extra[4];
};

/* Forward declarations from helper file */
struct LargeStruct __attribute__((noinline)) helper_func1(int a, float b, double c, v4si v);
struct LargeStruct __attribute__((noinline)) helper_func2(struct LargeStruct s1, struct LargeStruct s2);
int __attribute__((noinline)) helper_func3(v4sf fv, v2df dv, long* arr);

/* Inline assembly to clobber physical registers and force pseudo-register usage */
#define CLOBBER_REGS() \
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
                 "memory")

/* Main test function with dense computation to force register pressure */
__attribute__((noinline, optimize("O3")))
int test_function(int iter) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    long l1, l2, l3, l4, l5;
    v4si vint1, vint2, vint3, vint4;
    v4sf vfloat1, vfloat2, vfloat3;
    v2df vdouble1, vdouble2;
    
    /* Initialize with volatile to prevent constant propagation */
    a1 = g_volatile_seed + iter;
    a2 = a1 * 3;
    a3 = a2 - 7919;
    a4 = a3 / 2;
    a5 = a4 + 65537;
    
    /* Chain of interdependent computations - forces serial evaluation */
    f1 = (float)a1 * 1.234f;
    f2 = f1 * 2.718f + (float)a2;
    f3 = f2 / 3.141f - (float)a3;
    f4 = f3 * f2 + f1;
    f5 = f4 - f3 / f2;
    
    d1 = (double)a4 * 1.41421356;
    d2 = d1 + (double)f1 * 2.0;
    d3 = d2 * d1 - (double)a5;
    d4 = d3 / d2 + d1;
    
    l1 = (long)a1 * 2147483647L;
    l2 = l1 + (long)a2 * 3;
    l3 = l2 - (long)a3 * 5;
    l4 = l3 + (long)a4 * 7;
    l5 = l4 - (long)a5 * 11;
    
    /* Vector operations - use wide registers */
    vint1 = (v4si){a1, a2, a3, a4};
    vint2 = (v4si){a5, a1 + 1, a2 + 2, a3 + 3};
    vint3 = vint1 + vint2;
    vint4 = vint3 * (v4si){2, 3, 4, 5};
    
    vfloat1 = (v4sf){f1, f2, f3, f4};
    vfloat2 = (v4sf){f5, f1 * 2.0f, f2 * 3.0f, f3 * 4.0f};
    vfloat3 = vfloat1 + vfloat2 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    vdouble1 = (v2df){d1, d2};
    vdouble2 = (v2df){d3, d4};
    
    /* Critical pattern: pseudo-register used as both operand and destination
       in adjacent statements to create multiple uses */
    int temp1 = a1 + a2;
    int temp2 = temp1 * a3;  /* temp1 used here */
    int temp3 = temp2 - a4;  /* temp2 used here */
    int temp4 = temp3 / a5;  /* temp3 used here */
    int temp5 = temp4 + temp1; /* temp4 and temp1 used here */
    
    /* More chains with mixed types */
    float ftemp1 = (float)temp1 * f1;
    float ftemp2 = ftemp1 + f2;  /* ftemp1 used */
    float ftemp3 = ftemp2 * f3;  /* ftemp2 used */
    float ftemp4 = ftemp3 - f4;  /* ftemp3 used */
    float ftemp5 = ftemp4 / f5 + ftemp1; /* ftemp4 and ftemp1 used */
    
    /* Force register pressure with clobber */
    CLOBBER_REGS();
    
    /* Call helper functions to increase inter-procedural pressure */
    struct LargeStruct s1 = helper_func1(temp1, ftemp1, d1, vint1);
    struct LargeStruct s2 = helper_func1(temp2, ftemp2, d2, vint2);
    
    /* Complex computation using helper results */
    struct LargeStruct s3 = helper_func2(s1, s2);
    
    /* More computation after call */
    int temp6 = temp5 + s3.vec_int[0];
    int temp7 = temp6 * s3.vec_int[1];
    int temp8 = temp7 - s3.vec_int[2];
    int temp9 = temp8 + s3.vec_int[3];
    
    /* Use all vector variables */
    vint4 = vint4 + s3.vec_int;
    vfloat3 = vfloat3 + s3.vec_float;
    
    /* Final complex expression using all temporaries */
    long final_result = (long)temp9 * 3 +
                       (long)temp5 * 5 +
                       (long)(ftemp5 * 100.0f) +
                       l5 +
                       s3.extra[0] +
                       s3.extra[1] +
                       s3.extra[2] +
                       s3.extra[3];
    
    /* Another clobber to force spills */
    CLOBBER_REGS();
    
    /* Final helper call with many arguments */
    int final_check = helper_func3(vfloat3, vdouble1 + vdouble2, s3.extra);
    
    return (int)(final_result % 1000000) + final_check;
}

int main() {
    int total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iterations; i++) {
        /* Volatile to prevent loop unrolling elimination */
        volatile int iter_mod = i % 100;
        total += test_function(iter_mod);
        
        /* Prevent dead code elimination */
        if (total > 1000000000) {
            total = total % 1000000;
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
