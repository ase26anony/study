/* test_optabs.c - Program to trigger 10-operand optab expansion */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle function that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                        int idx0, int idx1, int idx2, int idx3,
                                        int idx4, int idx5) {
    /* This complex shuffle pattern should expand to something needing 10 operands:
     * 4 source vectors + 6 immediate indices = 10 operands
     * The compiler may lower this to an internal function with many arguments
     */
    
    /* Create a control vector from the 6 indices (using only 4 for v4si) */
    int control[4] = {idx0, idx1, idx2, idx3};
    
    /* Complex shuffle using __builtin_shuffle with multiple vectors */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){control[0], control[1], control[2], control[3]});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){control[0] ^ idx4, control[1] ^ idx5, 
                                                control[2] ^ idx4, control[3] ^ idx5});
    
    /* Final blend/shuffle that mixes all 4 vectors */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){idx4, idx5, idx0, idx1});
    
    return result;
}

/* Function using vector conversions with many operands */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d,
                                      float scale0, float scale1, 
                                      float scale2, float scale3,
                                      int offset0, int offset1) {
    /* Complex conversion pattern that may expand to many operands */
    
    /* First convert vectors to float */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Apply scaling factors - this creates a pattern with many constants */
    v4sf scales = {scale0, scale1, scale2, scale3};
    v4sf offsets = {(float)offset0, (float)offset1, (float)offset0, (float)offset1};
    
    /* Complex expression with many operands */
    v4sf result = fa * scales + fb * (scales * 2.0f) + 
                  fc * (scales * 3.0f) + fd * (scales * 4.0f) + offsets;
    
    return result;
}

/* Function using atomic built-in with many arguments */
static long long atomic_complex_operation(long long *ptr1, long long *ptr2,
                                          long long *ptr3, long long *ptr4,
                                          long long val1, long long val2,
                                          long long val3, long long val4,
                                          int mem_order1, int mem_order2) {
    /* Complex atomic pattern - though atomic built-ins typically have fewer args,
     * the expansion might need many operands for memory barriers, etc. */
    
    long long result = 0;
    
    /* Perform multiple atomic operations */
    result += __atomic_fetch_add(ptr1, val1, mem_order1);
    result += __atomic_fetch_sub(ptr2, val2, mem_order2);
    result ^= __atomic_fetch_and(ptr3, val3, __ATOMIC_RELAXED);
    result |= __atomic_fetch_or(ptr4, val4, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with non-trivial patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4si shuffle_result = {0};
    v4sf convert_result = {0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with many operands */
        shuffle_result = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            i, i+1, i+2, i+3, i+4, i+5);
        
        /* Test 2: Vector conversion with many operands */
        convert_result = vector_conversion_complex(
            vec_a, vec_b, vec_c, vec_d,
            1.5f + i, 2.5f + i, 3.5f + i, 4.5f + i,
            i * 10, i * 20);
        
        /* Modify vectors slightly for next iteration */
        vec_a += (v4si){1, 1, 1, 1};
        vec_b += (v4si){2, 2, 2, 2};
    }
    
    /* Test 3: Atomic operations */
    long long atomic_vars[4] = {100, 200, 300, 400};
    long long atomic_result = atomic_complex_operation(
        &atomic_vars[0], &atomic_vars[1], &atomic_vars[2], &atomic_vars[3],
        10, 20, 30, 40,
        __ATOMIC_ACQ_REL, __ATOMIC_SEQ_CST);
    
    /* Print results to prevent optimization */
    printf("Shuffle result: %d %d %d %d\n", 
           shuffle_result[0], shuffle_result[1], 
           shuffle_result[2], shuffle_result[3]);
    
    printf("Convert result: %.2f %.2f %.2f %.2f\n",
           convert_result[0], convert_result[1],
           convert_result[2], convert_result[3]);
    
    printf("Atomic result: %lld\n", atomic_result);
    printf("Atomic vars: %lld %lld %lld %lld\n",
           atomic_vars[0], atomic_vars[1], atomic_vars[2], atomic_vars[3]);
    
    return 0;
}
