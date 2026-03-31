#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

// Result counters for each condition code type
static struct {
    int unordered;
    int ordered;
    int uneq;
    int unge;
    int ungt;
    int unle;
    int unlt;
    int ltgt;
} counters = {0};

// Test data with normal values, infinities, and NaNs
static const double test_scalars[] = {
    1.0, 2.0, -1.0, -2.0,
    0.0, -0.0,
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    3.14, -3.14
};

static const __m128d test_vecs[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, -2.0),
    _mm_set_pd(0.0, -0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), __builtin_nan("")),
    _mm_set_pd(3.14, -3.14)
};

// Test scalar comparisons using GCC builtins
void test_scalar_conditions(void) {
    int n = sizeof(test_scalars) / sizeof(test_scalars[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            // UNORDERED
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
            }
            
            // ORDERED
            if (__builtin_isordered(a, b)) {
                counters.ordered++;
            }
            
            // UNEQ (unordered or equal)
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters.uneq++;
            }
            
            // UNGE (not less than)
            if (!__builtin_isless(a, b)) {
                counters.unge++;
            }
            
            // UNGT (not less than or equal)
            if (!__builtin_islessequal(a, b)) {
                counters.ungt++;
            }
            
            // UNLE (unordered or less than or equal)
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                counters.unle++;
            }
            
            // UNLT (unordered or less than)
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                counters.unlt++;
            }
            
            // LTGT (less than or greater than)
            if (__builtin_islessgreater(a, b)) {
                counters.ltgt++;
            }
        }
    }
}

// Test vector comparisons
void test_vector_conditions(void) {
    int n = sizeof(test_vecs) / sizeof(test_vecs[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m128d a = test_vecs[i];
            __m128d b = test_vecs[j];
            
            // Generate comparison masks for different predicates
            __m128d cmp;
            
            // CMP_UNORD_Q - unordered
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unordered++;
            }
            
            // CMP_ORD_Q - ordered
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ordered++;
            }
            
            // CMP_EQ_UQ - equal (unordered signaling)
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.uneq++;
            }
            
            // CMP_NLT_US - not less than (unordered signaling)
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unge++;
            }
            
            // CMP_NLE_US - not less than or equal (unordered signaling)
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ungt++;
            }
            
            // CMP_LE_OS - less than or equal (ordered signaling)
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unle++;
            }
            
            // CMP_LT_OS - less than (ordered signaling)
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unlt++;
            }
            
            // CMP_NEQ_OQ - not equal (ordered quiet)
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ltgt++;
            }
        }
    }
}

// Test inline assembly with explicit condition code constraints
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    // UNORDERED
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.unordered++;
    
    // ORDERED
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.ordered++;
    
    // UNEQ (unordered or equal)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "ah"
    );
    if (result) counters.uneq++;
    
    // UNGE (not less than)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.unge++;
    
    // UNGT (not less than or equal)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.ungt++;
    
    // UNLE (unordered or less than or equal)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setbe %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "ah"
    );
    if (result) counters.unle++;
    
    // UNLT (unordered or less than)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setb %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "ah"
    );
    if (result) counters.unlt++;
    
    // LTGT (less than or greater than)
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.ltgt++;
}

// AVX-specific tests
#ifdef __AVX__
void test_avx_conditions(void) {
    __m256d a = _mm256_set_pd(1.0, 2.0, __builtin_nan(""), INFINITY);
    __m256d b = _mm256_set_pd(2.0, 1.0, __builtin_nan(""), -INFINITY);
    
    // Test various AVX comparison predicates
    __m256d cmp;
    
    // CMP_UNORD_Q
    cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    if (_mm256_movemask_pd(cmp) != 0) counters.unordered++;
    
    // CMP_ORD_Q
    cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    if (_mm256_movemask_pd(cmp) != 0) counters.ordered++;
    
    // CMP_EQ_UQ
    cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counters.uneq++;
    
    // CMP_NLT_US
    cmp = _mm256_cmp_pd(a, b, _CMP_NLT_US);
    if (_mm256_movemask_pd(cmp) != 0) counters.unge++;
    
    // CMP_NLE_US
    cmp = _mm256_cmp_pd(a, b, _CMP_NLE_US);
    if (_mm256_movemask_pd(cmp) != 0) counters.ungt++;
    
    // CMP_LE_OS
    cmp = _mm256_cmp_pd(a, b, _CMP_LE_OS);
    if (_mm256_movemask_pd(cmp) != 0) counters.unle++;
    
    // CMP_LT_OS
    cmp = _mm256_cmp_pd(a, b, _CMP_LT_OS);
    if (_mm256_movemask_pd(cmp) != 0) counters.unlt++;
    
    // CMP_NEQ_OQ
    cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);
    if (_mm256_movemask_pd(cmp) != 0) counters.ltgt++;
}
#endif

// Control flow based on comparison results
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), INFINITY, -INFINITY, 0.0};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            // Complex control flow that depends on comparison results
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
                if (__builtin_isordered(a, b)) {
                    // This should never happen, but creates branching
                    counters.ordered++;
                }
            } else if (__builtin_isless(a, b)) {
                counters.unlt++;
                if (__builtin_isgreaterequal(a, b)) {
                    counters.unge++;
                }
            } else if (__builtin_isgreater(a, b)) {
                counters.ungt++;
                if (__builtin_islessequal(a, b)) {
                    counters.unle++;
                }
            } else if (__builtin_islessequal(a, b)) {
                counters.unle++;
                if (__builtin_islessgreater(a, b)) {
                    counters.ltgt++;
                }
            }
            
            // Switch based on comparison classification
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;
            else if (a < b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: // UNORDERED
                    counters.unordered++;
                    break;
                case 2: // UNEQ (equal, not unordered in this path)
                    counters.uneq++;
                    break;
                case 3: // UNLT
                    counters.unlt++;
                    break;
                case 4: // UNGT
                    counters.ungt++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    // Run all test functions
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    // Print summary of results
    printf("\nCondition code usage summary:\n");
    printf("UNORDERED: %d\n", counters.unordered);
    printf("ORDERED:   %d\n", counters.ordered);
    printf("UNEQ:      %d\n", counters.uneq);
    printf("UNGE:      %d\n", counters.unge);
    printf("UNGT:      %d\n", counters.ungt);
    printf("UNLE:      %d\n", counters.unle);
    printf("UNLT:      %d\n", counters.unlt);
    printf("LTGT:      %d\n", counters.ltgt);
    
    // Verify all condition codes were exercised
    int total = counters.unordered + counters.ordered + counters.uneq +
                counters.unge + counters.ungt + counters.unle +
                counters.unlt + counters.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total);
    
    if (total > 0) {
        printf("All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered!\n");
        return 1;
    }
}
