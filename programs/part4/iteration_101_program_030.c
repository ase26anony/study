/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define KEEP(expr) do { asm volatile("" : : "r"(expr) : "memory"); } while(0)

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* AVX types for more operands */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* AVX-512 types for maximum operand count */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Noinline to prevent inlining and optimization */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v8si a, v8si b, v8si c, v8si d, 
                               v8si e, v8si f, v8si g, v8si h) {
    volatile v8si temp1, temp2, temp3;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    temp1 = shuffled;
    KEEP(temp1);
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v8si cmp_result = (a > b) ? (c * d + e) : (f - g * h);
    temp2 = cmp_result;
    KEEP(temp2);
    
    /* Chain of operations that might expand to multi-operand insn */
    v8si complex_op = (a + b) * (c - d) + (e & f) | (g ^ h);
    
    /* Blend operation using conditional */
    v8si blend_mask = (shuffled > complex_op);
    v8si blended = blend_mask ? cmp_result : complex_op;
    
    /* Another shuffle with dynamic mask */
    v8si mask2 = a + b + c;
    v8si final_shuffle = __builtin_shuffle(blended, cmp_result, mask2);
    
    /* Force memory operations */
    temp3 = final_shuffle;
    KEEP(temp3);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    return final_shuffle + temp1 + temp2;
}

__attribute__((noinline, target("avx512f")))
v16si test_11_operand_expansion(v16si a, v16si b, v16si c, v16si d,
                                v16si e, v16si f, v16si g, v16si h,
                                v16si i, v16si j) {
    volatile v16si temp[4];
    
    /* AVX-512 specific operations that may need many operands */
    v16si mask1 = a > b;
    v16si mask2 = c < d;
    
    /* Complex conditional blend */
    v16si blended1 = mask1 ? (e + f) : (g - h);
    temp[0] = blended1;
    KEEP(temp[0]);
    
    /* Another blend with different condition */
    v16si blended2 = mask2 ? (i * j) : (a & b);
    temp[1] = blended2;
    KEEP(temp[1]);
    
    /* Shuffle with computed mask */
    v16si shuffle_mask = a + b + c + d;
    v16si shuffled = __builtin_shuffle(blended1, blended2, shuffle_mask);
    temp[2] = shuffled;
    KEEP(temp[2]);
    
    /* Arithmetic chain - each operation might need many operands */
    v16si arith_chain = ((a * b) + (c * d) - (e * f)) / (g + h + i + j);
    temp[3] = arith_chain;
    KEEP(temp[3]);
    
    /* Combine with conditional */
    v16si final = (shuffled > arith_chain) ? 
                  (blended1 + blended2) : 
                  (blended1 - blended2);
    
    /* Multiple barriers to prevent optimization */
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    
    return final + temp[0] + temp[1] + temp[2] + temp[3];
}

/* Helper to initialize vectors */
void init_vector(v8si *v, int start) {
    for (int i = 0; i < 8; i++) {
        (*v)[i] = start + i;
    }
}

void init_vector_16(v16si *v, int start) {
    for (int i = 0; i < 16; i++) {
        (*v)[i] = start + i;
    }
}

int main() {
    /* Initialize test vectors */
    v8si a8, b8, c8, d8, e8, f8, g8, h8;
    v16si a16, b16, c16, d16, e16, f16, g16, h16, i16, j16;
    
    init_vector(&a8, 1);
    init_vector(&b8, 10);
    init_vector(&c8, 20);
    init_vector(&d8, 30);
    init_vector(&e8, 40);
    init_vector(&f8, 50);
    init_vector(&g8, 60);
    init_vector(&h8, 70);
    
    init_vector_16(&a16, 1);
    init_vector_16(&b16, 20);
    init_vector_16(&c16, 40);
    init_vector_16(&d16, 60);
    init_vector_16(&e16, 80);
    init_vector_16(&f16, 100);
    init_vector_16(&g16, 120);
    init_vector_16(&h16, 140);
    init_vector_16(&i16, 160);
    init_vector_16(&j16, 180);
    
    /* Call test functions */
    v8si result8 = test_10_operand_expansion(a8, b8, c8, d8, e8, f8, g8, h8);
    v16si result16 = test_11_operand_expansion(a16, b16, c16, d16, e16, f16, 
                                               g16, h16, i16, j16);
    
    /* Compute checksums to prevent dead code elimination */
    int sum8 = 0;
    for (int i = 0; i < 8; i++) {
        sum8 += result8[i];
    }
    
    int sum16 = 0;
    for (int i = 0; i < 16; i++) {
        sum16 += result16[i];
    }
    
    /* Use results to affect program output */
    if ((sum8 + sum16) != 0) {
        printf("Test completed: sum8 = %d, sum16 = %d\n", sum8, sum16);
        return 0;
    } else {
        printf("Unexpected result\n");
        return 1;
    }
}
