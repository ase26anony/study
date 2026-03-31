/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values for shuffle indices = 6 operands
     * Total: 10 operands for the expansion
     */
    
    /* Create a complex shuffle using multiple builtins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Final shuffle combining results with more control values */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){2, 5, 0, 7});
    
    return result;
}

/* Vector conversion with many operands */
static v4sf convert_and_blend(v4si a, v4si b, v4sf c, v4sf d) {
    /* Convert vectors and blend with multiple control values */
    v4sf conv_a = __builtin_convertvector(a, v4sf);
    v4sf conv_b = __builtin_convertvector(b, v4sf);
    
    /* Complex blend operation with immediate control */
    v4sf result = __builtin_ia32_blendps256(
        __builtin_ia32_blendps256(conv_a, conv_b, 0x5),
        __builtin_ia32_blendps256(c, d, 0xA),
        0x3
    );
    
    return result;
}

/* Multi-operand arithmetic expression */
static v4sf complex_fma_like(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Expression designed to expand into multiple operations:
     * a * 3.14f + b * 2.71f + c * 1.41f + d * 1.62f
     * Each constant becomes an operand during expansion
     */
    v4sf const1 = (v4sf){3.14f, 3.14f, 3.14f, 3.14f};
    v4sf const2 = (v4sf){2.71f, 2.71f, 2.71f, 2.71f};
    v4sf const3 = (v4sf){1.41f, 1.41f, 1.41f, 1.41f};
    v4sf const4 = (v4sf){1.62f, 1.62f, 1.62f, 1.62f};
    
    return a * const1 + b * const2 + c * const3 + d * const4;
}

/* Cryptographic-like permutation pattern */
static v2di table_lookup_style(v2di a, v2di b, v2di c, v2di d) {
    /* Simulate a table lookup/permutation requiring many operands */
    v2di mask1 = (v2di){0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL};
    v2di mask2 = (v2di){0xFFFF0000FFFF0000ULL, 0x0000FFFF0000FFFFULL};
    
    /* Complex bitwise operations with multiple constants */
    v2di temp1 = (a & mask1) | (b & ~mask1);
    v2di temp2 = (c & mask2) | (d & ~mask2);
    
    /* Final permutation */
    v2di result = __builtin_shuffle(temp1, temp2, (v2di){1, 0});
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec_int1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_int2 = {argc + 4, argc + 5, argc + 6, argc + 7};
    v4si vec_int3 = {argc + 8, argc + 9, argc + 10, argc + 11};
    v4si vec_int4 = {argc + 12, argc + 13, argc + 14, argc + 15};
    
    v4sf vec_float1 = {argc * 1.0f, argc * 2.0f, argc * 3.0f, argc * 4.0f};
    v4sf vec_float2 = {argc * 5.0f, argc * 6.0f, argc * 7.0f, argc * 8.0f};
    v4sf vec_float3 = {argc * 9.0f, argc * 10.0f, argc * 11.0f, argc * 12.0f};
    v4sf vec_float4 = {argc * 13.0f, argc * 14.0f, argc * 15.0f, argc * 16.0f};
    
    v2di vec_long1 = {argc * 100LL, argc * 200LL};
    v2di vec_long2 = {argc * 300LL, argc * 400LL};
    v2di vec_long3 = {argc * 500LL, argc * 600LL};
    v2di vec_long4 = {argc * 700LL, argc * 800LL};
    
    /* Accumulator to prevent dead code elimination */
    v4si acc_int = {0, 0, 0, 0};
    v4sf acc_float = {0.0f, 0.0f, 0.0f, 0.0f};
    v2di acc_long = {0, 0};
    
    /* Loop to execute operations multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Execute all complex operations */
        acc_int = acc_int + complex_shuffle_10op(vec_int1, vec_int2, vec_int3, vec_int4);
        acc_float = acc_float + convert_and_blend(vec_int1, vec_int2, vec_float1, vec_float2);
        acc_float = acc_float + complex_fma_like(vec_float1, vec_float2, vec_float3, vec_float4);
        acc_long = acc_long + table_lookup_style(vec_long1, vec_long2, vec_long3, vec_long4);
        
        /* Modify inputs slightly to prevent loop invariant removal */
        vec_int1[0] += i;
        vec_float1[0] += i * 0.1f;
        vec_long1[0] += i * 10LL;
    }
    
    /* Print results to create side effects */
    printf("Integer result: %d %d %d %d\n", 
           acc_int[0], acc_int[1], acc_int[2], acc_int[3]);
    printf("Float result: %f %f %f %f\n",
           acc_float[0], acc_float[1], acc_float[2], acc_float[3]);
    printf("Long result: %lld %lld\n",
           acc_long[0], acc_long[1]);
    
    return 0;
}
