/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually uses many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern should require many operands during expansion:
     * 4 source vectors + 6 immediate control values = 10 operands
     * The control pattern selects elements from all 4 vectors in a complex pattern
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control */
    /* First, create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Final complex shuffle combining all 4 vectors */
    /* This pattern selects: a[0], b[1], c[2], d[3], a[1], b[2], c[3], d[0] */
    /* But rearranged into final 4-element vector */
    result = __builtin_shuffle(ab_shuffle, cd_shuffle, (v4si){0, 5, 2, 7});
    
    return result;
}

/* Another approach: Use vector conversion with complex pattern */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d) {
    /* Create a complex pattern using __builtin_convertvector 
     * with intermediate operations that may expand to many operands
     */
    v4si combined = a + b - c * d;
    
    /* Use multiple immediate constants in the conversion pattern */
    v4sf temp1 = __builtin_convertvector(combined, v4sf);
    v4sf temp2 = __builtin_convertvector(a - b + c, v4sf);
    
    /* Complex shuffle with immediate control */
    return __builtin_shuffle(temp1, temp2, (v4si){3, 2, 1, 0});
}

/* Function using atomic operations with many arguments */
static long long atomic_complex_operation(volatile long long *ptr, 
                                         v2di mask1, v2di mask2, 
                                         v2di val1, v2di val2) {
    /* Complex atomic operation pattern that may require many operands */
    long long result = 0;
    
    /* Use __sync builtins with complex expressions */
    result = __sync_fetch_and_add(ptr, 
        ((mask1[0] & val1[0]) | (mask2[0] & val2[0])) +
        ((mask1[1] & val1[1]) | (mask2[1] & val2[1])));
    
    return result;
}

/* Main test function with loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vector variables with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    v2di mask1 = {0xFF00FF00FF00FF00LL, 0x00FF00FF00FF00FFLL};
    v2di mask2 = {0xF0F0F0F0F0F0F0F0LL, 0x0F0F0F0F0F0F0F0FLL};
    v2di val1 = {0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL};
    v2di val2 = {0x5555555555555555LL, 0xAAAAAAAAAAAAAAAALL};
    
    volatile long long atomic_var = 0x1000;
    
    v4si final_result_int = {0, 0, 0, 0};
    v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    volatile long long final_atomic_result = 0;
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with potentially 10 operands */
        v4si shuffle_result = complex_shuffle_10_operand(
            vec_a + i, vec_b - i, vec_c * i, vec_d / (i + 1));
        
        /* Test 2: Vector conversion with complex pattern */
        v4sf convert_result = vector_conversion_complex(
            vec_a, vec_b + i, vec_c - i, vec_d * i);
        
        /* Test 3: Atomic operation with complex expression */
        long long atomic_result = atomic_complex_operation(
            &atomic_var, mask1, mask2 + i, val1 - i, val2 * i);
        
        /* Accumulate results to prevent optimization */
        final_result_int += shuffle_result;
        final_result_float += convert_result;
        final_atomic_result += atomic_result;
        
        /* Modify inputs slightly each iteration */
        vec_a[0] += 1;
        vec_b[1] += 1;
        vec_c[2] += 1;
        vec_d[3] += 1;
    }
    
    /* Print results to create side effects */
    printf("Final int result: %d %d %d %d\n", 
           final_result_int[0], final_result_int[1], 
           final_result_int[2], final_result_int[3]);
    
    printf("Final float result: %.2f %.2f %.2f %.2f\n",
           final_result_float[0], final_result_float[1],
           final_result_float[2], final_result_float[3]);
    
    printf("Final atomic result: %lld\n", final_atomic_result);
    
    return 0;
}

/* Additional complex pattern that might trigger 10-operand expansion */
v4si __attribute__((noinline)) 
complex_vector_pattern(v4si a, v4si b, v4si c, v4si d, 
                      v4si e, v4si f, v4si g, v4si h) {
    /* This complex expression uses many vector operations and constants
     * that may require 10 operands during RTL expansion
     */
    v4si result;
    
    /* Complex expression with many immediate constants */
    result = (a & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000}) |
             (b & (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555}) |
             (c << (v4si){1, 2, 3, 4}) |
             (d >> (v4si){4, 3, 2, 1}) |
             (e + (v4si){100, 200, 300, 400}) |
             (f - (v4si){50, 100, 150, 200}) |
             (g * (v4si){2, 3, 4, 5}) |
             (h / (v4si){2, 2, 2, 2});
    
    /* Final complex shuffle with immediate control */
    return __builtin_shuffle(result, a + b + c + d, 
                            (v4si){7, 6, 5, 4, 3, 2, 1, 0});
}
