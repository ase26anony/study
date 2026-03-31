/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __AVX__
#include <immintrin.h>
#endif

/* Define large vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Volatile store to prevent dead code elimination */
#define VOLATILE_STORE(var, val) do { \
    volatile __typeof__(var) _tmp = (val); \
    var = _tmp; \
    COMPILER_BARRIER(); \
} while(0)

/* Test function with many vector operations - marked noinline to prevent optimization */
__attribute__((noinline, target("sse2,avx,avx2")))
v8si test_function_10_operands(v8si a, v8si b, v8si c, v8si d, 
                               v4df e, v4df f, v4df g, v4df h) {
    volatile v8si v1, v2, v3, v4;
    volatile v4df vf1, vf2, vf3, vf4;
    
    /* Operation 1: Complex shuffle with many operands */
    v8si shuffle_temp;
    {
        /* Create a non-constant mask for shuffle */
        v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
        /* This shuffle may expand to multiple operations */
        shuffle_temp = __builtin_shuffle(a, b, mask);
        VOLATILE_STORE(v1, shuffle_temp);
    }
    
    /* Operation 2: Vector conditional expression with comparison */
    v8si cond_result;
    {
        v8si cmp_mask = (a > b);
        /* Complex conditional with arithmetic on both sides */
        cond_result = cmp_mask ? (a * b + c) : (d - a / (b + 1));
        VOLATILE_STORE(v2, cond_result);
    }
    
    /* Operation 3: Chain of operations that may require many temporaries */
    v8si chain_result;
    {
        v8si t1 = a + b;
        v8si t2 = c * d;
        v8si t3 = t1 - t2;
        v8si t4 = t3 << 2;
        chain_result = t4 | shuffle_temp;
        VOLATILE_STORE(v3, chain_result);
    }
    
    /* Operation 4: Mixed float/int operations */
    v8si mixed_result;
    {
        /* Convert float comparison to int mask */
        v4df cmp_double = (e > f) ? (g * h) : (h / (g + 1.0));
        VOLATILE_STORE(vf1, cmp_double);
        
        /* Use float result to influence int computation */
        v4si int_from_double = __builtin_convertvector(cmp_double, v4si);
        v8si extended = {int_from_double[0], int_from_double[1], 
                         int_from_double[2], int_from_double[3],
                         int_from_double[0], int_from_double[1],
                         int_from_double[2], int_from_double[3]};
        mixed_result = chain_result & extended;
        VOLATILE_STORE(v4, mixed_result);
    }
    
    /* Final combination of all results */
    v8si final_result = shuffle_temp + cond_result * chain_result - mixed_result;
    
    /* Additional complex operation that might need 11 operands */
    {
        v8si temp1 = final_result;
        v8si temp2 = a + b;
        v8si temp3 = c - d;
        v8si temp4 = temp1 * temp2;
        v8si temp5 = temp3 << 1;
        
        /* Complex expression that could expand to many operands */
        v8si complex_expr = (temp4 > temp5) ? 
                           (temp1 + temp2 * temp3) : 
                           (temp5 - temp4 / (temp2 + 1));
        
        /* Blend-like operation using conditional */
        v8si blend_mask = {0, -1, 0, -1, 0, -1, 0, -1};
        final_result = blend_mask ? complex_expr : final_result;
    }
    
    return final_result;
}

/* Another test function focusing on AVX-512 style 64-byte vectors */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
v16si test_function_11_operands(v16si a, v16si b, v16si c, 
                                v16sf fa, v16sf fb, v16sf fc) {
    volatile v16si v1, v2, v3;
    volatile v16sf vf1, vf2;
    
    /* Complex shuffle on 16-element vectors */
    v16si shuffle_mask = {15, 14, 13, 12, 11, 10, 9, 8,
                          7, 6, 5, 4, 3, 2, 1, 0};
    v16si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    VOLATILE_STORE(v1, shuffled);
    
    /* Vector conditional with float comparison affecting ints */
    v16sf cmp = (fa > fb);
    v16si int_mask = __builtin_convertvector(cmp, v16si);
    VOLATILE_STORE(v2, int_mask);
    
    /* Complex expression mixing multiple operations */
    v16si result = (int_mask & shuffled) | 
                   (c + (a * b) - (shuffled >> 2));
    
    /* Additional operation that might need many operands */
    v16si temp = result;
    for (int i = 0; i < 3; i++) {
        temp = (temp * a + b) >> 1;
        COMPILER_BARRIER();
    }
    
    result = result + temp;
    VOLATILE_STORE(v3, result);
    
    return result;
}
#endif

/* Main function that drives the tests */
int main() {
    /* Initialize test data */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    v4df e = {1.0, 2.0, 3.0, 4.0};
    v4df f = {4.0, 3.0, 2.0, 1.0};
    v4df g = {0.5, 1.5, 2.5, 3.5};
    v4df h = {3.5, 2.5, 1.5, 0.5};
    
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Call test function multiple times with different data */
    v8si result1 = test_function_10_operands(a, b, c, d, e, f, g, h);
    
    /* Modify inputs and call again */
    a = a + 1;
    b = b - 1;
    v8si result2 = test_function_10_operands(a, b, c, d, e, f, g, h);
    
    /* Combine results to prevent elimination */
    v8si final_result = result1 + result2;
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += final_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    #ifdef __AVX512F__
    printf("Testing AVX-512 patterns...\n");
    
    v16si va = {0};
    v16si vb = {0};
    v16si vc = {0};
    v16sf vfa = {0};
    v16sf vfb = {0};
    v16sf vfc = {0};
    
    /* Initialize with pattern */
    for (int i = 0; i < 16; i++) {
        va[i] = i;
        vb[i] = 15 - i;
        vc[i] = i * 2;
        vfa[i] = i * 0.5f;
        vfb[i] = (15 - i) * 0.5f;
        vfc[i] = i * 0.25f;
    }
    
    v16si vresult = test_function_11_operands(va, vb, vc, vfa, vfb, vfc);
    
    int vchecksum = 0;
    for (int i = 0; i < 16; i++) {
        vchecksum += vresult[i];
    }
    printf("AVX-512 checksum: %d\n", vchecksum);
    #endif
    
    /* Return based on checksum to ensure execution */
    return (checksum > 0) ? 0 : 1;
}
