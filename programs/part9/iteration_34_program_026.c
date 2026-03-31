/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually requires many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern aims to trigger case 10:
     * 4 source vectors + 6 immediate control values = 10 operands
     * The compiler may expand this into an internal function requiring
     * exactly 10 arguments during RTL expansion.
     */
    
    /* Create a complex shuffle with multiple control values */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Another layer of shuffle with different control values */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){3, 2, 1, 0});
    
    /* Mix with bitwise operations and constants */
    result = result ^ (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    result = result | (v4si){0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000};
    result = result & (v4si){0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000};
    
    return result;
}

/* Vector conversion with many operands */
static v4sf vector_conversion_complex(v2di a, v2di b, v2df c, v2df d) {
    /* Complex conversion pattern that may require many operands */
    v4si int_vec1 = __builtin_convertvector(a, v4si);
    v4si int_vec2 = __builtin_convertvector(b, v4si);
    
    /* Mix with floating point conversions */
    v4sf float_vec1 = __builtin_convertvector(int_vec1, v4sf);
    v4sf float_vec2 = __builtin_convertvector(int_vec2, v4sf);
    
    /* Complex operation with multiple constants */
    v4sf result = float_vec1 * (v4sf){1.41421356f, 2.71828182f, 3.14159265f, 1.61803398f}
                + float_vec2 * (v4sf){0.57721566f, 1.73205080f, 2.23606797f, 2.64575131f};
    
    return result;
}

/* Atomic-style operation simulation with many arguments */
static v4si atomic_style_operation(v4si mem, v4si val, v4si mask) {
    /* Simulate complex atomic operation that may expand to many operands */
    v4si result;
    
    /* Complex bit manipulation with multiple constants */
    result = (mem & mask) | (val & ~mask);
    result = result ^ (v4si){0x12345678, 0x9ABCDEF0, 0x12345678, 0x9ABCDEF0};
    result = (result << 2) | (result >> 30);  /* Rotate */
    result = result + (v4si){1, 2, 3, 4};
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = 2;  /* Prevent constant propagation */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 5 + 1;  /* Small number of iterations */
    }
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v2di vec_di1 = {0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL};
    v2di vec_di2 = {0x1122334455667788LL, 0x8877665544332211LL};
    v2df vec_df1 = {3.141592653589793, 2.718281828459045};
    v2df vec_df2 = {1.414213562373095, 1.618033988749895};
    
    v4si mask = {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000};
    
    v4si final_result_int = {0, 0, 0, 0};
    v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        v4si shuffle_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        v4sf convert_result = vector_conversion_complex(vec_di1, vec_di2, vec_df1, vec_df2);
        v4si atomic_result = atomic_style_operation(shuffle_result, vec_a, mask);
        
        /* Accumulate results to create data dependencies */
        final_result_int = final_result_int + shuffle_result + atomic_result;
        
        /* Convert float result to int and accumulate */
        v4si float_as_int = __builtin_convertvector(convert_result, v4si);
        final_result_int = final_result_int + float_as_int;
        
        /* Modify source vectors slightly to prevent constant folding */
        vec_a = vec_a + (v4si){1, 1, 1, 1};
        vec_b = vec_b + (v4si){2, 2, 2, 2};
    }
    
    /* Use the result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += final_result_int[i];
    }
    
    printf("Result checksum: %d\n", sum);
    
    /* Also print float result to ensure both paths are used */
    float fsum = 0.0f;
    for (int i = 0; i < 4; i++) {
        fsum += final_result_float[i];
    }
    printf("Float checksum: %f\n", fsum);
    
    return sum != 0 ? 0 : 1;  /* Return non-zero if everything was optimized away */
}
