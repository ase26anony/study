/* test_optabs_10_operands.c
 * Designed to trigger optabs.cc lines 8254-8263 (case 10:)
 * Compile with: gcc -O3 -march=native -ftree-vectorize -fno-inline -c test_optabs_10_operands.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually needs many operands */
static v4si complex_vector_shuffle(v4si a, v4si b, v4si c, v4si d) {
    /* This complex expression should expand to something needing many operands:
     * 1. Multiple source vectors (a, b, c, d)
     * 2. Multiple immediate control values for shuffling
     * 3. Multiple arithmetic operations with constants
     */
    
    /* Create a complex shuffle pattern using builtins with immediate arguments */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Mix with arithmetic operations using many constants */
    v4si result = temp1 + (v4si){1, 2, 3, 4};
    result = result * (v4si){2, 3, 4, 5};
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    result = result | temp2;
    
    /* Final shuffle with complex pattern */
    return __builtin_shuffle(result, temp2, (v4si){3, 2, 1, 0});
}

/* Function using vector conversions that may need many operands */
static v4sf vector_conversion_test(v4si int_vec, v4sf float_vec) {
    /* __builtin_convertvector with complex expression as source */
    v4si complex_int = int_vec * (v4si){2, 3, 4, 5} + (v4si){10, 20, 30, 40};
    complex_int = complex_int & (v4si){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    
    /* Convert with multiple steps */
    v4sf temp1 = __builtin_convertvector(complex_int, v4sf);
    v4sf temp2 = float_vec * (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
    
    /* Complex FMA-like expression */
    return temp1 * temp2 + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
}

/* Test atomic operations with many arguments (if supported) */
static long long test_atomic_ops(long long *ptr, long long a, long long b, 
                                 long long c, long long d) {
    /* Complex atomic expression */
    long long old = __atomic_fetch_add(ptr, a, __ATOMIC_SEQ_CST);
    old = __atomic_fetch_and(ptr, b, __ATOMIC_SEQ_CST);
    old = __atomic_fetch_or(ptr, c, __ATOMIC_SEQ_CST);
    old = __atomic_fetch_xor(ptr, d, __ATOMIC_SEQ_CST);
    
    /* Compare-exchange with many arguments */
    long long expected = old;
    long long desired = a + b + c + d;
    __atomic_compare_exchange(ptr, &expected, &desired, 0, 
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return old;
}

/* Main test function with loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    long long atomic_var = 0;
    long long atomic_result = 0;
    
    v4si final_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Complex vector shuffle with many operands */
        v4si shuffle_result = complex_vector_shuffle(vec_a, vec_b, vec_c, vec_d);
        
        /* Test 2: Vector conversion with many operands */
        float_result = vector_conversion_test(shuffle_result, vec_f1);
        
        /* Test 3: Atomic operations (may generate complex patterns) */
        atomic_result = test_atomic_ops(&atomic_var, 
                                        i, i*2, i*3, i*4);
        
        /* Mix results to create data dependency */
        final_result = shuffle_result + (v4si){atomic_result, atomic_result, 
                                               atomic_result, atomic_result};
        
        /* Modify inputs slightly to prevent constant folding */
        vec_a[0] += i;
        vec_b[1] += i;
        vec_c[2] += i;
        vec_d[3] += i;
    }
    
    /* Use results to prevent optimization */
    int sum = final_result[0] + final_result[1] + final_result[2] + final_result[3];
    printf("Result: %d (atomic: %lld)\n", sum, atomic_result);
    
    /* Also print float result to use it */
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    printf("Float result: %f\n", fsum);
    
    return 0;
}
