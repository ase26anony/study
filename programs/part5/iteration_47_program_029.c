/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink = 0;

/* Function with multiple FP parameters to generate condition codes */
__attribute__((noinline))
static int compare_chain(float a, float b, float c, float d, float e, float f) {
    int result = 0;
    
    /* Mixed ordered/unordered comparisons */
    result += (a < b) ? (c != d) : (e >= f);
    result += (a > b) ? (c == d) : (e <= f);
    
    /* Unordered checks using standard macros */
    result += isunordered(a, b);
    result += islessgreater(c, d);
    
    /* Direct unordered comparisons (allowed with -ffast-math) */
    result += !(a == a);  /* Check for NaN */
    result += (b != b);   /* Another NaN check */
    
    return result;
}

/* Function to trigger UNEQ/LTGT condition codes */
__attribute__((noinline))
static int unordered_comparisons(double x, double y, double z, double w) {
    int sum = 0;
    
    /* These should generate UNEQ/LTGT with -ffast-math */
    sum += (x == y) ? 1 : 0;    /* May become UNEQ */
    sum += (x != y) ? 2 : 0;    /* May become LTGT */
    sum += (x < y) ? 4 : 0;
    sum += (x > y) ? 8 : 0;
    sum += (x <= y) ? 16 : 0;
    sum += (x >= y) ? 32 : 0;
    
    /* Complex conditional with mixed operators */
    sum += ((x < y) && (z > w)) || ((x == y) && (z != w)) ? 64 : 0;
    
    return sum;
}

/* Vector comparison function using GCC vector extensions */
__attribute__((noinline))
static int vector_comparisons(v4sf va, v4sf vb, v4sf vc, v4sf vd) {
    v4sf mask1 = va < vb;    /* Element-wise comparison */
    v4sf mask2 = vc > vd;
    v4sf mask3 = va == vb;
    v4sf mask4 = vc != vd;
    
    /* Combine masks */
    v4sf result = mask1 & mask2 | mask3 & mask4;
    
    /* Extract to integer */
    int sum = 0;
    float r[4];
    memcpy(r, &result, sizeof(r));
    for (int i = 0; i < 4; i++) {
        sum += (r[i] != 0.0f) ? (1 << i) : 0;
    }
    
    return sum;
}

/* AVX intrinsic version for specific condition codes */
#ifdef __AVX__
__attribute__((noinline))
static int avx_unordered_comparisons(__m128 a, __m128 b) {
    /* Use unordered comparison predicates */
    __m128 cmp_unord = _mm_cmp_ps(a, b, _CMP_UNORD_Q);  /* UNORD */
    __m128 cmp_ord = _mm_cmp_ps(a, b, _CMP_ORD_Q);      /* ORD */
    __m128 cmp_neq_uq = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);  /* UNEQ */
    __m128 cmp_nlt_uq = _mm_cmp_ps(a, b, _CMP_NLT_UQ);  /* UNGE */
    __m128 cmp_nle_uq = _mm_cmp_ps(a, b, _CMP_NLE_UQ);  /* UNGT */
    
    /* Convert to integer mask */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq_uq = _mm_movemask_ps(cmp_neq_uq);
    int mask_nlt_uq = _mm_movemask_ps(cmp_nlt_uq);
    int mask_nle_uq = _mm_movemask_ps(cmp_nle_uq);
    
    return mask_unord + mask_ord + mask_neq_uq + mask_nlt_uq + mask_nle_uq;
}
#endif

/* Function with switch-like behavior based on FP comparisons */
__attribute__((noinline))
static int fp_switch(float a, float b, float c, float d) {
    int result = 0;
    
    /* This complex expression should generate multiple condition codes */
    if ((a < b) ? (c != d) : (a == b)) {
        result |= 1;
    }
    
    if ((a > b) && (c <= d)) {
        result |= 2;
    }
    
    if ((a != b) || (c == d)) {
        result |= 4;
    }
    
    if (isunordered(a, b) || islessgreater(c, d)) {
        result |= 8;
    }
    
    /* Ternary with different comparison types */
    result += (a == b) ? ((c < d) ? 16 : 32) : ((c > d) ? 64 : 128);
    
    return result;
}

int main(int argc, char **argv) {
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normal numbers, zeros, and some NaNs */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 4 == 0) ? 0.0f : 
                   (i % 4 == 1) ? 1.0f / (i + 1) :
                   (i % 4 == 2) ? -2.0f * i :
                   __builtin_nanf("");  /* NaN */
        
        ddata[i] = (i % 3 == 0) ? 0.0 :
                   (i % 3 == 1) ? 1.0 / (i + 1) :
                   __builtin_nan("");   /* NaN */
    }
    
    int total_sum = 0;
    
    /* Use argc to prevent loop unrolling from collapsing comparisons */
    int iterations = (argc > 1) ? argc : 4;
    iterations = (iterations > 100) ? 100 : iterations;  /* Limit iterations */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float offset = iter * 0.1f;
        double doffset = iter * 0.01;
        
        /* Test 1: Chain of comparisons */
        for (int i = 0; i < 8; i += 2) {
            total_sum += compare_chain(
                fdata[i] + offset,
                fdata[i+1] - offset,
                fdata[i+2] * 1.1f,
                fdata[i+3] * 0.9f,
                fdata[i+4] + 0.5f,
                fdata[i+5] - 0.5f
            );
        }
        
        /* Test 2: Unordered comparisons */
        for (int i = 0; i < 8; i += 2) {
            total_sum += unordered_comparisons(
                ddata[i] + doffset,
                ddata[i+1] - doffset,
                ddata[i+2] * 1.01,
                ddata[i+3] * 0.99
            );
        }
        
        /* Test 3: Vector comparisons */
        v4sf va = {fdata[0], fdata[1], fdata[2], fdata[3]};
        v4sf vb = {fdata[4], fdata[5], fdata[6], fdata[7]};
        v4sf vc = {fdata[8], fdata[9], fdata[10], fdata[11]};
        v4sf vd = {fdata[12], fdata[13], fdata[14], fdata[15]};
        
        total_sum += vector_comparisons(va + offset, vb - offset, vc, vd);
        
        /* Test 4: AVX intrinsics if available */
        #ifdef __AVX__
        __m128 a = _mm_set_ps(fdata[3], fdata[2], fdata[1], fdata[0]);
        __m128 b = _mm_set_ps(fdata[7], fdata[6], fdata[5], fdata[4]);
        total_sum += avx_unordered_comparisons(a, b);
        #endif
        
        /* Test 5: FP switch-like behavior */
        for (int i = 0; i < 12; i += 3) {
            total_sum += fp_switch(
                fdata[i] + offset * 0.5f,
                fdata[i+1] - offset * 0.5f,
                fdata[i+2],
                fdata[(i+3) % 16]
            );
        }
        
        /* Prevent dead code elimination */
        sink = total_sum & 1;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
