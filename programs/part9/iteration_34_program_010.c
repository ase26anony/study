/* test_optabs.c - Target GCC optabs.cc lines 8254-8263 (case 10) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle with many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This should expand to something needing 10 operands:
     * 4 source vectors + 6 immediate control values
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control */
    /* Control mask: {0,4,1,5,2,6,3,7} - interleaves a and b */
    result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Another shuffle with different control pattern */
    v4si tmp = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Combine results - this may trigger complex expansion */
    result = result + tmp;
    
    return result;
}

/* Vector conversion with many operands */
static v4sf convert_and_shuffle(v4si a, v4si b, v4sf c, v4sf d) {
    /* Multiple operations that may combine into one expansion */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    
    /* Complex shuffle pattern with immediate control */
    v4sf shuffled = __builtin_shuffle(fa, fb, (v4si){3, 2, 1, 0});
    
    /* Fused multiply-add style operation with constants */
    v4sf result = shuffled * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} 
                + c * (v4sf){0.5f, 1.5f, 2.5f, 3.5f}
                + d * (v4sf){0.25f, 0.75f, 1.25f, 1.75f};
    
    return result;
}

/* Atomic-style operation simulation with many parameters */
static v2di multi_operand_atomic_like(v2di a, v2di b, v2di c, v2di mask) {
    /* Complex bitwise operation with multiple constants */
    v2di result = (a & (v2di){0xFF00FF00FF00FF00LL, 0x00FF00FF00FF00FFLL})
                | (b & (v2di){0x00FF00FF00FF00FFLL, 0xFF00FF00FF00FF00LL})
                ^ (c & mask)
                | (v2di){0xAAAAAAAAAAAAAAAALL, 0x5555555555555555LL};
    
    /* Additional shuffle with control */
    result = __builtin_shuffle(result, 
                              (v2di){0x1111111111111111LL, 0x2222222222222222LL},
                              (v2di){1, 0});
    
    return result;
}

/* Main test function with non-optimizable computation */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf fvec2 = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    
    v2di dvec1 = {(long long)argc, (long long)(argc + 100)};
    v2di dvec2 = {(long long)(argc * 200), (long long)(argc * 300)};
    v2di dvec3 = {(long long)(argc * 400), (long long)(argc * 500)};
    v2di mask = {0xF0F0F0F0F0F0F0F0LL, 0x0F0F0F0F0F0F0F0FLL};
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v2di double_result = {0, 0};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operand(vec1, vec2, vec3, vec4);
        float_result = convert_and_shuffle(vec1, vec2, fvec1, fvec2);
        double_result = multi_operand_atomic_like(dvec1, dvec2, dvec3, mask);
        
        /* Mix results to create data dependencies */
        vec1 = vec1 + int_result;
        fvec1 = fvec1 + float_result;
        dvec1 = dvec1 ^ double_result;
    }
    
    /* Use results to prevent dead code elimination */
    int sum = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    long long dsum = double_result[0] + double_result[1];
    
    printf("Results: int=%d, float=%f, double=%lld\n", sum, fsum, dsum);
    
    return (sum > 0) ? 0 : 1;
}

/* Additional test with explicit built-in using many arguments */
void test_explicit_builtins(void) {
    /* Using vector built-ins that may require many operands */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = {9, 10, 11, 12};
    v4si v4 = {13, 14, 15, 16};
    
    /* Complex expression that might be expanded as a single operation
     * with 10 operands: 4 vectors + 6 control values/constants */
    v4si complex_result = 
        __builtin_shuffle(
            __builtin_shuffle(v1, v2, (v4si){0, 4, 1, 5}),
            __builtin_shuffle(v3, v4, (v4si){2, 6, 3, 7}),
            (v4si){0, 1, 4, 5}
        ) * (v4si){2, 3, 4, 5}
        + (v4si){10, 20, 30, 40};
    
    /* Use volatile to ensure computation happens */
    volatile v4si sink = complex_result;
    (void)sink;
}
