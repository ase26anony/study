/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually requires many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This shuffle pattern uses 4 source vectors and 6 immediate control values
       to produce the result. During expansion, this may require 10 operands:
       4 source vectors + 6 immediate indices = 10 total */
    
    /* Create a complex shuffle pattern using multiple builtins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){2, 0, 3, 1});
    
    /* Final shuffle combining results with more immediate indices */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){1, 3, 0, 2});
    
    return result;
}

/* Vector conversion with many operands */
static v4sf convert_and_blend(v4si a, v4si b, v4sf c, v4sf d) {
    /* Convert vectors and blend - may expand to many operands */
    v4sf conv_a = __builtin_convertvector(a, v4sf);
    v4sf conv_b = __builtin_convertvector(b, v4sf);
    
    /* Complex blend operation with immediate mask */
    v4sf result = __builtin_shuffle(conv_a, conv_b, (v4si){0, 5, 2, 7});
    
    /* Additional operation mixing with other vectors */
    result = result * c + d;
    
    return result;
}

/* Multi-operand atomic operation simulation */
static long long complex_atomic_op(long long *ptr, int a, int b, int c, int d, 
                                   int e, int f, int g, int h) {
    /* Create a complex expression that might expand to many operands */
    long long val = *ptr;
    
    /* Complex bitwise expression with many constants */
    val = (val & 0xFFFFFFFF) | ((long long)a << 32);
    val = val ^ ((long long)b << 16);
    val = val | ((long long)c << 48);
    val = val & ~((long long)d << 8);
    val = val ^ ((long long)e << 24);
    val = val | ((long long)f << 40);
    val = val & ~((long long)g << 56);
    val = val ^ ((long long)h << 32);
    
    return val;
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vector variables with non-constant values */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {argc * 1.1f, argc * 1.2f, argc * 1.3f, argc * 1.4f};
    v4sf fvec2 = {argc * 1.5f, argc * 1.6f, argc * 1.7f, argc * 1.8f};
    
    long long atomic_var = 0x123456789ABCDEF0LL;
    
    /* Accumulator to prevent optimization */
    v4si acc_int = {0, 0, 0, 0};
    v4sf acc_float = {0.0f, 0.0f, 0.0f, 0.0f};
    long long acc_atomic = 0;
    
    /* Loop to ensure code is executed but not overly simplified */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call the 10-operand shuffle function */
        v4si shuffle_result = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        acc_int = acc_int + shuffle_result;
        
        /* Call the conversion and blend function */
        v4sf convert_result = convert_and_blend(vec1, vec2, fvec1, fvec2);
        acc_float = acc_float + convert_result;
        
        /* Call the complex atomic-like operation */
        long long atomic_result = complex_atomic_op(&atomic_var, 
            i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        acc_atomic ^= atomic_result;
        
        /* Modify inputs slightly to prevent constant propagation */
        vec1[0] += i;
        fvec1[0] += (float)i;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Accumulated int vector: %d %d %d %d\n", 
           acc_int[0], acc_int[1], acc_int[2], acc_int[3]);
    printf("Accumulated float vector: %f %f %f %f\n",
           acc_float[0], acc_float[1], acc_float[2], acc_float[3]);
    printf("Accumulated atomic: %llx\n", acc_atomic);
    
    return 0;
}
