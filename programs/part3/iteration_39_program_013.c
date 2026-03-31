/* main.c - Primary test file to induce register pressure */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* External helper functions */
extern struct DataPair helper1(int a, int b, int c, int d);
extern struct DataPair helper2(float a, float b, double c, double d);
extern struct DataPair helper3(long a, long b, int64_t c, int64_t d);
extern void clobber_registers(void);

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct to force register pressure across calls */
struct DataPair {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long_val;
    double double_val;
    int int_val;
};

/* Force noinline to prevent optimization */
__attribute__((noinline, optimize("O0")))
static struct DataPair create_pressure(void) {
    /* Many local variables to force pseudo-register usage */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    volatile double d5 = 5.55;
    
    volatile long l1 = 1000L;
    volatile long l2 = 2000L;
    volatile long l3 = 3000L;
    volatile long l4 = 4000L;
    
    volatile int64_t ll1 = 10000LL;
    volatile int64_t ll2 = 20000LL;
    volatile int64_t ll3 = 30000LL;
    
    /* Vector variables - use wide registers */
    volatile v4si vec1 = {v1, v2, v3, v4};
    volatile v4si vec2 = {v5, v6, v7, v8};
    volatile v4sf vecf1 = {f1, f2, f3, f4};
    volatile v4sf vecf2 = {f5, f1, f2, f3};
    volatile v2df vecd1 = {d1, d2};
    volatile v2df vecd2 = {d3, d4};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    int t1 = v1 + v2;      /* Pseudo-reg for t1 */
    int t2 = t1 * v3;      /* t1 used here, new pseudo for t2 */
    int t3 = t2 - v4;      /* t2 used here */
    int t4 = t3 / v5;      /* t3 used here */
    int t5 = t4 ^ v6;      /* t4 used here */
    int t6 = t5 | v7;      /* t5 used here */
    int t7 = t6 & v8;      /* t6 used here */
    int t8 = t7 << v9;     /* t7 used here */
    int t9 = t8 >> v10;    /* t8 used here */
    
    /* More chains with different types */
    float ft1 = f1 + f2;
    float ft2 = ft1 * f3;
    float ft3 = ft2 - f4;
    float ft4 = ft3 / f5;
    float ft5 = ft4 + f1;
    
    double dt1 = d1 + d2;
    double dt2 = dt1 * d3;
    double dt3 = dt2 - d4;
    double dt4 = dt3 / d5;
    double dt5 = dt4 + d1;
    
    long lt1 = l1 + l2;
    long lt2 = lt1 * l3;
    long lt3 = lt2 - l4;
    long lt4 = lt3 / l1;
    
    /* Vector operations - use wide registers */
    v4si vec_t1 = vec1 + vec2;
    v4si vec_t2 = vec_t1 * vec1;
    v4si vec_t3 = vec_t2 - vec2;
    v4si vec_t4 = vec_t3 / vec1;
    
    v4sf vec_ft1 = vecf1 + vecf2;
    v4sf vec_ft2 = vec_ft1 * vecf1;
    v4sf vec_ft3 = vec_ft2 - vecf2;
    
    v2df vec_dt1 = vecd1 + vecd2;
    v2df vec_dt2 = vec_dt1 * vecd1;
    v2df vec_dt3 = vec_dt2 - vecd2;
    
    /* Artificial clobber to reduce available registers */
    /* This forces more pseudo-register usage */
    asm volatile(
        "# Clobber physical registers to increase pressure\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        "mov r3, %3\n\t"
        : 
        : "r" (t1), "r" (t2), "r" (ft1), "r" (dt1)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* More computations after clobber */
    int t10 = t9 + t1;
    int t11 = t10 * t2;
    int t12 = t11 - t3;
    int t13 = t12 / t4;
    int t14 = t13 ^ t5;
    int t15 = t14 | t6;
    int t16 = t15 & t7;
    int t17 = t16 << t8;
    int t18 = t17 >> t9;
    
    /* Cross-type computations to create more pressure */
    double mixed1 = t1 + ft1 + dt1;
    double mixed2 = t2 * ft2 * dt2;
    double mixed3 = t3 - ft3 - dt3;
    double mixed4 = t4 / ft4 / dt4;
    
    /* Call helper functions to create inter-procedural pressure */
    struct DataPair dp1 = helper1(t1, t2, t3, t4);
    struct DataPair dp2 = helper2(ft1, ft2, dt1, dt2);
    struct DataPair dp3 = helper3(lt1, lt2, ll1, ll2);
    
    /* Use all computed values to prevent dead code elimination */
    struct DataPair result;
    
    /* Combine vector results */
    result.vec_int = vec_t1 + vec_t2 + vec_t3 + vec_t4;
    result.vec_int[0] += t10 + t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18;
    
    result.vec_float = vec_ft1 + vec_ft2 + vec_ft3;
    result.vec_float[0] += ft1 + ft2 + ft3 + ft4 + ft5;
    
    result.vec_double = vec_dt1 + vec_dt2 + vec_dt3;
    result.vec_double[0] += dt1 + dt2 + dt3 + dt4 + dt5;
    result.vec_double[1] += mixed1 + mixed2 + mixed3 + mixed4;
    
    result.long_val = lt1 + lt2 + lt3 + lt4;
    result.double_val = dp1.double_val + dp2.double_val + dp3.double_val;
    result.int_val = dp1.int_val + dp2.int_val + dp3.int_val;
    
    /* Add contributions from all variables */
    result.int_val += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result.double_val += f1 + f2 + f3 + f4 + f5;
    result.long_val += l1 + l2 + l3 + l4;
    
    return result;
}

__attribute__((noinline))
int test_function(void) {
    struct DataPair total = {0};
    
    /* Hot loop with many iterations */
    for (int i = 0; i < g_iterations; i++) {
        struct DataPair result = create_pressure();
        
        /* Accumulate results to ensure all computations are used */
        total.vec_int += result.vec_int;
        total.vec_float += result.vec_float;
        total.vec_double += result.vec_double;
        total.long_val += result.long_val;
        total.double_val += result.double_val;
        total.int_val += result.int_val;
        
        /* Modify volatile to prevent loop unrolling */
        asm volatile("" : "+g" (g_iterations));
    }
    
    /* Return a value based on all accumulated results */
    int final_result = total.int_val;
    final_result += total.vec_int[0] + total.vec_int[1] + total.vec_int[2] + total.vec_int[3];
    final_result += (int)total.vec_float[0];
    final_result += (int)total.vec_double[0];
    final_result += (int)total.long_val;
    final_result += (int)total.double_val;
    
    return final_result;
}

int main(void) {
    int result = test_function();
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
