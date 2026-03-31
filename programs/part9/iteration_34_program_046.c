/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                       int idx0, int idx1, int idx2, int idx3,
                                       int idx4, int idx5) {
    /* This complex shuffle pattern conceptually needs:
       - 4 source vectors (a, b, c, d) = 4 operands
       - 6 immediate indices = 6 operands
       Total: 10 operands
       
       We'll implement it using multiple builtins that might get combined */
    
    /* First, create a 16-element "virtual" array from our 4 vectors */
    /* Each v4si has 4 elements, so 4 vectors = 16 elements */
    
    /* Extract elements using GCC builtins - these may expand to multi-operand patterns */
    int e0 = __builtin_shufflevector(a, b, idx0 % 8);
    int e1 = __builtin_shufflevector(b, c, idx1 % 8);
    int e2 = __builtin_shufflevector(c, d, idx2 % 8);
    int e3 = __builtin_shufflevector(d, a, idx3 % 8);
    
    /* Combine using another shuffle with immediate control */
    v4si temp1 = __builtin_shufflevector(a, b, idx0, idx1, idx2, idx3);
    v4si temp2 = __builtin_shufflevector(c, d, idx4, idx5, idx0, idx1);
    
    /* Final blend operation - this is where the 10-operand expansion might occur */
    /* We're creating a pattern that might be recognized as a complex permute */
    v4si result = __builtin_shufflevector(temp1, temp2, 
                                         (idx0 + idx4) % 4,
                                         (idx1 + idx5) % 4,
                                         (idx2 + idx0) % 4,
                                         (idx3 + idx1) % 4);
    
    return result;
}

/* Vector conversion with many constants - another candidate for 10-operand expansion */
static v4sf vector_convert_with_constants(v4si src, 
                                         float c0, float c1, float c2, float c3,
                                         float m0, float m1, float m2, float m3) {
    /* Convert int vector to float vector with scaling by constants */
    /* This pattern might expand to something like:
       result[i] = src[i] * constants[i] + multipliers[i] */
    
    v4sf result;
    
    /* Use __builtin_convertvector which might internally use multi-operand optabs */
    v4sf converted = __builtin_convertvector(src, v4sf);
    
    /* Create constant vectors - these become immediate operands */
    v4sf constants = {c0, c1, c2, c3};
    v4sf multipliers = {m0, m1, m2, m3};
    
    /* Complex expression that might require many operands during expansion */
    result = converted * constants + multipliers;
    
    return result;
}

/* Atomic-style operation simulation with many parameters */
static long long atomic_style_operation(volatile long long *ptr,
                                       long long a, long long b, long long c,
                                       long long d, long long e, long long f,
                                       long long g, long long h) {
    /* Simulate a complex atomic update that might expand to 10+ operands */
    long long old = *ptr;
    long long new_val = ((old * a + b) ^ c) & d | (e << f) + (g >> h);
    *ptr = new_val;
    return old;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    /* Shuffle indices - use argc to make them non-constant */
    int idx0 = argc % 8;
    int idx1 = (argc + 1) % 8;
    int idx2 = (argc + 2) % 8;
    int idx3 = (argc + 3) % 8;
    int idx4 = (argc + 4) % 8;
    int idx5 = (argc + 5) % 8;
    
    /* Conversion constants */
    float conv_consts[] = {1.5f, 2.5f, 3.5f, 4.5f};
    float mult_consts[] = {0.1f * argc, 0.2f * argc, 0.3f * argc, 0.4f * argc};
    
    volatile v4si final_result_int;
    volatile v4sf final_result_float;
    volatile long long atomic_result = 0;
    
    /* Loop to prevent optimization but keep it small */
    for (int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with many operands */
        v4si shuffled = complex_shuffle_10_operands(vec_a, vec_b, vec_c, vec_d,
                                                   idx0 + i, idx1 + i, idx2 + i,
                                                   idx3 + i, idx4 + i, idx5 + i);
        final_result_int = shuffled;
        
        /* Test 2: Vector conversion with many constants */
        v4sf converted = vector_convert_with_constants(
            vec_a,
            conv_consts[0] + i * 0.1f,
            conv_consts[1] + i * 0.1f,
            conv_consts[2] + i * 0.1f,
            conv_consts[3] + i * 0.1f,
            mult_consts[0] + i * 0.01f,
            mult_consts[1] + i * 0.01f,
            mult_consts[2] + i * 0.01f,
            mult_consts[3] + i * 0.01f
        );
        final_result_float = converted;
        
        /* Test 3: Atomic-style operation with many parameters */
        long long atomic_val = atomic_style_operation(&atomic_result,
                                                     argc + i, argc * 2 + i,
                                                     argc * 3 + i, argc * 4 + i,
                                                     argc * 5 + i, argc * 6 + i,
                                                     argc * 7 + i, argc * 8 + i);
        (void)atomic_val; /* Use result to prevent optimization */
    }
    
    /* Print results to create side effects */
    printf("Shuffle result: [%d, %d, %d, %d]\n",
           final_result_int[0], final_result_int[1],
           final_result_int[2], final_result_int[3]);
    
    printf("Conversion result: [%f, %f, %f, %f]\n",
           (double)final_result_float[0], (double)final_result_float[1],
           (double)final_result_float[2], (double)final_result_float[3]);
    
    printf("Atomic result: %lld\n", (long long)atomic_result);
    
    return 0;
}
