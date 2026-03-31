/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Volatile store to force operations */
#define VOLATILE_STORE(var, val) \
    do { \
        volatile __typeof__(val) _tmp = (val); \
        (var) = _tmp; \
        COMPILER_BARRIER(); \
    } while(0)

/* Test function with many vector operations - marked noinline */
__attribute__((noinline, target("avx2,fma")))
v8si test_vector_operations(v8si a, v8si b, v8si c, v8si d, 
                           v8sf f1, v8sf f2, v8sf f3, v8sf f4,
                           v4df d1, v4df d2, v4df d3, v4df d4) {
    volatile v8si v1, v2, v3, v4, v5, v6, v7, v8;
    volatile v8sf fv1, fv2, fv3, fv4;
    volatile v4df dv1, dv2, dv3, dv4;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    VOLATILE_STORE(v1, shuffled);
    
    /* Vector conditional expression with comparison */
    v8si cmp_result = (a > b) ? (c * d) : (c + d);
    VOLATILE_STORE(v2, cmp_result);
    
    /* Chain of operations that may expand to many operands */
    v8si complex_op = a * b + c / (d + 1);
    VOLATILE_STORE(v3, complex_op);
    
    /* Float vector operations */
    v8sf fcmp = (f1 > f2) ? (f3 * f4) : (f3 / f4);
    VOLATILE_STORE(fv1, fcmp);
    
    /* Mixed float/int operations */
    v8si float_to_int = __builtin_convertvector(f1, v8si);
    VOLATILE_STORE(v4, float_to_int);
    
    /* Double vector operations with conditional */
    v4df dbl_cmp = (d1 > d2) ? (d3 * d4) : (d3 + d4);
    VOLATILE_STORE(dv1, dbl_cmp);
    
    /* Another complex shuffle with arithmetic */
    v8si shuffle_mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si shuffled2 = __builtin_shuffle(a * b, c + d, shuffle_mask2);
    VOLATILE_STORE(v5, shuffled2);
    
    /* Vector blend-like operation using conditional */
    v8si blend_mask = a > (b + 10);
    v8si blended = blend_mask ? (a * c) : (b * d);
    VOLATILE_STORE(v6, blended);
    
    /* Complex expression that may require temporary registers */
    v8si expr1 = (a + b) * (c - d);
    v8si expr2 = (a - b) * (c + d);
    v8si final_expr = (expr1 > expr2) ? expr1 : expr2;
    VOLATILE_STORE(v7, final_expr);
    
    /* Use AVX2-specific operations if available */
#ifdef __AVX2__
    /* Gather-like operation simulation */
    v8si indices = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si gathered = __builtin_shuffle(a, b, indices);
    VOLATILE_STORE(v8, gathered);
#endif
    
    /* Combine all results into a checksum */
    v8si result = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    
    /* Add float and double contributions */
    v8si f_as_int = __builtin_convertvector(fv1, v8si);
    v8si d_as_int = __builtin_convertvector((v8sf)dv1, v8si);
    
    result = result + f_as_int + d_as_int;
    
    return result;
}

/* Another test function specifically for 11 operands */
__attribute__((noinline, target("avx512f")))
v16si test_avx512_operations(v16si a, v16si b, v16si c, v16si d,
                            v16sf f1, v16sf f2, v16sf f3, v16sf f4,
                            v16si e, v16si f, v16si g) {
    volatile v16si v1, v2, v3, v4;
    volatile v16sf fv1, fv2;
    
    /* AVX-512 specific patterns that may use many operands */
    v16si mask = a > b;
    
    /* Complex conditional with mask */
    v16si cond_result = mask ? (c * d) : (e + f);
    VOLATILE_STORE(v1, cond_result);
    
    /* Shuffle with large vectors */
    v16si shuffle_idx = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
    v16si shuffled = __builtin_shuffle(a, b, shuffle_idx);
    VOLATILE_STORE(v2, shuffled);
    
    /* Float operations */
    v16sf f_result = (f1 > f2) ? (f3 * f4) : (f3 + f4);
    VOLATILE_STORE(fv1, f_result);
    
    /* Convert and combine */
    v16si from_float = __builtin_convertvector(fv1, v16si);
    v16si final = v1 + v2 + from_float + g;
    
    return final;
}

int main() {
    /* Initialize test vectors with pattern values */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    v8sf f1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf f2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf f3 = {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f};
    v8sf f4 = {1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f};
    
    v4df d1 = {1.0, 2.0, 3.0, 4.0};
    v4df d2 = {4.0, 3.0, 2.0, 1.0};
    v4df d3 = {2.0, 4.0, 6.0, 8.0};
    v4df d4 = {1.0, 3.0, 5.0, 7.0};
    
    /* Call test function */
    v8si result = test_vector_operations(a, b, c, d, f1, f2, f3, f4, d1, d2, d3, d4);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    v16si a16 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b16 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si c16 = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si d16 = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
    v16si e16 = {5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80};
    v16si f16 = {3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48};
    v16si g16 = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    v16sf f1_16 = {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,
                   9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f};
    v16sf f2_16 = {16.0f,15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,
                   8.0f,7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f};
    v16sf f3_16 = {2.0f,4.0f,6.0f,8.0f,10.0f,12.0f,14.0f,16.0f,
                   18.0f,20.0f,22.0f,24.0f,26.0f,28.0f,30.0f,32.0f};
    v16sf f4_16 = {1.0f,3.0f,5.0f,7.0f,9.0f,11.0f,13.0f,15.0f,
                   17.0f,19.0f,21.0f,23.0f,25.0f,27.0f,29.0f,31.0f};
    
    v16si result16 = test_avx512_operations(a16, b16, c16, d16, 
                                          f1_16, f2_16, f3_16, f4_16,
                                          e16, f16, g16);
    
    int checksum16 = 0;
    for (int i = 0; i < 16; i++) {
        checksum16 += result16[i];
    }
    printf("AVX-512 Checksum: %d\n", checksum16);
#endif
    
    return (checksum > 0) ? 0 : 1;
}
