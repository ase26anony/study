/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle function that requires many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually needs:
     * 4 source vectors (a, b, c, d)
     * 6 immediate control values for shuffle indices
     * Total: 10 operands
     */
    
    /* Create a complex shuffle using multiple built-ins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Final shuffle combining results - this may expand to 10 operands */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){3, 0, 5, 2});
    
    return result;
}

/* Vector blend with many constants - potentially 10 operands */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation with many constants */
    v4sf result;
    
    /* This series of operations creates a pattern that may require
     * many operands during expansion */
    result = a * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
             b * (v4sf){5.0f, 6.0f, 7.0f, 8.0f} +
             c * (v4sf){9.0f, 10.0f, 11.0f, 12.0f} +
             d * (v4sf){13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Additional shuffle to potentially trigger 10-operand expansion */
    result = __builtin_shuffle(result, (v4sf){0.0f, 0.0f, 0.0f, 0.0f}, 
                              (v4si){3, 2, 1, 0});
    
    return result;
}

/* Table lookup simulation with many operands */
static v4si table_lookup_10op(v4si table0, v4si table1, 
                              v4si table2, v4si table3,
                              v4si indices) {
    /* Simulate a 4-table lookup - may expand to operation with 10 operands:
     * 4 table vectors
     * 1 indices vector  
     * 5 control constants for the lookup pattern
     */
    
    /* Extract individual indices */
    int idx0 = indices[0] & 3;
    int idx1 = indices[1] & 3;
    int idx2 = indices[2] & 3;
    int idx3 = indices[3] & 3;
    
    /* Select from appropriate table - this pattern may trigger
     * complex expansion with many operands */
    v4si result;
    
    /* Use conditional selects that may expand to multi-operand operation */
    v4si mask0 = (v4si){idx0 == 0, idx1 == 0, idx2 == 0, idx3 == 0};
    v4si mask1 = (v4si){idx0 == 1, idx1 == 1, idx2 == 1, idx3 == 1};
    v4si mask2 = (v4si){idx0 == 2, idx1 == 2, idx2 == 2, idx3 == 2};
    v4si mask3 = (v4si){idx0 == 3, idx1 == 3, idx2 == 3, idx3 == 3};
    
    result = (mask0 & table0) | (mask1 & table1) | 
             (mask2 & table2) | (mask3 & table3);
    
    return result;
}

/* Main function with non-trivial computation to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = 3; /* Prevent constant propagation */
    if (argc > 1) {
        iterations = atoi(argv[1]) & 3; /* Limit to 0-3 */
        if (iterations == 0) iterations = 1;
    }
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf vec_af = {(float)argc, (float)(argc + 1), 
                   (float)(argc + 2), (float)(argc + 3)};
    v4sf vec_bf = {(float)(argc * 2), (float)(argc * 3),
                   (float)(argc * 4), (float)(argc * 5)};
    v4sf vec_cf = {(float)(argc * 6), (float)(argc * 7),
                   (float)(argc * 8), (float)(argc * 9)};
    v4sf vec_df = {(float)(argc * 10), (float)(argc * 11),
                   (float)(argc * 12), (float)(argc * 13)};
    
    v4si result_int = {0, 0, 0, 0};
    v4sf result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        result_int = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        
        /* Mix with another potentially 10-operand operation */
        result_float = vector_blend_complex(vec_af, vec_bf, vec_cf, vec_df);
        
        /* Table lookup with many operands */
        v4si indices = {i, i + 1, i + 2, i + 3};
        v4si table_result = table_lookup_10op(vec_a, vec_b, vec_c, vec_d, indices);
        
        /* Combine results to ensure they're used */
        result_int = result_int + table_result;
    }
    
    /* Print results to create side effects and prevent dead code elimination */
    printf("Result int: %d %d %d %d\n", 
           result_int[0], result_int[1], result_int[2], result_int[3]);
    printf("Result float: %f %f %f %f\n",
           result_float[0], result_float[1], result_float[2], result_float[3]);
    
    return 0;
}
