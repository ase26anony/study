#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int counters[8] = {0};
enum {
    CNT_UNORDERED = 0,
    CNT_ORDERED,
    CNT_UNEQ,
    CNT_UNGE,
    CNT_UNGT,
    CNT_UNLE,
    CNT_UNLT,
    CNT_LTGT
};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    0.0/0.0           /* Another NaN */
};

#define TEST_COUNT (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* ========== Scalar builtin tests ========== */
void test_scalar_builtins(void) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED */
            if (!__builtin_isunordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE (not less than) */
            if (!(a < b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!(a <= b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT (less than or greater than, but not equal) */
            if (a != b && !__builtin_isunordered(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* ========== Vector SSE tests ========== */
#ifdef __SSE2__
void test_sse_vectors(void) {
    __m128d vec_a, vec_b, cmp_result;
    __m128d nan_vec = _mm_set1_pd(__builtin_nan(""));
    __m128d inf_vec = _mm_set1_pd(1.0/0.0);
    __m128d normal_vec = _mm_set_pd(1.0, 2.0);
    
    /* Test different vector combinations */
    __m128d test_vecs[] = {
        normal_vec,
        nan_vec,
        inf_vec,
        _mm_set_pd(-1.0, 0.0)
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            vec_a = test_vecs[i];
            vec_b = test_vecs[j];
            
            /* Various comparison predicates that map to condition codes */
            cmp_result = _mm_cmpord_pd(vec_a, vec_b);  /* ORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_ORDERED] += 2;
            }
            
            cmp_result = _mm_cmpunord_pd(vec_a, vec_b); /* UNORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNORDERED] += 2;
            }
            
            cmp_result = _mm_cmpnlt_pd(vec_a, vec_b);  /* UNGE (not less than) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGE] += 2;
            }
            
            cmp_result = _mm_cmpnle_pd(vec_a, vec_b);  /* UNGT (not less than or equal) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGT] += 2;
            }
            
            cmp_result = _mm_cmple_pd(vec_a, vec_b);   /* UNLE (unordered or less than or equal) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNLE] += 2;
            }
            
            cmp_result = _mm_cmplt_pd(vec_a, vec_b);   /* UNLT (unordered or less than) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNLT] += 2;
            }
            
            cmp_result = _mm_cmpneq_pd(vec_a, vec_b);  /* LTGT (not equal and ordered) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_LTGT] += 2;
            }
        }
    }
}
#endif

/* ========== AVX vector tests ========== */
#ifdef __AVX__
void test_avx_vectors(void) {
    __m256d vec_a, vec_b, cmp_result;
    __m256d nan_vec = _mm256_set1_pd(__builtin_nan(""));
    __m256d normal_vec = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    
    vec_a = normal_vec;
    vec_b = nan_vec;
    
    /* Test various AVX comparison predicates */
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);  /* UNORDERED */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_UNORDERED] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);    /* ORDERED */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_ORDERED] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);   /* UNGE (not greater than or equal, unordered) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_UNGE] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);   /* UNGT (not greater than, unordered) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_UNGT] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_LE_OS);    /* UNLE (less than or equal, ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_UNLE] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_LT_OS);    /* UNLT (less than, ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_UNLT] += 4;
    }
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_OS);   /* LTGT (not equal, ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[CNT_LTGT] += 4;
    }
}
#endif

/* ========== Inline assembly with condition code constraints ========== */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[CNT_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* UNEQ constraint via inline asm with condition code string */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=@ueq"(result)
        : "x"(a), "x"(a)
        : "cc"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE constraint (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=@nlt"(result)
        : "x"(c), "x"(a)
        : "cc"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT constraint (not less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=@nle"(result)
        : "x"(c), "x"(a)
        : "cc"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=@ule"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=@ult"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT constraint (not equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=@une"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    if (result) counters[CNT_LTGT]++;
}

/* ========== Control flow based on comparisons ========== */
void test_control_flow(void) {
    volatile double vals[] = {1.0, __builtin_nan(""), 0.0, -1.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = vals[i];
            double b = vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
                if (!__builtin_isless(a, b)) {
                    counters[CNT_UNGE]++;
                }
            } else if (__builtin_isless(a, b)) {
                counters[CNT_UNLT]++;
                if (__builtin_islessequal(a, b)) {
                    counters[CNT_UNLE]++;
                }
            } else if (__builtin_isgreater(a, b)) {
                counters[CNT_UNGT]++;
                if (__builtin_isgreaterequal(a, b)) {
                    counters[CNT_UNGE]++;
                }
            } else if (a == b) {
                counters[CNT_UNEQ]++;
            } else {
                counters[CNT_LTGT]++;
            }
            
            /* Switch based on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a < b) cmp_class = 2;
            else if (a > b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: counters[CNT_UNORDERED]++; break;
                case 2: counters[CNT_UNLT]++; break;
                case 3: counters[CNT_UNGT]++; break;
                case 4: counters[CNT_UNEQ]++; break;
            }
        }
    }
}

/* ========== Main function ========== */
int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    /* Run all tests */
    test_scalar_builtins();
    
#ifdef __SSE2__
    test_sse_vectors();
#endif
    
#ifdef __AVX__
    test_avx_vectors();
#endif
    
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    const char *names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("\nCondition code usage summary:\n");
    printf("=============================\n");
    int total = 0;
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", names[i], counters[i]);
        total += counters[i];
    }
    printf("Total comparisons: %d\n", total);
    
    /* Verify we hit all condition codes */
    int all_hit = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("WARNING: %s was not triggered!\n", names[i]);
            all_hit = 0;
        }
    }
    
    if (all_hit) {
        printf("\nSUCCESS: All condition codes were exercised!\n");
    } else {
        printf("\nWARNING: Some condition codes were not triggered\n");
    }
    
    return all_hit ? 0 : 1;
}
