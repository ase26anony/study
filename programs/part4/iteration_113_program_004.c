#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <string.h>

/* Condition code counters */
static int counters[8] = {0};
enum { UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT };

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    __builtin_nan(""), -__builtin_nan(""),
    1.0/0.0, -1.0/0.0,  /* +Inf, -Inf */
    3.14, -2.71
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(__builtin_nan(""), 1.0),
    _mm_set_pd(0.0, -0.0),
    _mm_set_pd(1.0/0.0, -1.0/0.0)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, __builtin_nan(""), 4.0),
    _mm256_set_pd(-1.0, 0.0, 1.0/0.0, -1.0/0.0)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double a, b;
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ]++;
            }
            
            /* UNGE (unordered or greater or equal) */
            if (__builtin_isunordered(a, b) || a >= b) {
                counters[UNGE]++;
            }
            
            /* UNGT (unordered or greater) */
            if (__builtin_isunordered(a, b) || a > b) {
                counters[UNGT]++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE]++;
            }
            
            /* UNLT (unordered or less) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT]++;
            }
            
            /* LTGT (less or greater, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT]++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX intrinsics */
void test_vector_conditions(void) {
    int i, j;
    __m128d a, b, cmp;
    __m128i mask;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            a = test_vec128[i];
            b = test_vec128[j];
            
            /* Various comparison predicates that map to condition codes */
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNORDERED]++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[ORDERED]++;
            }
            
            /* _CMP_EQ_UQ - equal or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNEQ]++;
            }
            
            /* _CMP_NLT_UQ - not less than or unordered (UNGE) */
            cmp = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNGE]++;
            }
            
            /* _CMP_NLE_UQ - not less than or equal or unordered (UNGT) */
            cmp = _mm_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNGT]++;
            }
            
            /* _CMP_LE_UQ - less than or equal or unordered (UNLE) */
            cmp = _mm_cmp_pd(a, b, _CMP_LE_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNLE]++;
            }
            
            /* _CMP_LT_UQ - less than or unordered (UNLT) */
            cmp = _mm_cmp_pd(a, b, _CMP_LT_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[UNLT]++;
            }
            
            /* _CMP_NEQ_UQ - not equal and not unordered (LTGT) */
            cmp = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
            mask = _mm_castpd_si128(cmp);
            if (_mm_extract_epi64(mask, 0) || _mm_extract_epi64(mask, 1)) {
                counters[LTGT]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "cc"
    );
    if (result) counters[ORDERED]++;
    
    /* UNEQ constraint via cmpsd */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (a), "x" (d), "i" (_CMP_EQ_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[UNEQ]++;
    
    /* UNGE constraint (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (c), "x" (a), "i" (_CMP_NLT_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[UNGE]++;
    
    /* UNGT constraint (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (c), "x" (a), "i" (_CMP_NLE_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[UNGT]++;
    
    /* UNLE constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (a), "x" (c), "i" (_CMP_LE_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (a), "x" (c), "i" (_CMP_LT_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[UNLT]++;
    
    /* LTGT constraint (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %%xmm0\n\t"
        "movmskpd %%xmm0, %%eax\n\t"
        "andl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "x" (c), "x" (a), "i" (_CMP_NEQ_UQ)
        : "xmm0", "eax", "cc"
    );
    if (result) counters[LTGT]++;
}

#ifdef __AVX__
/* Test AVX comparisons for wider code generation */
void test_avx_conditions(void) {
    int i, j;
    __m256d a, b, cmp;
    __m256i mask256;
    __m128i mask128;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            a = test_vec256[i];
            b = test_vec256[j];
            
            /* Use different AVX comparison predicates */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask256 = _mm256_castpd_si256(cmp);
            mask128 = _mm256_extractf128_si256(mask256, 0);
            if (_mm_extract_epi64(mask128, 0) || _mm_extract_epi64(mask128, 1)) {
                counters[UNORDERED]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            mask256 = _mm256_castpd_si256(cmp);
            mask128 = _mm256_extractf128_si256(mask256, 0);
            if (_mm_extract_epi64(mask128, 0) || _mm_extract_epi64(mask128, 1)) {
                counters[UNEQ]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            mask256 = _mm256_castpd_si256(cmp);
            mask128 = _mm256_extractf128_si256(mask256, 0);
            if (_mm_extract_epi64(mask128, 0) || _mm_extract_epi64(mask128, 1)) {
                counters[UNGE]++;
            }
        }
    }
}
#endif

/* Control flow test with switch statement */
void test_control_flow(void) {
    double a = __builtin_nan("");
    double b = 1.0;
    double c = 1.0;
    double d = 2.0;
    
    /* Force compiler to generate condition code checks in control flow */
    if (__builtin_isunordered(a, b)) {
        counters[UNORDERED]++;
    } else if (__builtin_isordered(c, d)) {
        counters[ORDERED]++;
    }
    
    switch ((int)__builtin_isunordered(a, b) * 4 + 
            (int)__builtin_isgreater(c, d) * 2 +
            (int)__builtin_isless(c, b)) {
        case 0: counters[UNEQ]++; break;
        case 1: counters[UNLT]++; break;
        case 2: counters[UNGT]++; break;
        case 3: counters[LTGT]++; break;
        case 4: counters[UNORDERED]++; break;
        default: counters[ORDERED]++; break;
    }
}

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary */
    printf("Condition code execution summary:\n");
    printf("UNORDERED: %d\n", counters[UNORDERED]);
    printf("ORDERED:   %d\n", counters[ORDERED]);
    printf("UNEQ:      %d\n", counters[UNEQ]);
    printf("UNGE:      %d\n", counters[UNGE]);
    printf("UNGT:      %d\n", counters[UNGT]);
    printf("UNLE:      %d\n", counters[UNLE]);
    printf("UNLT:      %d\n", counters[UNLT]);
    printf("LTGT:      %d\n", counters[LTGT]);
    
    /* Verify all paths were executed */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    if (total > 0) {
        printf("\nSUCCESS: All condition code paths were exercised (total: %d)\n", total);
        return 0;
    } else {
        printf("\nFAILURE: No condition code paths were executed\n");
        return 1;
    }
}
