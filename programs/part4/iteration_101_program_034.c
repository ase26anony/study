/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Force no inlining to preserve complex operations */
__attribute__((noinline, target("avx512f,avx512vl")))
v16si test_10_11_operands(v4si a, v4si b, v4si c, v4si d,
                          v4df e, v4df f, v4df g, v4df h) {
    /* Volatile variables to prevent optimization */
    volatile v8si v8_temp;
    volatile v16si v16_temp;
    volatile v4df v4df_temp;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* 1. Complex shuffle with many operands - may expand to 10+ operands */
    v8si v8a = __builtin_shufflevector(a, b, 0, 1, 2, 3, 4, 5, 6, 7);
    v8si v8b = __builtin_shufflevector(c, d, 3, 2, 1, 0, 7, 6, 5, 4);
    
    /* 2. Vector conditional with comparison - generates VEC_COND_EXPR */
    v8si mask = v8a > v8b;
    v8si v8c = mask ? v8a * v8b + v8a : v8b - v8a * 2;
    
    /* Store to volatile to force memory ops */
    v8_temp = v8c;
    
    /* 3. Convert between vector types - may need many temp registers */
    v4df converted = __builtin_convertvector((v4si){v8c[0], v8c[1], v8c[2], v8c[3]}, v4df);
    
    /* 4. Complex FP vector expression with blending */
    v4df blend_mask = e > f;
    v4df v4d = blend_mask ? e * f + g : h / (e + 1.0);
    v4df v4e = __builtin_shufflevector(v4d, converted, 0, 2, 1, 3);
    
    /* 5. Chain operations to increase operand count */
    v4df v4f = v4e * 2.5 - v4d;
    v4df_temp = v4f;
    
    /* 6. AVX-512 style masking operation (if supported) */
#ifdef __AVX512F__
    v16si v16a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si v16b = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    
    /* Complex masked operation - may require many operands */
    v16si v16c = v16a + v16b;
    v16si v16d = v16c * 2 - v16a;
    
    /* Shuffle with large mask - potentially 10+ operands */
    v16si v16e = __builtin_shufflevector(v16d, v16c, 
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    v16_temp = v16e;
    
    /* Combine all results */
    v16si result = {0};
    for (int i = 0; i < 8; i++) {
        result[i] = v8c[i];
        result[i+8] = (int)v4f[i % 4] + v16e[i+8];
    }
#else
    /* Fallback for non-AVX512 */
    v16si result = {0};
    for (int i = 0; i < 8; i++) {
        result[i] = v8c[i];
        result[i+8] = (int)v4f[i % 4] + i;
    }
#endif
    
    /* Final compiler barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Another test function focusing on 11 operands */
__attribute__((noinline, target("avx2")))
v8df test_more_operands(v4df a, v4df b, v4df c, v4df d,
                        v4si mask_vec) {
    volatile v8df v8_temp;
    
    /* Complex chain of operations */
    v4df t1 = a + b;
    v4df t2 = c - d;
    v4df t3 = t1 * t2;
    v4df t4 = __builtin_shufflevector(t3, a, 2, 3, 0, 1);
    
    /* Vector conditional with multiple operations */
    v4df cmp = a > b;
    v4df t5 = cmp ? t1 / (t2 + 1.0) : t3 * t4;
    
    /* Convert and expand */
    v8df expanded = {t5[0], t5[1], t5[2], t5[3],
                     t5[0]*2, t5[1]*2, t5[2]*2, t5[3]*2};
    
    /* Another shuffle with complex pattern */
    v8df shuffled = __builtin_shufflevector(expanded, expanded,
        7, 6, 5, 4, 3, 2, 1, 0);
    
    /* Blend based on integer mask */
    v8df alt = expanded * 0.5;
    v8df result = shuffled;
    
    /* Manual blending - each element conditional */
    for (int i = 0; i < 8; i++) {
        if (mask_vec[i % 4] & (1 << (i % 8))) {
            result[i] = alt[i];
        }
    }
    
    v8_temp = result;
    return result;
}

int main() {
    /* Initialize test vectors with patterns */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    
    v4df e = {1.1, 2.2, 3.3, 4.4};
    v4df f = {5.5, 6.6, 7.7, 8.8};
    v4df g = {9.9, 10.1, 11.11, 12.12};
    v4df h = {13.13, 14.14, 15.15, 16.16};
    
    v4si mask_vec = {0xAA, 0x55, 0xF0, 0x0F};
    
    /* Call test functions */
    v16si result1 = test_10_11_operands(a, b, c, d, e, f, g, h);
    v8df result2 = test_more_operands(e, f, g, h, mask_vec);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    for (int i = 0; i < 16; i++) {
        checksum1 += result1[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum2 += result2[i];
    }
    
    /* Use results to affect program output */
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %f\n", checksum2);
    
    /* Return based on checksums to ensure execution */
    return (checksum1 > 100 && checksum2 > 50.0) ? 0 : 1;
}
