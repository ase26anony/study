/* Test to trigger early rematerialization pseudo-register replacement */
/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -c early_remat_test.c */
/* Link with: gcc -O2 -fearly-remat early_remat_test.c early_remat_helpers.c -o test */

#include <stdint.h>
#include <stdio.h>

/* Force compiler to use many pseudo-registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Complex struct to force register pressure across calls */
struct MultiRegStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long_val;
    double double_val;
    int int_val;
};

/* Forward declarations for helper functions */
struct MultiRegStruct __attribute__((noinline)) helper_func1(int a, float b, double c, v4si v);
struct MultiRegStruct __attribute__((noinline)) helper_func2(struct MultiRegStruct s1, struct MultiRegStruct s2);
v4si __attribute__((noinline)) vector_op(v4si a, v4si b, v4si c);

/* Main test function with dense register usage */
static long __attribute__((noinline)) 
test_function(int input) 
{
    /* Declare many variables of different types to create register pressure */
    int a1 = input;
    int a2 = a1 * 3;
    int a3 = a2 + 7;
    int a4 = a3 - input;
    int a5 = a4 * 2;
    
    float f1 = (float)a1 * 1.5f;
    float f2 = f1 + 3.14f;
    float f3 = f2 * 2.0f;
    float f4 = f3 - f1;
    float f5 = f4 / 2.0f;
    
    double d1 = (double)a2 * 1.234567;
    double d2 = d1 + 9.876543;
    double d3 = d2 * 3.0;
    double d4 = d3 - d1;
    double d5 = d4 / 1.5;
    
    long l1 = (long)a3 * 1000L;
    long l2 = l1 + 9999L;
    long l3 = l2 * 2L;
    long l4 = l3 - l1;
    long l5 = l4 / 2L;
    
    /* Vector operations - use wide registers */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, input, a1, a2};
    v4si v3 = v1 + v2;
    v4si v4 = v3 * v1;
    v4si v5 = vector_op(v1, v2, v3);
    
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = {f5, f1, f2, f3};
    v4sf vf3 = vf1 + vf2;
    v4sf vf4 = vf3 * vf1;
    
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    v2df vd3 = vd1 + vd2;
    v2df vd4 = vd3 * vd1;
    
    /* Create complex dependency chain with many intermediate pseudo-registers */
    /* This should force early rematerialization to consider replacements */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 + a5;
    int t5 = t4 * t1;
    
    float ft1 = f1 + f2;
    float ft2 = ft1 * f3;
    float ft3 = ft2 - f4;
    float ft4 = ft3 + f5;
    float ft5 = ft4 * ft1;
    
    double dt1 = d1 + d2;
    double dt2 = dt1 * d3;
    double dt3 = dt2 - d4;
    double dt4 = dt3 + d5;
    double dt5 = dt4 * dt1;
    
    long lt1 = l1 + l2;
    long lt2 = lt1 * l3;
    long lt3 = lt2 - l4;
    long lt4 = lt3 + l5;
    long lt5 = lt4 * lt1;
    
    /* Mix all types in complex expressions */
    /* This creates many pseudo-registers with multiple uses */
    double mixed1 = (double)t1 + ft1 + dt1 + (double)lt1;
    float mixed2 = (float)t2 + ft2 + (float)dt2 + (float)lt2;
    int mixed3 = t3 + (int)ft3 + (int)dt3 + (int)lt3;
    long mixed4 = (long)t4 + (long)ft4 + (long)dt4 + lt4;
    
    /* Critical section: adjacent statements using same variable as 
       operand and destination - increases chance for DF_REF replacement */
    int critical_var = mixed3 * 2;
    critical_var = critical_var + t5;  /* Use as both source and dest */
    int critical_var2 = critical_var * 3;
    critical_var = critical_var2 - input;  /* Another use */
    
    float critical_float = mixed2 * 1.5f;
    critical_float = critical_float + ft5;
    float critical_float2 = critical_float * 2.0f;
    critical_float = critical_float2 - f1;
    
    /* Inline assembly to clobber physical registers and increase pressure */
    /* Clobber multiple registers to force more pseudo-register usage */
    asm volatile(
        "# Artificial register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        : 
        : "r" (critical_var), "r" (critical_float)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after assembly to ensure pseudo-registers are live */
    int final1 = critical_var * 2 + t1;
    float final2 = critical_float * 3.0f + ft1;
    double final3 = mixed1 * 4.0 + dt1;
    long final4 = mixed4 * 5L + lt1;
    
    /* Call helper functions to create inter-procedural pressure */
    struct MultiRegStruct s1 = helper_func1(final1, final2, final3, v5);
    struct MultiRegStruct s2 = helper_func1(final4, final2 * 2.0f, final3 / 2.0, v4);
    
    struct MultiRegStruct s3 = helper_func2(s1, s2);
    
    /* Use all variables in final computation to keep them live */
    long result = (long)final1 + (long)final2 + (long)final3 + final4 +
                  (long)s3.int_val + (long)s3.long_val +
                  (long)v5[0] + (long)v5[1] + (long)v5[2] + (long)v5[3] +
                  (long)vf4[0] + (long)vf4[1] + (long)vf4[2] + (long)vf4[3] +
                  (long)vd4[0] + (long)vd4[1] +
                  (long)t5 + (long)ft5 + (long)dt5 + lt5;
    
    return result;
}

int main(void) 
{
    long total = 0;
    int iterations = g_volatile_counter;
    
    /* Loop to increase compilation complexity and register pressure */
    for (int i = 0; i < iterations; i++) {
        /* Vary input to prevent constant propagation */
        int input = i + (g_volatile_counter & 0xFF);
        total += test_function(input);
        
        /* Prevent loop unrolling from reducing register pressure */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %ld\n", total);
    return (int)(total & 0x7FFFFFFF);
}
