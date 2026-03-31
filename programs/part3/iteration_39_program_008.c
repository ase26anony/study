/* Main test file to trigger early rematerialization pseudo-register replacement */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to use many pseudo-registers */
#define FORCE_REGISTER_PRESSURE 1

/* Volatile to prevent optimization */
volatile int g_volatile_counter = 1000;

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex struct for inter-procedural pressure */
struct MultiRegStruct {
    v4si vec_int;
    v4sf vec_float;
    v2df vec_double;
    long long_data;
    double double_data;
    int int_data;
};

/* Forward declarations for helper functions in separate file */
struct MultiRegStruct __attribute__((noinline)) 
helper_complex_calc(struct MultiRegStruct a, struct MultiRegStruct b, int count);

struct MultiRegStruct __attribute__((noinline))
helper_vector_ops(v4si vi, v4sf vf, v2df vd, int scale);

int __attribute__((noinline))
helper_mixed_ops(int a, float b, double c, long d, v4si v);

/* Inline assembly to clobber physical registers and force pseudo-register usage */
static inline void clobber_registers(void) {
    /* Clobber many registers to increase pressure */
    asm volatile(
        "# Clobber registers to force pseudo-register usage\n"
        "mov r0, r0\n"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
}

/* Main test function with dense computation to create register pressure */
static int __attribute__((noinline, optimize("O3")))
test_function(int seed) {
    /* Declare many local variables of different types */
    int t1 = seed + 1;
    int t2 = seed * 2;
    int t3 = seed / 3;
    int t4 = seed - 4;
    int t5 = seed % 5;
    
    float f1 = t1 * 1.1f;
    float f2 = t2 * 2.2f;
    float f3 = t3 * 3.3f;
    float f4 = t4 * 4.4f;
    float f5 = t5 * 5.5f;
    
    double d1 = f1 * 1.111;
    double d2 = f2 * 2.222;
    double d3 = f3 * 3.333;
    double d4 = f4 * 4.444;
    double d5 = f5 * 5.555;
    
    long l1 = t1 * 1000L;
    long l2 = t2 * 2000L;
    long l3 = t3 * 3000L;
    long l4 = t4 * 4000L;
    long l5 = t5 * 5000L;
    
    /* Vector variables - each uses a wide register */
    v4si v1 = {t1, t2, t3, t4};
    v4si v2 = {t5, t1, t2, t3};
    v4si v3 = {t4, t5, t1, t2};
    
    v4sf vf1 = {f1, f2, f3, f4};
    v4sf vf2 = {f5, f1, f2, f3};
    v4sf vf3 = {f4, f5, f1, f2};
    
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    v2df vd3 = {d5, d1};
    
    /* Create complex structs for inter-procedural pressure */
    struct MultiRegStruct s1 = {
        .vec_int = v1,
        .vec_float = vf1,
        .vec_double = vd1,
        .long_data = l1,
        .double_data = d1,
        .int_data = t1
    };
    
    struct MultiRegStruct s2 = {
        .vec_int = v2,
        .vec_float = vf2,
        .vec_double = vd2,
        .long_data = l2,
        .double_data = d2,
        .int_data = t2
    };
    
    /* Long chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations with pseudo-register reuse */
    t1 = t2 + t3;  /* Pseudo-reg for t1 gets replaced? */
    t4 = t1 * t5;  /* t1 used here - multiple uses */
    t2 = t4 - t3;
    t3 = t2 / t1;  /* t1 used again */
    t5 = t3 % t4;
    t1 = t5 + t2;  /* t1 reassigned - new pseudo-reg */
    
    /* Chain 2: Float operations */
    f1 = f2 + f3;
    f4 = f1 * f5;  /* f1 used here */
    f2 = f4 - f3;
    f3 = f2 / f1;  /* f1 used again */
    f5 = f3 * f4;
    f1 = f5 - f2;  /* f1 reassigned */
    
    /* Chain 3: Double operations with inline assembly clobber */
    d1 = d2 + d3;
    clobber_registers();  /* Force register spilling */
    d4 = d1 * d5;  /* d1 used after clobber */
    d2 = d4 - d3;
    d3 = d2 / d1;  /* d1 used again */
    d5 = d3 * d4;
    d1 = d5 - d2;
    
    /* Chain 4: Vector operations - each operation creates pseudo-regs */
    v1 = v2 + v3;
    v2 = v1 * v3;  /* v1 used here */
    v3 = v2 - v1;  /* v1 used again */
    v1 = v3 / v2;  /* v1 reassigned */
    
    /* Chain 5: Mixed operations creating cross-type dependencies */
    /* This creates complex dataflow patterns */
    int mixed1 = (int)f1 + t1;
    float mixed2 = (float)t2 * d1;
    double mixed3 = (double)f3 + l1;
    long mixed4 = (long)d2 * t3;
    
    /* Use results in complex expressions */
    t1 = mixed1 * 2;
    f1 = mixed2 / 2.0f;
    d1 = mixed3 * 2.0;
    l1 = mixed4 / 2L;
    
    /* More operations with pseudo-register reuse patterns */
    for (int i = 0; i < 5; i++) {
        /* Loop creates additional register pressure */
        t1 = t1 + t2;
        t2 = t1 - t3;  /* t1 used */
        t3 = t2 * t4;
        t4 = t3 / t5;
        t5 = t4 % t1;  /* t1 used again */
        
        /* Force pseudo-register to have multiple uses in adjacent statements */
        int temp = t1 * 2;    /* t1 used as operand */
        t1 = temp + t2;       /* t1 as destination - this pattern might trigger replacement */
        temp = t1 / 3;        /* t1 used immediately after assignment */
        t2 = temp * t1;       /* t1 used again */
    }
    
    /* Call helper functions to create inter-procedural register pressure */
    struct MultiRegStruct s3 = helper_complex_calc(s1, s2, t1);
    struct MultiRegStruct s4 = helper_vector_ops(v1, vf1, vd1, t2);
    
    /* More computation with results */
    v1 = v1 + s3.vec_int;
    vf1 = vf1 * s3.vec_float;
    vd1 = vd1 + s3.vec_double;
    
    /* Final complex computation using all temporaries */
    int result = t1 + t2 + t3 + t4 + t5;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    
    /* Use vector elements */
    int vsum = 0;
    for (int i = 0; i < 4; i++) {
        vsum += v1[i] + v2[i] + v3[i];
        vsum += (int)vf1[i] + (int)vf2[i] + (int)vf3[i];
    }
    for (int i = 0; i < 2; i++) {
        vsum += (int)vd1[i] + (int)vd2[i] + (int)vd3[i];
    }
    
    result += vsum;
    result += s3.int_data + s4.int_data;
    result += helper_mixed_ops(t1, f1, d1, l1, v1);
    
    return result;
}

int main(void) {
    int total = 0;
    volatile int iterations = g_volatile_counter;
    
    printf("Starting early rematerialization test...\n");
    
    /* Loop to increase compilation complexity and register pressure */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        int seed = i + (iterations % 37);
        int result = test_function(seed);
        
        /* Use result to prevent dead code elimination */
        total += result;
        
        /* Periodically clobber registers */
        if (i % 100 == 0) {
            clobber_registers();
        }
    }
    
    printf("Test completed. Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
