/* test_optabs_10_operands.c
 * Designed to trigger the 10-operand expansion case in optabs.cc
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -c test_optabs_10_operands.c -fdump-rtl-expand
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                        int idx0, int idx1, int idx2, int idx3,
                                        int idx4, int idx5) {
    /* This complex expression should expand to something needing 10 operands:
     * 4 source vectors + 6 immediate indices = 10 operands
     */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, idx2, idx3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){idx4, idx5, 2, 3});
    
    /* Mix them with bitwise operations using constants */
    v4si mask1 = (v4si){0xFF00FF00, 0x00FF00FF, 0xFF00FF00, 0x00FF00FF};
    v4si mask2 = (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F};
    
    v4si result = (temp1 & mask1) | (temp2 & mask2);
    result = result ^ (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    
    return result;
}

/* Another approach using vector conversions and arithmetic */
static v4sf complex_fma_chain(v4sf a, v4sf b, v4sf c, v4sf d,
                              float k1, float k2, float k3, float k4,
                              float k5, float k6) {
    /* Fused multiply-add chain with many constants */
    v4sf kvec1 = (v4sf){k1, k2, k3, k4};
    v4sf kvec2 = (v4sf){k5, k6, k1, k2};
    
    /* Complex expression that might expand to 10 operands */
    v4sf result = a * kvec1 + b * kvec2;
    result = result * c + d * (v4sf){k3, k4, k5, k6};
    
    /* Additional operations to prevent simplification */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Use atomic built-in with many arguments (if available) */
#ifdef __ATOMIC_RELAXED
static long long atomic_cmpxchg_10_args(long long *ptr, long long oldval, 
                                        long long newval, int memorder1,
                                        int memorder2, int memorder3,
                                        int memorder4, int memorder5,
                                        int memorder6, int memorder7) {
    /* __atomic_compare_exchange can have many parameters */
    long long expected = oldval;
    int weak = 0;
    
    __atomic_compare_exchange(ptr, &expected, &newval, weak,
                              memorder1, memorder2);
    
    /* Chain multiple atomics to increase operand count */
    long long temp = __atomic_fetch_add(ptr, newval, memorder3);
    temp = __atomic_fetch_and(ptr, oldval, memorder4);
    temp = __atomic_fetch_or(ptr, newval, memorder5);
    temp = __atomic_fetch_xor(ptr, oldval, memorder6);
    
    return __atomic_fetch_nand(ptr, newval, memorder7);
}
#endif

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = (v4si){argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = (v4si){argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = (v4si){argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = (v4si){argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec_a = (v4sf){argc * 1.1f, argc * 1.2f, argc * 1.3f, argc * 1.4f};
    v4sf fvec_b = (v4sf){argc * 1.5f, argc * 1.6f, argc * 1.7f, argc * 1.8f};
    v4sf fvec_c = (v4sf){argc * 1.9f, argc * 2.0f, argc * 2.1f, argc * 2.2f};
    v4sf fvec_d = (v4sf){argc * 2.3f, argc * 2.4f, argc * 2.5f, argc * 2.6f};
    
    v4si final_result = (v4si){0, 0, 0, 0};
    v4sf final_fresult = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call the 10-operand shuffle function */
        v4si shuffle_result = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            i & 3, (i + 1) & 3, (i + 2) & 3,
            (i + 3) & 3, (i + 4) & 3, (i + 5) & 3
        );
        
        /* Call the FMA chain function */
        v4sf fma_result = complex_fma_chain(
            fvec_a, fvec_b, fvec_c, fvec_d,
            i * 0.1f, i * 0.2f, i * 0.3f,
            i * 0.4f, i * 0.5f, i * 0.6f
        );
        
        /* Mix results to create data dependency */
        final_result += shuffle_result;
        final_fresult += fma_result;
        
        /* Modify input vectors slightly to prevent loop invariant removal */
        vec_a[0] += i;
        fvec_a[0] += i * 0.01f;
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Vector result: [%d, %d, %d, %d]\n", 
           final_result[0], final_result[1], 
           final_result[2], final_result[3]);
    
    printf("Float result: [%f, %f, %f, %f]\n",
           final_fresult[0], final_fresult[1],
           final_fresult[2], final_fresult[3]);
    
    return final_result[0] + (int)final_fresult[0];
}

/* Additional test targeting specific vector built-ins */
void test_vector_builtins() {
    v4si a = (v4si){1, 2, 3, 4};
    v4si b = (v4si){5, 6, 7, 8};
    v4si c = (v4si){9, 10, 11, 12};
    v4si d = (v4si){13, 14, 15, 16};
    
    /* Complex expression that might need many operands during expansion */
    v4si result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    result = __builtin_shuffle(result, c, (v4si){2, 6, 3, 7});
    result = __builtin_shuffle(result, d, (v4si){0, 4, 1, 5});
    
    /* Use __builtin_convertvector with complex pattern */
    v4sf float_vec = __builtin_convertvector(result, v4sf);
    v4si int_vec = __builtin_convertvector(float_vec, v4si);
    
    /* Complex bitwise operations with many constants */
    int_vec = int_vec & (v4si){0xFFFF0000, 0x0000FFFF, 0xFFFF0000, 0x0000FFFF};
    int_vec = int_vec | (v4si){0x0000AAAA, 0xAAAA0000, 0x0000AAAA, 0xAAAA0000};
    int_vec = int_vec ^ (v4si){0x55555555, 0xAAAAAAAA, 0x55555555, 0xAAAAAAAA};
    
    /* Prevent optimization */
    volatile v4si dont_optimize = int_vec;
    (void)dont_optimize;
}
