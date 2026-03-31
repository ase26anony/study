/* Test program to trigger x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent excessive optimization */
static volatile int vcounter = 0;

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with noinline to preserve RTL patterns */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate "unord" */
    if (isunordered(a, b)) sum |= 1;
    if (!isunordered(c, d)) sum |= 2;  /* Should generate "ord" */
    
    /* NaN checks using self-comparison */
    if (a != a) sum |= 4;      /* true if a is NaN */
    if (!(c == c)) sum |= 8;   /* true if c is NaN */
    
    /* Mixed ordered/unordered comparisons */
    if (isunordered(a, b) || (c < d)) sum |= 16;
    
    return sum;
}

__attribute__((noinline))
static int test_uneq_unge(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These should generate "ueq" and "nlt" (unge) under fast-math */
    if (!(a < b) && !(a > b)) sum |= 1;    /* a == b or unordered -> ueq */
    if (!(c < d)) sum |= 2;                /* c >= d or unordered -> nlt */
    
    /* Complex expression that might generate multiple condition codes */
    if ((a == b) ? (c >= d) : (a != a)) sum |= 4;
    
    return sum;
}

__attribute__((noinline))
static int test_ungt_unle(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Should generate "nle" (ungt) and "ule" */
    if (!(a <= b)) sum |= 1;               /* a > b or unordered -> nle */
    if ((c <= d) || (c != c)) sum |= 2;    /* c <= d or c is NaN -> ule */
    
    /* Mixed comparisons in ternary */
    float res = (a > b) ? ((c < d) ? 1.0f : 2.0f) : 3.0f;
    sum += (int)res;
    
    return sum;
}

__attribute__((noinline))
static int test_unlt_ltgt(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Should generate "ult" and "une" (ltgt) */
    if ((a < b) || (a != a)) sum |= 1;     /* a < b or a is NaN -> ult */
    if (islessgreater(c, d)) sum |= 2;     /* c != d and ordered -> une (ltgt) */
    
    /* Complex chain */
    if ((a != b) && !isunordered(a, b)) sum |= 4;  /* ltgt equivalent */
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons - may generate unordered condition codes */
    v4sf vcmp = va < vb;
    v2df vcmp2 = vc == vd;
    
    /* Extract results to prevent elimination */
    float fcmp[4];
    double dcmp[2];
    memcpy(fcmp, &vcmp, sizeof(vcmp));
    memcpy(dcmp, &vcmp2, sizeof(vcmp2));
    
    for (int i = 0; i < 4; i++) {
        if (fcmp[i] != 0.0f) sum += 1 << i;
    }
    for (int i = 0; i < 2; i++) {
        if (dcmp[i] != 0.0) sum += 1 << (4 + i);
    }
    
    return sum;
}

#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m256 va, __m256 vb) {
    int sum = 0;
    
    /* Use AVX intrinsics with unordered comparators */
    __m256 vcmp_unord = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
    __m256 vcmp_neq_uq = _mm256_cmp_ps(va, vb, _CMP_NEQ_UQ);
    __m256 vcmp_nlt_uq = _mm256_cmp_ps(va, vb, _CMP_NLT_UQ);
    
    /* Extract mask bits */
    int mask_unord = _mm256_movemask_ps(vcmp_unord);
    int mask_neq_uq = _mm256_movemask_ps(vcmp_neq_uq);
    int mask_nlt_uq = _mm256_movemask_ps(vcmp_nlt_uq);
    
    sum = mask_unord + (mask_neq_uq << 8) + (mask_nlt_uq << 16);
    
    return sum;
}
#endif

__attribute__((noinline))
static int test_mixed_conditional(float a, float b, double c, double d, 
                                  float e, float f, double g, double h) {
    int sum = 0;
    
    /* Complex conditional expression mixing different comparisons */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        if ((a < b) ? (c != d) : (e >= f)) {
            sum += 1;
        }
        
        if ((g == h) || (a != a) || (b != b)) {
            sum += 2;
        }
        
        /* Nested ternary with floating comparisons */
        float tmp = (c > d) ? 
                   ((e < f) ? 1.0f : ((g != h) ? 2.0f : 3.0f)) : 
                   ((a == b) ? 4.0f : 5.0f);
        sum += (int)tmp;
        
        /* Rotate values to prevent constant folding */
        float temp = a;
        a = b; b = c; c = d; d = temp;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        __builtin_nanf(""), 3.0f, __builtin_nanf("1"), 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double ddata[] = {
        1.0, 2.0, 0.0, -0.0,
        __builtin_nan(""), 3.0, __builtin_nan("1"), 4.0
    };
    
    /* Initialize vector data */
    v4sf vfa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vfb = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vda = {1.0, 2.0};
    v2df vdb = {2.0, 1.0};
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int i = 0; i < iterations; i++) {
        vcounter = i;  /* Volatile to prevent optimization */
        
        /* Call all test functions with different combinations */
        int idx = i % 8;
        int idx2 = (i + 1) % 8;
        int idx3 = (i + 2) % 8;
        int idx4 = (i + 3) % 8;
        
        total_sum += test_unordered_comparisons(
            fdata[idx], fdata[idx2], ddata[idx3], ddata[idx4]);
        
        total_sum += test_uneq_unge(
            fdata[idx], fdata[idx2], fdata[idx3], fdata[idx4]);
        
        total_sum += test_ungt_unle(
            fdata[idx], fdata[idx2], ddata[idx3], ddata[idx4]);
        
        total_sum += test_unlt_ltgt(
            fdata[idx], fdata[idx2], ddata[idx3], ddata[idx4]);
        
        total_sum += test_vector_comparisons(vfa, vfb, vda, vdb);
        
        total_sum += test_mixed_conditional(
            fdata[idx], fdata[idx2], ddata[idx3], ddata[idx4],
            fdata[(idx+4)%8], fdata[(idx2+4)%8], 
            ddata[(idx3+4)%8], ddata[(idx4+4)%8]);
        
        #ifdef __AVX__
        __m256 avx_a = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 
                                     5.0f, 6.0f, 7.0f, 8.0f);
        __m256 avx_b = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f,
                                     4.0f, 3.0f, 2.0f, 1.0f);
        total_sum += test_avx_intrinsics(avx_a, avx_b);
        #endif
        
        /* Modify data slightly each iteration */
        fdata[idx] += 0.1f;
        ddata[idx3] += 0.1;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
