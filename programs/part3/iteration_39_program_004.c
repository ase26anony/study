/* Main test driver with volatile loop control */
#include <stdint.h>
#include <stdio.h>

/* External helper functions from second compilation unit */
extern struct MultiArg multi_op1(struct MultiArg a, struct MultiArg b);
extern struct MultiArg multi_op2(struct MultiArg a, struct MultiArg b, struct MultiArg c);
extern double complex_math(double a, double b, double c, double d);

/* Volatile to prevent optimization */
volatile int iteration_count = 100;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for cross-function pressure */
struct MultiArg {
    double d1, d2;
    float f1, f2;
    int i1, i2;
    v4si vec;
};

/* Force noinline to prevent optimization */
__attribute__((noinline, optimize("O0")))
static struct MultiArg create_multi_arg(double base) {
    struct MultiArg arg;
    arg.d1 = base;
    arg.d2 = base * 2.0;
    arg.f1 = (float)base;
    arg.f2 = (float)(base * 3.0f);
    arg.i1 = (int)base;
    arg.i2 = (int)(base * 4.0);
    
    v4si v = {arg.i1, arg.i2, arg.i1 * 2, arg.i2 * 3};
    arg.vec = v;
    
    return arg;
}

/* Main test function with high register pressure */
__attribute__((noinline))
double test_function(double seed) {
    /* Many local variables of different types */
    double d1 = seed * 1.1;
    double d2 = seed * 2.2;
    double d3 = seed * 3.3;
    double d4 = seed * 4.4;
    double d5 = seed * 5.5;
    
    float f1 = (float)seed * 1.1f;
    float f2 = (float)seed * 2.2f;
    float f3 = (float)seed * 3.3f;
    float f4 = (float)seed * 4.4f;
    
    int i1 = (int)seed * 11;
    int i2 = (int)seed * 22;
    int i3 = (int)seed * 33;
    int i4 = (int)seed * 44;
    int i5 = (int)seed * 55;
    
    long l1 = (long)seed * 111L;
    long l2 = (long)seed * 222L;
    long l3 = (long)seed * 333L;
    
    /* Vector variables */
    v4si v1 = {i1, i2, i3, i4};
    v4si v2 = {i2, i3, i4, i5};
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = {f2, f3, f4, f1};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Double operations */
    d1 = d2 + d3;          /* Use d2, d3 -> d1 */
    d4 = d1 * d5;          /* Use d1, d5 -> d4 */
    d2 = d4 - d3;          /* Use d4, d3 -> d2 */
    d5 = d2 / d1;          /* Use d2, d1 -> d5 */
    d3 = d5 * d4;          /* Use d5, d4 -> d3 */
    
    /* Chain 2: Float operations with dependencies on doubles */
    f1 = (float)(d1 + d2);
    f2 = f1 * (float)d3;
    f3 = f2 - (float)d4;
    f4 = f3 / (float)d5;
    
    /* Chain 3: Integer operations */
    i1 = i2 + i3;
    i4 = i1 * i5;
    i2 = i4 - i3;
    i5 = i2 / (i1 ? i1 : 1);
    i3 = i5 * i4;
    
    /* Chain 4: Long operations */
    l1 = l2 + l3;
    l2 = l1 * 3L;
    l3 = l2 - l1;
    
    /* Chain 5: Vector operations */
    v1 = v1 + v2;
    v2 = v1 * v2;
    v1 = v2 - v1;
    
    vf1 = vf1 + vf2;
    vf2 = vf1 * vf2;
    vf1 = vf2 - vf1;
    
    vd1 = vd1 + vd2;
    vd2 = vd1 * vd2;
    vd1 = vd2 - vd1;
    
    /* Artificial register pressure with inline assembly */
    /* Clobber many registers to force pseudo-register usage */
    asm volatile (
        "/* Clobber physical registers */\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* More interdependent computations after clobber */
    d1 = d1 + f1 + i1 + l1;
    d2 = d2 + f2 + i2 + l2;
    d3 = d3 + f3 + i3 + l3;
    d4 = d4 + f4 + i4;
    d5 = d5 + i5;
    
    /* Use structs for cross-function pressure */
    struct MultiArg arg1 = create_multi_arg(d1);
    struct MultiArg arg2 = create_multi_arg(d2);
    struct MultiArg arg3 = create_multi_arg(d3);
    
    /* Call external functions that use multiple arguments */
    struct MultiArg res1 = multi_op1(arg1, arg2);
    struct MultiArg res2 = multi_op2(arg1, arg2, arg3);
    
    /* More computations using results */
    d1 += res1.d1 + res1.d2;
    d2 += res2.d1 + res2.d2;
    
    /* Final complex computation using all variables */
    double result = d1 + d2 + d3 + d4 + d5
                  + f1 + f2 + f3 + f4
                  + i1 + i2 + i3 + i4 + i5
                  + l1 + l2 + l3;
    
    /* Extract elements from vectors */
    int *vp = (int*)&v1;
    float *vfp = (float*)&vf1;
    double *vdp = (double*)&vd1;
    
    for (int j = 0; j < 4; j++) {
        result += vp[j] + vfp[j];
        if (j < 2) result += vdp[j];
    }
    
    /* Another assembly clobber */
    asm volatile (
        "nop"
        : 
        : 
        : "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
          "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
          "memory"
    );
    
    return result;
}

int main() {
    double total = 0.0;
    double seed = 1.2345;
    
    /* Loop to increase compilation complexity */
    for (int i = 0; i < iteration_count; i++) {
        /* Vary seed slightly each iteration */
        seed += 0.001;
        
        /* Call test function repeatedly */
        double result = test_function(seed);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Volatile memory operation */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    /* Print result to prevent optimization */
    printf("Result: %f\n", total);
    
    return (int)total % 256;
}
