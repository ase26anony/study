/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline,noipa))
v4si test_10_operand_expansion(v4si a, v4si b, v4si c, v4si d, 
                               v4si e, v4si f, v4si g, v4si h) {
    /* Complex shuffle with many operands */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si temp1, temp2, temp3, temp4;
    
    /* Use volatile to prevent optimization */
    volatile v4si vtemp;
    
    /* Multiple shuffle operations that may require many operands */
    temp1 = __builtin_shuffle(a, b, shuffle_mask);
    temp2 = __builtin_shuffle(c, d, shuffle_mask);
    temp3 = __builtin_shuffle(e, f, shuffle_mask);
    temp4 = __builtin_shuffle(g, h, shuffle_mask);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Complex conditional expression with vector comparison */
    v4si cmp_mask = temp1 > temp2;
    v4si result1 = cmp_mask ? (temp1 * temp2 + temp3) : (temp4 - temp1);
    
    /* Store to volatile to force operation */
    vtemp = result1;
    
    /* More operations with different vector types */
    v2di di1 = {1, 2};
    v2di di2 = {3, 4};
    v2di di3 = {5, 6};
    v2di di4 = {7, 8};
    
    /* Chain operations that may expand to many operands */
    v2di di_result = di1 + di2 * di3 - di4;
    
    /* Convert between vector types - may require many operands */
    v4sf sf1 = __builtin_convertvector(temp1, v4sf);
    v4sf sf2 = __builtin_convertvector(temp2, v4sf);
    v4sf sf3 = __builtin_convertvector(temp3, v4sf);
    v4sf sf4 = __builtin_convertvector(temp4, v4sf);
    
    /* Complex floating point expression */
    v4sf sf_result = sf1 * sf2 + sf3 / sf4;
    
    /* Convert back */
    v4si final_result = __builtin_convertvector(sf_result, v4si);
    
    /* Combine all results */
    return final_result + result1 + __builtin_convertvector(di_result, v4si);
}

__attribute__((noinline,noipa))
v8si test_11_operand_expansion(v8si a, v8si b, v8si c, v8si d,
                               v8si e, v8si f, v8si g, v8si h,
                               v8si i, v8si j) {
    /* Create a complex mask for 8-element shuffle */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    
    /* Multiple shuffle operations on large vectors */
    v8si temp1 = __builtin_shuffle(a, b, shuffle_mask);
    v8si temp2 = __builtin_shuffle(c, d, shuffle_mask);
    v8si temp3 = __builtin_shuffle(e, f, shuffle_mask);
    v8si temp4 = __builtin_shuffle(g, h, shuffle_mask);
    v8si temp5 = __builtin_shuffle(i, j, shuffle_mask);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Complex arithmetic chain - each operation may need many operands */
    v8si result1 = temp1 * temp2 + temp3 - temp4 / (temp5 + 1);
    
    /* Vector comparison and conditional */
    v8si cmp_mask = result1 > temp1;
    v8si result2 = cmp_mask ? (temp2 * temp3) : (temp4 + temp5);
    
    /* Blend operation simulated with conditional */
    v8si blend_mask = {0, -1, 0, -1, 0, -1, 0, -1}; /* Alternating mask */
    v8si blended = blend_mask ? result1 : result2;
    
    /* Convert to float and back for more complexity */
    v8sf sf1 = __builtin_convertvector(blended, v8sf);
    v8sf sf2 = __builtin_convertvector(temp1, v8sf);
    v8sf sf_result = sf1 * sf2 + sf1 / (sf2 + 1.0f);
    
    /* Final conversion back */
    return __builtin_convertvector(sf_result, v8si);
}

/* Test with AVX-512 if available */
#ifdef __AVX512F__
__attribute__((noinline,noipa))
v16si test_avx512_many_operands(v16si a, v16si b, v16si c, v16si d,
                                v16si e, v16si f, v16si g, v16si h) {
    /* 16-element shuffle mask */
    v16si mask = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    
    /* Multiple operations that may require many operands */
    v16si temp1 = __builtin_shuffle(a, b, mask);
    v16si temp2 = __builtin_shuffle(c, d, mask);
    v16si temp3 = __builtin_shuffle(e, f, mask);
    v16si temp4 = __builtin_shuffle(g, h, mask);
    
    /* Complex expression tree */
    v16si result = (temp1 > temp2) ? (temp3 * temp4) : (temp1 + temp2);
    
    /* Convert to float for more operations */
    v16sf sf1 = __builtin_convertvector(result, v16sf);
    v16sf sf2 = __builtin_convertvector(temp1, v16sf);
    v16sf sf_result = sf1 * sf2 - sf1 / sf2;
    
    return __builtin_convertvector(sf_result, v16si);
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors with pattern values */
    v4si v4a = {1, 2, 3, 4};
    v4si v4b = {5, 6, 7, 8};
    v4si v4c = {9, 10, 11, 12};
    v4si v4d = {13, 14, 15, 16};
    v4si v4e = {17, 18, 19, 20};
    v4si v4f = {21, 22, 23, 24};
    v4si v4g = {25, 26, 27, 28};
    v4si v4h = {29, 30, 31, 32};
    
    /* Test 10-operand expansion path */
    v4si result4 = test_10_operand_expansion(v4a, v4b, v4c, v4d, v4e, v4f, v4g, v4h);
    
    /* Initialize 8-element vectors */
    v8si v8a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v8b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si v8c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si v8d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si v8e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si v8f = {41, 42, 43, 44, 45, 46, 47, 48};
    v8si v8g = {49, 50, 51, 52, 53, 54, 55, 56};
    v8si v8h = {57, 58, 59, 60, 61, 62, 63, 64};
    v8si v8i = {65, 66, 67, 68, 69, 70, 71, 72};
    v8si v8j = {73, 74, 75, 76, 77, 78, 79, 80};
    
    /* Test 11-operand expansion path */
    v8si result8 = test_11_operand_expansion(v8a, v8b, v8c, v8d, v8e, 
                                            v8f, v8g, v8h, v8i, v8j);
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    v16si v16a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si v16b = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    v16si v16c = {33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
    v16si v16d = {49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64};
    v16si v16e = {65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80};
    v16si v16f = {81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96};
    v16si v16g = {97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112};
    v16si v16h = {113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128};
    
    v16si result16 = test_avx512_many_operands(v16a, v16b, v16c, v16d,
                                              v16e, v16f, v16g, v16h);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    
    /* Extract elements from 4-element result */
    int *r4 = (int*)&result4;
    for (int i = 0; i < 4; i++) {
        checksum += r4[i];
    }
    
    /* Extract elements from 8-element result */
    int *r8 = (int*)&result8;
    for (int i = 0; i < 8; i++) {
        checksum += r8[i];
    }
    
#ifdef __AVX512F__
    /* Extract elements from 16-element result */
    int *r16 = (int*)&result16;
    for (int i = 0; i < 16; i++) {
        checksum += r16[i];
    }
#endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum != 0) ? 0 : 1;
}
