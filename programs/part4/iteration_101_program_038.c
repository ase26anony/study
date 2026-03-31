/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* SSE/AVX types for intrinsics */
#ifdef __SSE2__
#include <emmintrin.h>
#include <immintrin.h>
#endif

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline test function to prevent inlining */
__attribute__((noinline, noipa))
v8si test_10_11_operands(v8si a, v8si b, v8si c, v8si d, 
                         v4df fa, v4df fb, v4df fc, v4df fd) {
    volatile v8si v1, v2, v3, v4;
    volatile v4df fv1, fv2, fv3, fv4;
    
    /* 1. Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    v1 = shuffled;
    BARRIER();
    
    /* 2. Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v8si cmp_result = a > b;
    v8si cond_result = cmp_result ? (a * b + c) : (b - a * d);
    v2 = cond_result;
    BARRIER();
    
    /* 3. Chain of operations that might expand to many operands */
    v8si temp1 = (a + b) * (c - d);
    v8si temp2 = (b + c) * (d - a);
    v8si temp3 = __builtin_shuffle(temp1, temp2, shuffle_mask);
    v8si chain_result = temp3 + shuffled - cond_result;
    v3 = chain_result;
    BARRIER();
    
    /* 4. Floating point vector conditional with conversion */
    v4df fcmp = fa > fb;
    v4df fcond = fcmp ? (fa * fb + fc) : (fb - fa * fd);
    fv1 = fcond;
    BARRIER();
    
    /* 5. Mixed-type operations that might require conversion */
    v8si int_from_float = __builtin_convertvector(fcond, v8si);
    v8si mixed_result = chain_result + int_from_float;
    v4 = mixed_result;
    BARRIER();
    
    /* 6. Another complex shuffle with arithmetic */
    v8si mask2 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si shuffle2 = __builtin_shuffle(mixed_result, cond_result, mask2);
    v8si final_shuffle = __builtin_shuffle(shuffle2, chain_result, shuffle_mask);
    
    /* 7. Final combination with barrier to prevent optimization */
    BARRIER();
    v8si result = final_shuffle + mixed_result - chain_result;
    
    /* Store to volatile to force all operations */
    volatile v8si final_store = result;
    
    return result;
}

/* Another test function focusing on floating point */
__attribute__((noinline, noipa))
v4df test_fp_10_11_operands(v4df a, v4df b, v4df c, v4df d,
                            v4df e, v4df f, v4df g, v4df h) {
    volatile v4df v1, v2, v3, v4;
    
    /* Complex floating point expression that might need many operands */
    v4df cmp1 = a > b;
    v4df cmp2 = c < d;
    v4df cmp3 = e != f;
    
    /* Nested conditional expressions */
    v4df temp1 = cmp1 ? (a * b + c) : (d - e * f);
    v4df temp2 = cmp2 ? (g * h - a) : (b + c * d);
    v4df temp3 = cmp3 ? (temp1 + temp2) : (temp1 - temp2);
    
    v1 = temp1;
    v2 = temp2;
    v3 = temp3;
    BARRIER();
    
    /* Shuffle with conversion */
    v4si int_vec = {0, 1, 2, 3};
    v4df shuffle_mask_df = __builtin_convertvector(int_vec, v4df);
    /* Note: __builtin_shuffle doesn't work directly on v4df with v4si mask
       in standard C, but GCC accepts it in some contexts */
    
    /* Alternative: use arithmetic combination */
    v4df result = temp1 * temp2 + temp3 / (a + 1.0) - b * c + d / e - f * g + h;
    v4 = result;
    
    return result;
}

/* Test with AVX-512 style 512-bit vectors if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noinline, noipa))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si d) {
    volatile v16si v1, v2;
    
    /* Large vector operations that might need many operands */
    v16si mask = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si shuffled = __builtin_shuffle(a, b, mask);
    
    /* Complex conditional on large vectors */
    v16si cmp = a > b;
    v16si cond_result = cmp ? (a * b + c) : (b - a * d);
    
    /* Chain operations */
    v16si temp1 = shuffled + cond_result;
    v16si temp2 = __builtin_shuffle(temp1, cond_result, mask);
    v16si result = temp1 * temp2 - shuffled + cond_result;
    
    v1 = result;
    BARRIER();
    
    return result;
}
#endif

int main() {
    /* Initialize integer vectors */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    /* Initialize floating point vectors */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {0.5, 1.5, 2.5, 3.5};
    v4df fd = {1.5, 2.5, 3.5, 4.5};
    
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Call test functions */
    v8si int_result = test_10_11_operands(a, b, c, d, fa, fb, fc, fd);
    v4df fp_result = test_fp_10_11_operands(fa, fb, fc, fd, 
                                           fa+1.0, fb+1.0, fc+1.0, fd+1.0);
    
    /* Compute checksums to prevent dead code elimination */
    int int_checksum = 0;
    for (int i = 0; i < 8; i++) {
        int_checksum += int_result[i];
    }
    
    double fp_checksum = 0.0;
    for (int i = 0; i < 4; i++) {
        fp_checksum += fp_result[i];
    }
    
    printf("Integer checksum: %d\n", int_checksum);
    printf("Floating point checksum: %f\n", fp_checksum);
    
    #ifdef __AVX512F__
    v16si avx512_a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si avx512_b = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si avx512_c = {2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};
    v16si avx512_d = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31};
    
    v16si avx512_result = test_avx512_many_operands(avx512_a, avx512_b, 
                                                   avx512_c, avx512_d);
    int avx512_checksum = 0;
    for (int i = 0; i < 16; i++) {
        avx512_checksum += avx512_result[i];
    }
    printf("AVX-512 checksum: %d\n", avx512_checksum);
    #endif
    
    /* Return based on checksums to ensure execution */
    return (int_checksum != 0 && fp_checksum != 0.0) ? 0 : 1;
}
