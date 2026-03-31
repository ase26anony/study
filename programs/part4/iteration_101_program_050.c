/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

/* AVX-512 types */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline test function to prevent inlining */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, 
                               v8si e, v8si f, v8si mask) {
    volatile v8si v1, v2, v3, v4, v5;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle1 = __builtin_shufflevector(a, b, 
        0, 8, 1, 9, 2, 10, 3, 11);
    KEEP(shuffle1);
    BARRIER();
    
    /* Another shuffle with different pattern */
    v8si shuffle2 = __builtin_shufflevector(c, d,
        4, 12, 5, 13, 6, 14, 7, 15);
    KEEP(shuffle2);
    BARRIER();
    
    /* Vector conditional expression with comparison */
    v8si cmp = (mask > (v8si){0,1,2,3,4,5,6,7}) ? shuffle1 : shuffle2;
    KEEP(cmp);
    BARRIER();
    
    /* Complex arithmetic chain */
    v8si t1 = a + b;
    v8si t2 = c * d;
    v8si t3 = e - f;
    v8si t4 = t1 & t2;
    v8si t5 = t3 | t4;
    
    KEEP(t1); KEEP(t2); KEEP(t3); KEEP(t4); KEEP(t5);
    BARRIER();
    
    /* Blend operation using conditional */
    v8si blend = (cmp > t5) ? t1 : t2;
    KEEP(blend);
    BARRIER();
    
    /* Final combination */
    v8si result = blend + cmp + t5;
    return result;
}

/* Another test for 11 operands */
__attribute__((noinline, target("avx512f")))
v16si test_11_operand_expansion(v16si a, v16si b, v16si c, v16si d,
                                v16si e, v16si f, v16si g, v16si h,
                                v16si mask) {
    volatile v16si v1, v2, v3;
    
    /* Very complex shuffle with multiple inputs */
    v16si shuffle1 = __builtin_shufflevector(a, b,
        0, 16, 1, 17, 2, 18, 3, 19,
        4, 20, 5, 21, 6, 22, 7, 23);
    KEEP(shuffle1);
    BARRIER();
    
    v16si shuffle2 = __builtin_shufflevector(c, d,
        8, 24, 9, 25, 10, 26, 11, 27,
        12, 28, 13, 29, 14, 30, 15, 31);
    KEEP(shuffle2);
    BARRIER();
    
    /* Multiple arithmetic operations */
    v16si t1 = a + b;
    v16si t2 = c * d;
    v16si t3 = e - f;
    v16si t4 = g & h;
    v16si t5 = t1 | t2;
    v16si t6 = t3 ^ t4;
    
    KEEP(t1); KEEP(t2); KEEP(t3); KEEP(t4); KEEP(t5); KEEP(t6);
    BARRIER();
    
    /* Nested conditional */
    v16si cmp1 = (mask > (v16si){0}) ? shuffle1 : shuffle2;
    v16si cmp2 = (t5 > t6) ? t1 : t2;
    v16si final_cmp = (cmp1 > cmp2) ? cmp1 : cmp2;
    
    KEEP(cmp1); KEEP(cmp2); KEEP(final_cmp);
    BARRIER();
    
    /* Complex blend with arithmetic */
    v16si blend1 = (final_cmp > t3) ? t4 : t5;
    v16si blend2 = (final_cmp > t6) ? blend1 : final_cmp;
    
    /* Final result with many operands */
    v16si result = blend2 + final_cmp + t1 + t2 + t3 + t4;
    return result;
}

/* SSE2 fallback test */
__attribute__((noinline, target("sse2")))
v4si test_sse2_many_operands(v4si a, v4si b, v4si c, v4si d,
                             v4si e, v4si f, v4si mask) {
    volatile v4si v1, v2, v3;
    
    /* Shuffle operations */
    v4si shuffle1 = __builtin_shufflevector(a, b, 0, 4, 1, 5);
    v4si shuffle2 = __builtin_shufflevector(c, d, 2, 6, 3, 7);
    
    /* Multiple conditionals */
    v4si cmp1 = (mask > (v4si){0,1,2,3}) ? shuffle1 : shuffle2;
    v4si cmp2 = (e > f) ? a : b;
    
    /* Arithmetic chain */
    v4si t1 = a + b;
    v4si t2 = c * d;
    v4si t3 = e - f;
    v4si t4 = t1 & t2;
    v4si t5 = t3 | cmp1;
    
    /* Final blend */
    v4si result = (cmp2 > t4) ? t5 : cmp1;
    result = result + t1 + t2 + t3;
    
    KEEP(shuffle1); KEEP(shuffle2); KEEP(cmp1); KEEP(cmp2);
    KEEP(t1); KEEP(t2); KEEP(t3); KEEP(t4); KEEP(t5);
    
    return result;
}

int main() {
    int checksum = 0;
    
    /* Initialize vectors with pattern */
    v4si sse_a = {1, 2, 3, 4};
    v4si sse_b = {5, 6, 7, 8};
    v4si sse_c = {9, 10, 11, 12};
    v4si sse_d = {13, 14, 15, 16};
    v4si sse_e = {17, 18, 19, 20};
    v4si sse_f = {21, 22, 23, 24};
    v4si sse_mask = {0, 1, 0, 1};
    
    /* Test SSE2 path */
    v4si sse_result = test_sse2_many_operands(sse_a, sse_b, sse_c, sse_d,
                                              sse_e, sse_f, sse_mask);
    
    /* Extract and sum elements */
    for (int i = 0; i < 4; i++) {
        checksum += sse_result[i];
    }
    
#ifdef __AVX2__
    /* Initialize AVX2 vectors */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si avx_e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si avx_f = {41, 42, 43, 44, 45, 46, 47, 48};
    v8si avx_mask = {0, 1, 0, 1, 0, 1, 0, 1};
    
    /* Test AVX2 path (10 operands) */
    v8si avx_result = test_10_operand_expansion(avx_a, avx_b, avx_c, avx_d,
                                                avx_e, avx_f, avx_mask);
    
    for (int i = 0; i < 8; i++) {
        checksum += avx_result[i];
    }
#endif
    
#ifdef __AVX512F__
    /* Initialize AVX-512 vectors */
    v16si avx512_a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si avx512_b = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    v16si avx512_c = {33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
    v16si avx512_d = {49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64};
    v16si avx512_e = {65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80};
    v16si avx512_f = {81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96};
    v16si avx512_g = {97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112};
    v16si avx512_h = {113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128};
    v16si avx512_mask = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
    
    /* Test AVX-512 path (11 operands) */
    v16si avx512_result = test_11_operand_expansion(
        avx512_a, avx512_b, avx512_c, avx512_d,
        avx512_e, avx512_f, avx512_g, avx512_h,
        avx512_mask);
    
    for (int i = 0; i < 16; i++) {
        checksum += avx512_result[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to affect return value */
    return (checksum > 0) ? 0 : 1;
}
