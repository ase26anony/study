/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
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

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Complex shuffle with runtime mask */
static v4si NOINLINE shuffle_complex(v4si a, v4si b, v4si mask) {
    /* This may expand to multiple operations requiring many operands */
    v4si t1 = __builtin_shuffle(a, b, mask);
    v4si t2 = __builtin_shuffle(b, a, mask + (v4si){1,1,1,1});
    return t1 + t2;
}

/* Vector conditional with complex expressions */
static v4df NOINLINE vec_cond_complex(v4df a, v4df b, v4df c, v4df d, v4df mask) {
    /* Complex conditional that may require many operands */
    v4df t1 = a * b;
    v4df t2 = c / (d + (v4df){1.0, 1.0, 1.0, 1.0});
    v4df cmp = mask > (v4df){0.5, 0.5, 0.5, 0.5};
    
    /* VEC_COND_EXPR with complex true/false values */
    v4df result = cmp ? t1 + t2 : t1 - t2;
    
    /* Additional operations to increase operand count */
    v4df t3 = __builtin_convertvector(__builtin_ia32_cvtpd2ps256(result), v4df);
    return result * t3;
}

/* Chain of AVX operations */
static v8si NOINLINE avx_chain(v8si a, v8si b, v8si c, v8si d) {
    /* Multiple operations that may require many operands */
    v8si t1 = a + b;
    v8si t2 = c * d;
    v8si t3 = t1 - t2;
    
    /* Blend operation */
    v8si mask = (a > b);
    v8si t4 = __builtin_ia32_pblendd256(t2, t3, 0xF0);
    
    /* Shuffle elements */
    v8si t5 = __builtin_shuffle(t4, t3, (v8si){0,7,1,6,2,5,3,4});
    
    /* Conditional select */
    v8si result = mask ? t5 : (t1 + t4);
    
    /* Force memory operations */
    volatile v8si mem1 = t1;
    volatile v8si mem2 = t2;
    BARRIER();
    
    return result + mem1 + mem2;
}

/* Main test function with many operand patterns */
static v4df NOINLINE test_many_operands(v4si vi1, v4si vi2, v4df vf1, v4df vf2, 
                                        v4df vf3, v4df vf4, v8si avxi, v8si avxj) {
    v4df result = {0};
    
    /* Test 1: Complex shuffle (may need 10+ operands) */
    v4si mask = {3,2,1,0};
    v4si shuffled = shuffle_complex(vi1, vi2, mask);
    KEEP(shuffled);
    
    /* Convert to double for accumulation */
    v4df d_shuffled = __builtin_convertvector(shuffled, v4df);
    result = result + d_shuffled;
    BARRIER();
    
    /* Test 2: Vector conditional with many operands */
    v4df cond_mask = {0.7, 0.3, 0.9, 0.1};
    v4df cond_result = vec_cond_complex(vf1, vf2, vf3, vf4, cond_mask);
    KEEP(cond_result);
    
    result = result * cond_result;
    BARRIER();
    
    /* Test 3: AVX chain operations */
    v8si avx_result = avx_chain(avxi, avxj, avxi + (v8si){1}, avxj - (v8si){1});
    
    /* Convert AVX result to double vector (uses 4 elements) */
    v4si avx_low = __builtin_convertvector(avx_result, v4si);
    v4df d_avx = __builtin_convertvector(avx_low, v4df);
    
    result = result + d_avx;
    BARRIER();
    
    /* Test 4: Builtin with explicit rounding mode */
    v4df rounded = __builtin_ia32_roundpd256(result, 8); /* _MM_FROUND_CUR_DIRECTION */
    KEEP(rounded);
    
    /* Blend operation with many operands */
    v4df blend_mask = result > rounded;
    v4df final = blend_mask ? result : rounded;
    
    /* Additional operation to ensure expansion */
    final = final + __builtin_ia32_sqrtpd256(result);
    
    return final;
}

int main(void) {
    /* Initialize vectors with pattern values */
    v4si vi1 = {1, 2, 3, 4};
    v4si vi2 = {5, 6, 7, 8};
    
    v4df vf1 = {1.0, 2.0, 3.0, 4.0};
    v4df vf2 = {5.0, 6.0, 7.0, 8.0};
    v4df vf3 = {9.0, 10.0, 11.0, 12.0};
    v4df vf4 = {13.0, 14.0, 15.0, 16.0};
    
    v8si avxi = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avxj = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Call test function */
    v4df result = test_many_operands(vi1, vi2, vf1, vf2, vf3, vf4, avxi, avxj);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < 4; i++) {
        checksum += result[i];
    }
    
    /* Use result to affect program output */
    printf("Result checksum: %f\n", checksum);
    
    /* Return based on checksum (prevents optimization) */
    if (checksum > 1000.0) {
        return 1; /* Unlikely path */
    }
    
    return 0;
}
