/* reload_test.c
 * Designed to trigger GCC's reload pass push_reload function
 * with secondary reload initialization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 9876543210LL;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.718281828459045;

/* Vector types for additional reload complexity */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to trigger various reload scenarios */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed integer types with specific register constraints
     * This forces reloads due to register class mismatches */
    asm volatile (
        /* Move with potential need for secondary reload */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        /* Use different register constraints to force reloads */
        : [out1] "=r" (out1), 
          [out3] "=r" (out3)
        : [in1] "ri" (a),           /* 'r' or 'i' constraint - may need secondary reload */
          [in2] "rm" (g_volatile_int), /* 'r' or 'm' with volatile global */
          [in3] "r" (b)             /* Register constraint only */
        : "cc", "memory"
    );
    accumulator += out1 + out3;
    
    /* ASM 2: Floating point with integer conversion
     * Forces mode changes and potential secondary reloads */
    asm volatile (
        /* Simulate a conversion operation */
        "cvtsi2ssl %[int_in], %[float_out]\n\t"
        "cvtss2sd %[float_out], %[double_out]\n\t"
        : [float_out] "=x" (out4),    /* SSE register constraint */
          [double_out] "=x" (out5)     /* SSE register constraint */
        : [int_in] "rm" (a),          /* Integer in memory or register */
          [float_in] "x" (c)          /* Floating point in SSE register */
        : 
    );
    accumulator += (long long)(out4 + out5);
    
    /* ASM 3: Vector operations with memory constraints
     * May require secondary reloads for vector constants */
    asm volatile (
        /* Vector add operation */
        "paddd %[vec_in1], %[vec_in2], %[vec_out]\n\t"
        : [vec_out] "=x" (out_vec_int)
        : [vec_in1] "xm" (vec_int),    /* Vector in memory or register */
          [vec_in2] "x" (vec_int)      /* Vector in register only */
        : 
    );
    
    /* ASM 4: Complex addressing modes with multiple outputs
     * Forces output reloads with different modes */
    asm volatile (
        /* Multiple operations with different operand types */
        "imull %[in1], %[out1]\n\t"
        "movq %[in2], %[out3]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3)
        : [in1] "0" (a),              /* Matching constraint */
          [in2] "rmi" (b),            /* r/m/i - may need secondary for immediate */
          [in3] "i" (0x12345678)      /* Large immediate - may need secondary reload */
        : "cc"
    );
    accumulator += out1 + out2 + out3;
    
    /* ASM 5: String operation with implicit registers
     * Forces specific register allocation */
    {
        void *src = &g_volatile_int;
        void *dst = &out1;
        asm volatile (
            "movsb"  /* Byte string move */
            : "+S" (src), "+D" (dst), "+c" (out2)
            : 
            : "memory"
        );
    }
    
    /* ASM 6: In-out operand with earlyclobber
     * Creates complex reload scenario */
    asm volatile (
        "lea (%[in], %[in], 2), %[out]\n\t"  /* out = in * 3 */
        : [out] "=&r" (out1)                 /* Earlyclobber */
        : [in] "r" (a)
        : 
    );
    accumulator += out1;
    
    return accumulator;
}

/* Wrapper function to ensure variables are used */
static long long test_wrapper(int argc, char **argv) {
    /* Initialize with non-constant values */
    int int_val = argc > 1 ? atoi(argv[1]) : 42;
    long long ll_val = argc > 2 ? atoll(argv[2]) : 1000000000LL;
    float float_val = argc > 3 ? atof(argv[3]) : 1.234f;
    double double_val = argc > 4 ? atof(argv[4]) : 5.678;
    
    /* Vector initialization */
    v4si vec_int = {int_val, int_val + 1, int_val + 2, int_val + 3};
    v4sf vec_float = {float_val, float_val * 2, float_val * 3, float_val * 4};
    
    /* Call the reload trigger multiple times with different args */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(int_val + i, ll_val + i, 
                                 float_val + i, double_val + i,
                                 vec_int, vec_float);
    }
    
    return result;
}

int main(int argc, char **argv) {
    long long result = test_wrapper(argc, argv);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional volatile operations to ensure code isn't optimized away */
    asm volatile ("" : : "r"(result) : "memory");
    
    return (result > 0) ? 0 : 1;
}
