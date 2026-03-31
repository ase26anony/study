/* test_optabs_10_operands.c
 * Designed to trigger case 10: in optabs.cc lines 8254-8263
 * This creates expressions requiring exactly 10 operands during RTL expansion
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern should require many operands:
     * 4 source vectors + 6 immediate control values = 10 operands
     * The compiler may expand this into an optab requiring 10 arguments
     */
    v4si result;
    
    /* Create a complex shuffle with multiple control values */
    result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});  /* 4 operands: a, b, control vector */
    
    /* Mix with another shuffle using different control values */
    v4si temp = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});  /* Another 4 operands */
    
    /* Final blend/shuffle combining results with more control values */
    result = __builtin_shuffle(result, temp, (v4si){0, 4, 2, 6});  /* 4 more operands */
    
    return result;
}

/* Vector reduction with many constants - may expand to multi-operand pattern */
static v4sf vector_fma_with_constants(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add with multiple constants:
     * Each constant operation adds operands to the expansion
     */
    v4sf result;
    
    /* Complex expression with many constants that can't be easily folded */
    v4sf const1 = (v4sf){3.14159f, 2.71828f, 1.41421f, 1.61803f};
    v4sf const2 = (v4sf){0.57721f, 1.73205f, 2.23607f, 0.69315f};
    v4sf const3 = (v4sf){1.23456f, 2.34567f, 3.45678f, 4.56789f};
    
    /* Multi-operand expression: a*const1 + b*const2 + c*const3 + d */
    result = a * const1;
    result += b * const2;
    result += c * const3;
    result += d;
    
    return result;
}

/* Use __builtin_convertvector with complex type conversions */
static v4si convert_complex_vector(v2df a, v2df b) {
    /* Convert two double vectors to int vector - may require many operands */
    v4sf float_vec1 = __builtin_convertvector(a, v4sf);
    v4sf float_vec2 = __builtin_convertvector(b, v4sf);
    
    /* Combine and convert to int */
    v4sf combined = float_vec1 + float_vec2;
    return __builtin_convertvector(combined, v4si);
}

/* Atomic operation simulation with many parameters */
static long long complex_atomic_operation(volatile long long *ptr, 
                                         long long a, long long b, 
                                         long long c, long long d) {
    /* Simulate complex atomic operation requiring many operands */
    long long old = *ptr;
    long long new_val = (old * a + b) ^ (c | d);
    
    /* Try to do compare-and-swap (__sync_val_compare_and_swap takes 3 args,
     * but complex expansion might need more) */
    long long expected = old;
    long long desired = new_val;
    
    /* __sync_bool_compare_and_swap takes 3 args, but its expansion
     * might involve more operands in the RTL */
    if (__sync_bool_compare_and_swap(ptr, expected, desired)) {
        return desired;
    }
    return old;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent dead code elimination */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf fvec2 = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    v4sf fvec3 = {(float)(argc * 6), (float)(argc * 7), (float)(argc * 8), (float)(argc * 9)};
    v4sf fvec4 = {(float)(argc * 10), (float)(argc * 11), (float)(argc * 12), (float)(argc * 13)};
    
    v2di dvec1 = {(long long)argc, (long long)(argc + 100)};
    v2di dvec2 = {(long long)(argc * 200), (long long)(argc * 300)};
    
    volatile long long atomic_var = 0;
    
    v4si final_result_int = {0, 0, 0, 0};
    v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to ensure code is executed but not optimized away */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        v4si shuffle_result = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        v4sf fma_result = vector_fma_with_constants(fvec1, fvec2, fvec3, fvec4);
        v4si convert_result = convert_complex_vector(
            __builtin_convertvector(dvec1, v2df),
            __builtin_convertvector(dvec2, v2df)
        );
        
        /* Accumulate results to prevent elimination */
        final_result_int += shuffle_result + convert_result;
        final_result_float += fma_result;
        
        /* Complex atomic operation */
        long long atomic_result = complex_atomic_operation(
            &atomic_var, 
            i, i * 2, i * 3, i * 4
        );
        atomic_var += atomic_result;
    }
    
    /* Print results to create side effects */
    printf("Final int result: %d %d %d %d\n", 
           final_result_int[0], final_result_int[1], 
           final_result_int[2], final_result_int[3]);
    printf("Final float result: %f %f %f %f\n",
           final_result_float[0], final_result_float[1],
           final_result_float[2], final_result_float[3]);
    printf("Atomic var: %lld\n", (long long)atomic_var);
    
    return 0;
}
