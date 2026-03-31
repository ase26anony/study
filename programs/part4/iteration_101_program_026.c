/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Force no inlining to preserve expansion patterns */
__attribute__((noinline, target("avx2")))
v8si test_10_operand_expansion(v4si a, v4df b, v8si c, v8sf d, v4di e) {
    volatile v8si v1, v2, v3, v4;
    volatile v4df f1, f2, f3;
    volatile v8sf s1, s2;
    
    /* Complex shuffle with many operands - may expand to 10+ operands */
    v8si shuffled = __builtin_shufflevector(c, c, 7,6,5,4,3,2,1,0);
    v1 = shuffled;
    
    /* Compiler barrier to prevent optimization */
    asm volatile("" ::: "memory");
    
    /* Vector conditional expression with arithmetic - generates VEC_COND_EXPR */
    v4si mask = a > (v4si){1,2,3,4};
    v4si cond_result = mask ? (v4si)(a * (v4si){2,3,4,5}) : (v4si)(a + (v4si){10,20,30,40});
    
    /* Convert to v8si for mixing */
    v8si expanded_cond = {cond_result[0], cond_result[1], cond_result[2], cond_result[3],
                          cond_result[0], cond_result[1], cond_result[2], cond_result[3]};
    v2 = expanded_cond;
    
    /* Complex blend operation using conditional operator on vectors */
    v8si blend_mask = shuffled > (v8si){0};
    v8si blended = blend_mask ? shuffled : expanded_cond;
    v3 = blended;
    
    /* AVX2 specific operations that may require many operands */
    v8si multiplied = blended * (v8si){2,3,4,5,6,7,8,9};
    v8si added = multiplied + (v8si){1,1,1,1,1,1,1,1};
    
    /* Another shuffle with different pattern */
    v8si shuffled2 = __builtin_shufflevector(added, added, 0,2,4,6,1,3,5,7);
    v4 = shuffled2;
    
    /* Mix with float vector using conversion */
    v8si converted = __builtin_convertvector(d, v8si);
    v8si final_result = shuffled2 + converted;
    
    return final_result;
}

__attribute__((noinline, target("avx512f")))
v8si test_11_operand_expansion(v8si a, v8si b, v8si c, v8si d, v8si e, v8si f) {
    volatile v8si v1, v2, v3, v4, v5;
    
    /* Chain of operations that may require 11 operands when expanded */
    
    /* Step 1: Complex blend with mask computation */
    v8si mask1 = a > b;
    v8si blended1 = mask1 ? (a * c) : (b + d);
    v1 = blended1;
    
    asm volatile("" ::: "memory");
    
    /* Step 2: Another blend with different sources */
    v8si mask2 = c < d;
    v8si blended2 = mask2 ? (c * e) : (d - f);
    v2 = blended2;
    
    /* Step 3: Shuffle both results */
    v8si shuffle1 = __builtin_shufflevector(blended1, blended2, 
                                           0,8,1,9,2,10,3,11);
    v8si shuffle2 = __builtin_shufflevector(blended2, blended1,
                                           4,12,5,13,6,14,7,15);
    v3 = shuffle1;
    v4 = shuffle2;
    
    /* Step 4: Final combination with multiple operations */
    v8si temp = shuffle1 * shuffle2;
    v8si temp2 = temp + a;
    v8si temp3 = temp2 - b;
    v8si final = (temp3 > (v8si){0}) ? temp3 : (v8si){1,1,1,1,1,1,1,1};
    v5 = final;
    
    return final;
}

/* Helper to initialize vectors with patterns */
void init_vectors(v4si *a, v4df *b, v8si *c, v8sf *d, v4di *e) {
    *a = (v4si){1, 2, 3, 4};
    *b = (v4df){1.0, 2.0, 3.0, 4.0};
    *c = (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    *d = (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    *e = (v4di){1LL, 2LL, 3LL, 4LL};
}

int main() {
    v4si a;
    v4df b;
    v8si c, c2, c3, c4, c5, c6;
    v8sf d;
    v4di e;
    
    /* Initialize test vectors */
    init_vectors(&a, &b, &c, &d, &e);
    
    /* Initialize additional vectors for 11-operand test */
    c2 = (v8si){2,3,4,5,6,7,8,9};
    c3 = (v8si){3,4,5,6,7,8,9,10};
    c4 = (v8si){4,5,6,7,8,9,10,11};
    c5 = (v8si){5,6,7,8,9,10,11,12};
    c6 = (v8si){6,7,8,9,10,11,12,13};
    
    /* Test both expansion paths */
    v8si result1 = test_10_operand_expansion(a, b, c, d, e);
    v8si result2 = test_11_operand_expansion(c, c2, c3, c4, c5, c6);
    
    /* Compute checksums to prevent dead code elimination */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < 8; i++) {
        sum1 += result1[i];
        sum2 += result2[i];
    }
    
    printf("Checksum 1: %d\n", sum1);
    printf("Checksum 2: %d\n", sum2);
    
    /* Return based on checksum to ensure execution */
    return (sum1 + sum2) > 0 ? 0 : 1;
}
