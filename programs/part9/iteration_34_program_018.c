/* test_optabs.c - Target GCC's 10-operand expansion path */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle that conceptually needs many operands */
static v4si complex_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This shuffle pattern uses 4 source vectors and 6 control values
       Conceptually needs: 4 vectors + 6 indices = 10 operands
       The compiler may expand this into a multi-operand optab entry */
    
    /* Create control vector with 6 immediate values mixed with computations */
    v4si control1 = {0, 4, 1, 5};  /* indices for first two vectors */
    v4si control2 = {2, 6, 3, 7};  /* indices for second two vectors */
    
    /* Complex shuffle expression that may require many operands during expansion */
    v4si result = __builtin_shuffle(a, b, control1);
    v4si temp = __builtin_shuffle(c, d, control2);
    
    /* Mix them with bitwise operations using multiple constants */
    result = result ^ (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    temp = temp | (v4si){0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000};
    
    /* Final blend - conceptually needs many operands */
    return __builtin_shuffle(result, temp, (v4si){0, 4, 1, 5});
}

/* Vector conversion with many operands */
static v4sf convert_and_blend(v4si a, v4si b, v4si c, v4si d) {
    /* Multiple conversions and blends that may expand to many operands */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Complex expression with many constants - may trigger multi-operand expansion */
    v4sf result = fa * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} +
                  fb * (v4sf){5.0f, 6.0f, 7.0f, 8.0f} +
                  fc * (v4sf){9.0f, 10.0f, 11.0f, 12.0f} +
                  fd * (v4sf){13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Additional shuffle with immediate control */
    return __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
}

/* Atomic-style operation simulation */
static v2di atomic_style_op(v2di a, v2di b, v2di c, v2di d) {
    /* Complex bitwise expression with many constants */
    v2di mask1 = (v2di){0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL};
    v2di mask2 = (v2di){0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL};
    v2di mask3 = (v2di){0xCCCCCCCCCCCCCCCCULL, 0x3333333333333333ULL};
    
    /* Expression that may expand to many operands */
    v2di result = (a & mask1) | (b & mask2);
    result = result ^ (c & mask3);
    result = result | (d & (v2di){0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL});
    
    /* Final rotate/shift with immediate */
    return (result << 1) | (result >> 63);
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with pattern */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    
    v2di vec1d = {0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL};
    v2di vec2d = {0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL};
    v2di vec3d = {0xCCCCCCCCCCCCCCCCULL, 0x3333333333333333ULL};
    v2di vec4d = {0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL};
    
    v4si int_result = {0};
    v4sf float_result = {0.0f};
    v2di long_result = {0};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10op(vec1, vec2, vec3, vec4);
        float_result = convert_and_blend(vec1, vec2, vec3, vec4);
        long_result = atomic_style_op(vec1d, vec2d, vec3d, vec4d);
        
        /* Modify inputs slightly to prevent constant folding */
        vec1 += (v4si){1, 1, 1, 1};
        vec1d += (v2di){1, 1};
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float results: %f %f %f %f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    printf("Long results: %llx %llx\n",
           (unsigned long long)long_result[0],
           (unsigned long long)long_result[1]);
    
    return 0;
}
