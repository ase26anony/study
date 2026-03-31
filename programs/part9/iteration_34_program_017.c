/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values = 6 operands
     * Total: 10 operands
     */
    
    /* Create a complex shuffle using multiple builtins and constants */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){3, 1, 0, 2});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){1, 3, 2, 0});
    
    /* Mix with immediate constants in operations */
    v4si result = temp1 + (v4si){5, 10, 15, 20};
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    result = result | (temp2 & (v4si){0xFF000000, 0xFF0000, 0xFF00, 0xFF});
    
    /* Additional shuffle with immediate control */
    return __builtin_shuffle(result, temp1, (v4si){2, 0, 3, 1});
}

/* Vector FMA-like expression with many constants */
static v4sf vector_polynomial(v4sf x, v4sf coeff0, v4sf coeff1, 
                              v4sf coeff2, v4sf coeff3) {
    /* Polynomial: coeff3*x^3 + coeff2*x^2 + coeff1*x + coeff0
     * Each operation involves multiple operands when expanded */
    
    v4sf x2 = x * x;
    v4sf x3 = x2 * x;
    
    /* This complex expression may expand to many operands */
    return ((coeff3 * x3 + (v4sf){1.1f, 2.2f, 3.3f, 4.4f}) * 
            (coeff2 * x2 + (v4sf){0.5f, 1.5f, 2.5f, 3.5f})) +
           ((coeff1 * x + (v4sf){0.1f, 0.2f, 0.3f, 0.4f}) * 
            (coeff0 + (v4sf){0.01f, 0.02f, 0.03f, 0.04f}));
}

/* Complex number multiplication (a+bi)*(c+di) */
static v2df complex_multiply(v2df a, v2df b) {
    /* GCC may expand complex multiplication to internal function
     * requiring multiple operands */
    v2df real = __builtin_shuffle(a, (v2df){0.0, 0.0}, (v2di){0, 2}) *
                __builtin_shuffle(b, (v2df){0.0, 0.0}, (v2di){0, 2}) -
                __builtin_shuffle(a, (v2df){0.0, 0.0}, (v2di){1, 3}) *
                __builtin_shuffle(b, (v2df){0.0, 0.0}, (v2di){1, 3});
    
    v2df imag = __builtin_shuffle(a, (v2df){0.0, 0.0}, (v2di){0, 2}) *
                __builtin_shuffle(b, (v2df){0.0, 0.0}, (v2di){1, 3}) +
                __builtin_shuffle(a, (v2df){0.0, 0.0}, (v2di){1, 3}) *
                __builtin_shuffle(b, (v2df){0.0, 0.0}, (v2di){0, 2});
    
    return __builtin_shuffle(real, imag, (v2di){0, 3});
}

/* Table lookup simulation with many operands */
static v4si vector_table_lookup(v4si index, v4si table0, v4si table1,
                                v4si table2, v4si table3) {
    /* Simulate a 4-vector table lookup with immediate masks */
    v4si mask0 = (index & (v4si){0x3, 0x3, 0x3, 0x3}) == (v4si){0, 0, 0, 0};
    v4si mask1 = (index & (v4si){0x3, 0x3, 0x3, 0x3}) == (v4si){1, 1, 1, 1};
    v4si mask2 = (index & (v4si){0x3, 0x3, 0x3, 0x3}) == (v4si){2, 2, 2, 2};
    v4si mask3 = (index & (v4si){0x3, 0x3, 0x3, 0x3}) == (v4si){3, 3, 3, 3};
    
    /* This blend operation with multiple vectors and masks
     * may require many operands during expansion */
    v4si result = (table0 & mask0) | (table1 & mask1) |
                  (table2 & mask2) | (table3 & mask3);
    
    return result;
}

/* Main function with non-trivial computation to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf vec_f1 = {(float)argc, (float)argc/2, (float)argc/3, (float)argc/4};
    v4sf coeff0 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf coeff1 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf coeff2 = {0.25f, 0.75f, 1.25f, 1.75f};
    v4sf coeff3 = {0.125f, 0.375f, 0.625f, 0.875f};
    
    v2df complex1 = {(double)argc, (double)argc/2};
    v2df complex2 = {(double)argc/3, (double)argc/4};
    
    v4si table0 = {100, 200, 300, 400};
    v4si table1 = {500, 600, 700, 800};
    v4si table2 = {900, 1000, 1100, 1200};
    v4si table3 = {1300, 1400, 1500, 1600};
    v4si indices = {argc % 4, (argc + 1) % 4, (argc + 2) % 4, (argc + 3) % 4};
    
    volatile v4si final_result_int = {0, 0, 0, 0};
    volatile v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    volatile v2df final_result_complex = {0.0, 0.0};
    
    /* Loop to prevent dead code elimination but not large enough to cause unrolling */
    for (int i = 0; i < iterations; i++) {
        /* Perform complex operations that may trigger 10-operand expansions */
        v4si shuffle_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        v4sf poly_result = vector_polynomial(vec_f1, coeff0, coeff1, coeff2, coeff3);
        v2df complex_result = complex_multiply(complex1, complex2);
        v4si lookup_result = vector_table_lookup(indices, table0, table1, table2, table3);
        
        /* Mix results to create data dependencies */
        final_result_int = shuffle_result + lookup_result;
        final_result_float = poly_result;
        final_result_complex = complex_result;
        
        /* Modify inputs slightly to prevent complete optimization */
        vec_a[0] += i;
        vec_f1[0] += (float)i * 0.1f;
        complex1[0] += (double)i * 0.01;
        indices[0] = (indices[0] + 1) % 4;
    }
    
    /* Print results to create observable side effects */
    printf("Integer result: %d %d %d %d\n", 
           final_result_int[0], final_result_int[1], 
           final_result_int[2], final_result_int[3]);
    
    printf("Float result: %f %f %f %f\n",
           final_result_float[0], final_result_float[1],
           final_result_float[2], final_result_float[3]);
    
    printf("Complex result: %f + %fi\n",
           final_result_complex[0], final_result_complex[1]);
    
    return 0;
}
