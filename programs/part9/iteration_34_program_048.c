/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that requires many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                       int idx0, int idx1, int idx2, int idx3,
                                       int idx4, int idx5) {
    /* This should trigger a 10-operand expansion:
       - 4 vector inputs (a, b, c, d)
       - 6 immediate indices (idx0-idx5)
       Total: 10 operands
    */
    
    /* Create a complex shuffle pattern using __builtin_shuffle */
    /* First, shuffle a and b based on first two indices */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){idx0, idx1, idx2, idx3});
    
    /* Then shuffle c and d based on next two indices */
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){idx4, idx5, idx0, idx1});
    
    /* Final shuffle combining both results */
    return __builtin_shuffle(ab_shuffle, cd_shuffle, (v4si){idx2, idx3, idx4, idx5});
}

/* Another approach: Vector conversion with many operands */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d,
                                     float scale0, float scale1, 
                                     float scale2, float scale3,
                                     int offset0, int offset1) {
    /* Convert vectors with scaling and offset - potentially 10 operands */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Apply different scales to each vector */
    v4sf scales = {scale0, scale1, scale2, scale3};
    fa = fa * scales;
    fb = fb * scales;
    fc = fc * scales;
    fd = fd * scales;
    
    /* Shuffle with offsets */
    v4si offset_mask = {offset0, offset1, offset0 + 1, offset1 + 1};
    v4sf result = __builtin_shuffle(fa, fb, offset_mask);
    v4sf result2 = __builtin_shuffle(fc, fd, offset_mask);
    
    return result + result2;
}

/* Use atomic built-in with many arguments (if available) */
#ifdef __ATOMIC_RELAXED
static long long atomic_ops_10_args(volatile long long *ptr1, 
                                   volatile long long *ptr2,
                                   volatile long long *ptr3,
                                   volatile long long *ptr4,
                                   long long val1, long long val2,
                                   long long val3, long long val4,
                                   int memorder1, int memorder2) {
    /* Multiple atomic operations that might be combined */
    __atomic_fetch_add(ptr1, val1, memorder1);
    __atomic_fetch_add(ptr2, val2, memorder1);
    __atomic_fetch_add(ptr3, val3, memorder2);
    __atomic_fetch_add(ptr4, val4, memorder2);
    
    return __atomic_load_n(ptr1, __ATOMIC_ACQUIRE) +
           __atomic_load_n(ptr2, __ATOMIC_ACQUIRE) +
           __atomic_load_n(ptr3, __ATOMIC_ACQUIRE) +
           __atomic_load_n(ptr4, __ATOMIC_ACQUIRE);
}
#endif

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent dead code elimination */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    /* Shuffle indices - these become immediate values */
    int idx0 = 0, idx1 = 1, idx2 = 2, idx3 = 3;
    int idx4 = 4, idx5 = 5;
    
    v4si result = {0, 0, 0, 0};
    
    /* Loop to ensure the expression is executed but not optimized away */
    for (volatile int i = 0; i < iterations; i++) {
        /* Modify indices slightly each iteration to prevent constant folding */
        idx0 = (idx0 + i) % 8;
        idx1 = (idx1 + i * 2) % 8;
        idx2 = (idx2 + i * 3) % 8;
        idx3 = (idx3 + i * 4) % 8;
        idx4 = (idx4 + i * 5) % 8;
        idx5 = (idx5 + i * 6) % 8;
        
        /* Call the complex shuffle function - this is the key expression
           that should trigger 10-operand expansion */
        v4si temp = complex_shuffle_10_operands(vec_a, vec_b, vec_c, vec_d,
                                               idx0, idx1, idx2, idx3,
                                               idx4, idx5);
        
        /* Accumulate results to prevent elimination */
        result = result + temp;
        
        /* Also test vector conversion */
        if (i % 3 == 0) {
            v4sf float_result = vector_conversion_complex(vec_a, vec_b, vec_c, vec_d,
                                                         1.0f + i * 0.1f, 2.0f + i * 0.2f,
                                                         3.0f + i * 0.3f, 4.0f + i * 0.4f,
                                                         i % 4, (i + 1) % 4);
            /* Use the result somehow */
            volatile float dummy __attribute__((unused));
            dummy = ((float*)&float_result)[0];
        }
    }
    
    /* Print result to create side effect */
    printf("Result: [%d, %d, %d, %d]\n", 
           result[0], result[1], result[2], result[3]);
    
    /* Test atomic operations if available */
#ifdef __ATOMIC_RELAXED
    if (iterations > 50) {
        volatile long long atomic_var1 = 0;
        volatile long long atomic_var2 = 0;
        volatile long long atomic_var3 = 0;
        volatile long long atomic_var4 = 0;
        
        long long atomic_result = atomic_ops_10_args(
            (volatile long long*)&atomic_var1,
            (volatile long long*)&atomic_var2,
            (volatile long long*)&atomic_var3,
            (volatile long long*)&atomic_var4,
            1, 2, 3, 4,
            __ATOMIC_RELAXED, __ATOMIC_SEQ_CST);
        
        printf("Atomic result: %lld\n", atomic_result);
    }
#endif
    
    return result[0] != 0 ? 0 : 1;
}

/* Additional test: Matrix multiplication pattern that might use many operands */
v4sf matrix_multiply_4x4(v4sf row0, v4sf row1, v4sf row2, v4sf row3,
                        v4sf col0, v4sf col1, v4sf col2, v4sf col3,
                        float bias0, float bias1) {
    /* This pattern might be expanded into a 10-operand operation
       when vectorized and optimized */
    
    /* Dot products with bias addition */
    v4sf result;
    
    /* Each component requires multiple operations that could be combined */
    result[0] = row0[0] * col0[0] + row0[1] * col1[0] + 
                row0[2] * col2[0] + row0[3] * col3[0] + bias0;
    result[1] = row1[0] * col0[1] + row1[1] * col1[1] + 
                row1[2] * col2[1] + row1[3] * col3[1] + bias1;
    result[2] = row2[0] * col0[2] + row2[1] * col1[2] + 
                row2[2] * col2[2] + row2[3] * col3[2] + bias0;
    result[3] = row3[0] * col0[3] + row3[1] * col1[3] + 
                row3[2] * col2[3] + row3[3] * col3[3] + bias1;
    
    return result;
}
