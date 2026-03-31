/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Prevent optimization of critical comparisons */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to force unordered comparisons */
__attribute__((noinline))
static int test_unordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate "unord" */
    if (isunordered(a, b)) sum |= 1;
    if (!isunordered(c, d)) sum |= 2;  /* Should generate "ord" */
    
    /* NaN checks that might generate unordered comparisons */
    if (a != a) sum |= 4;      /* Always true if a is NaN */
    if (!(c == c)) sum |= 8;   /* Always false if c is not NaN */
    
    return sum;
}

/* Function for UNEQ (unordered or equal) and LTGT (less or greater) */
__attribute__((noinline))
static int test_uneq_ltgt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Under -ffast-math, these may generate UNEQ/LTGT */
    if (a == b) sum |= 1;      /* May become UNEQ with fast-math */
    if (c != d) sum |= 2;      /* May become LTGT with fast-math */
    
    /* Mixed comparisons that could generate various codes */
    if ((a < b) && !isunordered(a, b)) sum |= 4;
    if ((c > d) || isunordered(c, d)) sum |= 8;
    
    return sum;
}

/* Function for UNGE/UNGT/UNLE/UNLT codes */
__attribute__((noinline))
static int test_unge_ungt(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These may generate the nlt/nle variants under fast-math */
    if (a >= b) sum |= 1;      /* May become UNGE -> "nlt" */
    if (c > d) sum |= 2;       /* May become UNGT -> "nle" */
    if (a <= b) sum |= 4;      /* May become UNLE -> "ule" */
    if (c < d) sum |= 8;       /* May become UNLT -> "ult" */
    
    /* Complex conditional to force multiple codes */
    sum += ((a < b) ? (c >= d) : (a != b)) ? 16 : 0;
    
    return sum;
}

/* Function using vector extensions for vector comparisons */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf vcmp1 = va < vb;      /* Element-wise less than */
    v4sf vcmp2 = va == vb;     /* Element-wise equal */
    v4sf vcmp3 = va >= vb;     /* Element-wise greater or equal */
    
    /* Extract results to prevent elimination */
    float f1 = vcmp1[0] + vcmp1[1] + vcmp1[2] + vcmp1[3];
    float f2 = vcmp2[0] + vcmp2[1] + vcmp2[2] + vcmp2[3];
    float f3 = vcmp3[0] + vcmp3[1] + vcmp3[2] + vcmp3[3];
    
    sum = (int)(f1 + f2 + f3);
    
    /* Double vector comparisons */
    v2df vcmp4 = vc != vd;
    v2df vcmp5 = vc > vd;
    
    sum += (int)(vcmp4[0] + vcmp4[1] + vcmp5[0] + vcmp5[1]);
    
    return sum;
}

/* Function using AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m128 va, __m128 vb, __m128d vc, __m128d vd) {
    int sum = 0;
    
    /* Generate specific comparison predicates */
    __m128 cmp_unord = _mm_cmpunord_ps(va, vb);    /* UNORD */
    __m128 cmp_neq_uq = _mm_cmpneq_ps(va, vb);     /* NEQ_UQ */
    __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);        /* NLT */
    __m128 cmp_nle = _mm_cmpnle_ps(va, vb);        /* NLE */
    
    /* Extract mask bits */
    int mask1 = _mm_movemask_ps(cmp_unord);
    int mask2 = _mm_movemask_ps(cmp_neq_uq);
    int mask3 = _mm_movemask_ps(cmp_nlt);
    int mask4 = _mm_movemask_ps(cmp_nle);
    
    sum = mask1 + mask2 + mask3 + mask4;
    
    /* Double precision comparisons */
    __m128d cmp_ord = _mm_cmpord_pd(vc, vd);       /* ORD */
    __m128d cmp_ueq = _mm_cmpueq_pd(vc, vd);       /* UEQ */
    
    int mask5 = _mm_movemask_pd(cmp_ord);
    int mask6 = _mm_movemask_pd(cmp_ueq);
    
    sum += mask5 + mask6;
    
    return sum;
}
#endif

/* Complex function with mixed comparison types */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain of comparisons that may generate multiple condition codes */
    if ((a < b) ? (c != d) : (e >= f)) {
        sum |= 1;
    }
    
    if ((isunordered(a, c) || (b > d)) && !(g == h)) {
        sum |= 2;
    }
    
    /* Nested ternary with floating comparisons */
    int val = (a == b) ? ((c < d) ? 4 : 8) : ((e != f) ? 16 : 32);
    sum += val;
    
    /* Loop with varying comparisons to prevent optimization */
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0: sum += (a <= b) ? 1 : 0; break;  /* May generate UNLE */
            case 1: sum += (c >= d) ? 2 : 0; break;  /* May generate UNGE */
            case 2: sum += (e != g) ? 4 : 0; break;  /* May generate LTGT */
            case 3: sum += islessgreater(f, h) ? 8 : 0; break;  /* Explicit LTGT */
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double ddata[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, INFINITY, -INFINITY,
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile reads to prevent constant propagation */
        vf1 = fdata[i % 12];
        vf2 = fdata[(i + 1) % 12];
        vd1 = ddata[i % 12];
        vd2 = ddata[(i + 2) % 12];
        
        /* Call test functions with mixed arguments */
        total_sum += test_unordered(vf1, vf2, vd1, vd2);
        total_sum += test_uneq_ltgt(vf1, vf2, fdata[(i + 3) % 12], fdata[(i + 4) % 12]);
        total_sum += test_unge_ungt(vf1, vf2, vd1, vd2);
        
        /* Vector tests */
        v4sf va = {vf1, vf2, fdata[(i + 5) % 12], fdata[(i + 6) % 12]};
        v4sf vb = {vf2, vf1, fdata[(i + 7) % 12], fdata[(i + 8) % 12]};
        v2df vc = {vd1, vd2};
        v2df vd = {vd2, vd1};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        #ifdef __AVX__
        __m128 mva = _mm_set_ps(fdata[(i+9)%12], fdata[(i+8)%12], 
                                fdata[(i+7)%12], fdata[(i+6)%12]);
        __m128 mvb = _mm_set_ps(fdata[(i+5)%12], fdata[(i+4)%12], 
                                fdata[(i+3)%12], fdata[(i+2)%12]);
        __m128d mvc = _mm_set_pd(ddata[(i+1)%12], ddata[i%12]);
        __m128d mvd = _mm_set_pd(ddata[(i+3)%12], ddata[(i+2)%12]);
        
        total_sum += test_avx_intrinsics(mva, mvb, mvc, mvd);
        #endif
        
        /* Mixed comparisons test */
        total_sum += test_mixed_comparisons(
            fdata[i % 12], fdata[(i + 1) % 12],
            fdata[(i + 2) % 12], fdata[(i + 3) % 12],
            ddata[i % 12], ddata[(i + 1) % 12],
            ddata[(i + 2) % 12], ddata[(i + 3) % 12]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
