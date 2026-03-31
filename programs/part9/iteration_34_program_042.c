/* test_optabs_10_operand.c
 * Target: Trigger case 10: in optabs.cc (10-operand expansion)
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -c test_optabs_10_operand.c -fdump-rtl-expand
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex shuffle operation requiring many operands */
static v8si complex_shuffle_10_operand(v8si a, v8si b, v8si c, v8si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate control values = 6 operands
     * Total: 10 operands
     */
    
    /* Create a complex shuffle using multiple builtins with immediate controls */
    v8si result;
    
    /* First, shuffle pairs with immediate controls */
    v8si ab_shuf = __builtin_ia32_pshufd256(a, 0x1B);  /* Control: 0x1B = 27 */
    v8si cd_shuf = __builtin_ia32_pshufd256(c, 0xE4);  /* Control: 0xE4 = 228 */
    
    /* Blend with immediate controls */
    v8si blend1 = __builtin_ia32_pblendd256(ab_shuf, b, 0xAA);  /* Control: 0xAA = 170 */
    v8si blend2 = __builtin_ia32_pblendd256(cd_shuf, d, 0x55);  /* Control: 0x55 = 85 */
    
    /* Final permute with immediate control */
    result = __builtin_ia32_permdi256(blend1, blend2, 0x31);  /* Control: 0x31 = 49 */
    
    return result;
}

/* Vector FMA-like operation with many constants */
static v8sf complex_fma_10_operand(v8sf x, v8sf y, v8sf z) {
    /* FMA with multiple constant coefficients:
     * x, y, z = 3 operands
     * 7 constant coefficients = 7 operands
     * Total: 10 operands
     */
    
    /* Create constant vectors (each is an operand) */
    v8sf c1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf c2 = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
    v8sf c3 = {0.25f, 0.75f, 1.25f, 1.75f, 2.25f, 2.75f, 3.25f, 3.75f};
    v8sf c4 = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    v8sf c5 = {100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f};
    v8sf c6 = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    v8sf c7 = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f, 0.06f, 0.07f, 0.08f};
    
    /* Complex expression that might expand to 10-operand operation */
    v8sf result = x * c1 + y * c2 + z * c3 + c4 * c5 + c6 * c7;
    
    /* Additional shuffle to ensure complex expansion */
    result = __builtin_shuffle(result, result, 
        (v8si){0, 7, 1, 6, 2, 5, 3, 4});  /* 8 immediate values */
    
    return result;
}

/* Table lookup-like operation - often requires many operands */
static v8si table_lookup_10_operand(v8si table, v8si indices, v8si mask) {
    /* Table lookup with multiple control values */
    v8si result;
    
    /* Complex bit manipulation with many constants */
    result = (table & 0xFF00FF00) | 
             ((indices << 8) & 0x00FF00FF) |
             ((mask >> 4) & 0x0000FFFF) |
             ((table >> 16) & 0xFFFF0000);
    
    /* Multiple immediate permutes */
    result = __builtin_ia32_pshufd256(result, 0x1B);  /* Control: 27 */
    result = __builtin_ia32_pshufd256(result, 0x4E);  /* Control: 78 */
    result = __builtin_ia32_pshufd256(result, 0x93);  /* Control: 147 */
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 3;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v8si vec1 = {argc, argc+1, argc+2, argc+3, argc+4, argc+5, argc+6, argc+7};
    v8si vec2 = {argc*2, argc*3, argc*4, argc*5, argc*6, argc*7, argc*8, argc*9};
    v8si vec3 = {argc*10, argc*11, argc*12, argc*13, argc*14, argc*15, argc*16, argc*17};
    v8si vec4 = {argc*20, argc*21, argc*22, argc*23, argc*24, argc*25, argc*26, argc*27};
    
    v8sf fvec1 = {(float)argc, (float)(argc+1), (float)(argc+2), (float)(argc+3),
                  (float)(argc+4), (float)(argc+5), (float)(argc+6), (float)(argc+7)};
    v8sf fvec2 = {(float)(argc*2), (float)(argc*3), (float)(argc*4), (float)(argc*5),
                  (float)(argc*6), (float)(argc*7), (float)(argc*8), (float)(argc*9)};
    v8sf fvec3 = {(float)(argc*10), (float)(argc*11), (float)(argc*12), (float)(argc*13),
                  (float)(argc*14), (float)(argc*15), (float)(argc*16), (float)(argc*17)};
    
    v8si result_int = {0};
    v8sf result_float = {0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that should trigger 10-operand expansions */
        result_int = complex_shuffle_10_operand(vec1, vec2, vec3, vec4);
        result_float = complex_fma_10_operand(fvec1, fvec2, fvec3);
        
        /* Mix results to create data dependencies */
        vec1 = __builtin_ia32_paddd256(vec1, result_int);
        fvec1 = result_float + fvec1;
    }
    
    /* Use results to prevent optimization */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    /* Extract and sum elements */
    for (int i = 0; i < 8; i++) {
        sum_int += result_int[i];
        sum_float += result_float[i];
    }
    
    printf("Result sums: int=%d, float=%.2f\n", sum_int, sum_float);
    
    return sum_int > 0 ? 0 : 1;
}

/* Additional test targeting specific 10-operand patterns */
void test_10_operand_patterns(void) {
    /* This function specifically tries to create patterns that might
     * expand to operations with exactly 10 operands */
    
    v4di v1 = {1, 2, 3, 4};
    v4di v2 = {5, 6, 7, 8};
    v4di v3 = {9, 10, 11, 12};
    v4di v4 = {13, 14, 15, 16};
    
    /* Complex permute with many control values */
    v4di perm_result = __builtin_ia32_permdi256(v1, v2, 0x12);
    perm_result = __builtin_ia32_permdi256(perm_result, v3, 0x34);
    perm_result = __builtin_ia32_permdi256(perm_result, v4, 0x56);
    
    /* Vector blend with multiple sources and control */
    v4df d1 = {1.0, 2.0, 3.0, 4.0};
    v4df d2 = {5.0, 6.0, 7.0, 8.0};
    v4df d3 = {9.0, 10.0, 11.0, 12.0};
    v4df d4 = {13.0, 14.0, 15.0, 16.0};
    
    /* Complex blend chain - might expand to multi-operand operation */
    v4df blend_temp = __builtin_ia32_blendpd256(d1, d2, 0x3);
    blend_temp = __builtin_ia32_blendpd256(blend_temp, d3, 0x5);
    v4df final_blend = __builtin_ia32_blendpd256(blend_temp, d4, 0xA);
    
    /* Use volatile to prevent optimization */
    volatile v4di v_vol = perm_result;
    volatile v4df d_vol = final_blend;
    (void)v_vol;
    (void)d_vol;
}
