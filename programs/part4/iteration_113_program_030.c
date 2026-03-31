#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

// Result counters for each condition code type
typedef struct {
    int unordered;
    int ordered;
    int uneq;
    int unge;
    int ungt;
    int unle;
    int unlt;
    int ltgt;
    int total;
} ConditionCounts;

// Initialize test data with normal numbers, infinities, and NaNs
void init_test_data(double* scalar_data, __m128d* vector_data, int size) {
    for (int i = 0; i < size; i++) {
        double val = (i % 10) * 1.5;
        scalar_data[i] = val;
        
        // Insert special values at specific indices
        if (i % 7 == 0) scalar_data[i] = __builtin_nan("");
        if (i % 11 == 0) scalar_data[i] = 1.0 / 0.0;  // +Inf
        if (i % 13 == 0) scalar_data[i] = -1.0 / 0.0; // -Inf
        
        // Initialize vector data
        vector_data[i] = _mm_set_pd(scalar_data[i], scalar_data[(i + 1) % size]);
    }
}

// Test scalar comparisons using GCC builtins
ConditionCounts test_scalar_conditions(const double* data, int size) {
    ConditionCounts counts = {0};
    
    for (int i = 0; i < size - 1; i++) {
        double a = data[i];
        double b = data[i + 1];
        
        // UNORDERED: a or b is NaN
        if (__builtin_isunordered(a, b)) {
            counts.unordered++;
            // Prevent optimization
            volatile int dummy = 1;
            (void)dummy;
        }
        
        // ORDERED: neither is NaN
        if (__builtin_isordered(a, b)) {
            counts.ordered++;
        }
        
        // UNEQ: unordered or equal
        // Use inline assembly to force specific condition code
        int uneq_result;
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setp %0\n\t"
            "or $1, %0"
            : "=r"(uneq_result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (uneq_result) counts.uneq++;
        
        // UNGE: unordered or greater or equal
        if (!__builtin_isless(a, b)) {
            counts.unge++;
        }
        
        // UNGT: unordered or greater than
        if (!__builtin_islessequal(a, b)) {
            counts.ungt++;
        }
        
        // UNLE: unordered or less or equal
        if (!__builtin_isgreater(a, b)) {
            counts.unle++;
        }
        
        // UNLT: unordered or less than
        if (!__builtin_isgreaterequal(a, b)) {
            counts.unlt++;
        }
        
        // LTGT: less than or greater than (but not equal and not unordered)
        if (__builtin_islessgreater(a, b)) {
            counts.ltgt++;
        }
        
        counts.total++;
    }
    
    return counts;
}

// Test vector comparisons with SSE/AVX
ConditionCounts test_vector_conditions(const __m128d* data, int size) {
    ConditionCounts counts = {0};
    
    for (int i = 0; i < size - 1; i++) {
        __m128d a = data[i];
        __m128d b = data[i + 1];
        
        // Vector comparisons that generate various condition codes
        __m128d cmp_result;
        
        // Compare unordered (CMP_UNORD_Q)
        cmp_result = _mm_cmpunord_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.unordered++;
        }
        
        // Compare ordered (CMP_ORD_Q)
        cmp_result = _mm_cmpord_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.ordered++;
        }
        
        // Compare not less than (CMP_NLT_UQ) - unordered or not less than
        cmp_result = _mm_cmpnlt_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.unge++;
        }
        
        // Compare not less than or equal (CMP_NLE_UQ) - unordered or not less or equal
        cmp_result = _mm_cmpnle_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.ungt++;
        }
        
        // Compare unordered or less or equal (CMP_LE_UQ)
        cmp_result = _mm_cmple_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.unle++;
        }
        
        // Compare unordered or less than (CMP_LT_UQ)
        cmp_result = _mm_cmplt_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.unlt++;
        }
        
        // Compare not equal (CMP_NEQ_UQ) - unordered or not equal
        cmp_result = _mm_cmpneq_pd(a, b);
        if (_mm_movemask_pd(cmp_result) != 0) {
            counts.uneq++;
        }
        
        // Compare not equal (CMP_NEQ_OQ) - ordered and not equal (LTGT)
        // This requires inline assembly to force the specific condition code
        int ltgt_mask;
        __asm__ volatile (
            "cmpordpd %1, %2\n\t"
            "cmpneqpd %1, %2\n\t"
            "movmskpd %2, %0"
            : "=r"(ltgt_mask)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (ltgt_mask != 0) {
            counts.ltgt++;
        }
        
        counts.total++;
    }
    
    return counts;
}

// Test inline assembly with explicit condition code constraints
ConditionCounts test_asm_constraints(const double* data, int size) {
    ConditionCounts counts = {0};
    
    for (int i = 0; i < size - 1; i++) {
        double a = data[i];
        double b = data[i + 1];
        int result;
        
        // Test each condition code via inline assembly constraints
        // UNORDERED
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %0"
            : "=@unord"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.unordered++;
        
        // ORDERED
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %0"
            : "=@ord"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.ordered++;
        
        // UNEQ (unordered or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"
            "setp %%al\n\t"
            "or %%al, %0"
            : "=@ueq"(result)
            : "x"(a), "x"(b)
            : "al", "cc"
        );
        if (result) counts.uneq++;
        
        // UNGE (unordered or not less than)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnb %0"
            : "=@nlt"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.unge++;
        
        // UNGT (unordered or not less or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnbe %0"
            : "=@nle"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.ungt++;
        
        // UNLE (unordered or less or equal)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %0"
            : "=@ule"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.unle++;
        
        // UNLT (unordered or less than)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %0"
            : "=@ult"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        if (result) counts.unlt++;
        
        // LTGT (less than or greater than, ordered)
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %0\n\t"
            "setnp %%al\n\t"
            "and %%al, %0"
            : "=@une"(result)
            : "x"(a), "x"(b)
            : "al", "cc"
        );
        if (result) counts.ltgt++;
        
        counts.total++;
    }
    
    return counts;
}

// Test with AVX-512 if available
#ifdef __AVX512F__
ConditionCounts test_avx512_conditions(const __m512d* data, int size) {
    ConditionCounts counts = {0};
    
    for (int i = 0; i < size - 1; i++) {
        __m512d a = data[i];
        __m512d b = data[i + 1];
        
        __mmask8 mask;
        
        // CMP_UNORD_Q
        mask = _mm512_cmp_pd_mask(a, b, _CMP_UNORD_Q);
        if (mask != 0) counts.unordered++;
        
        // CMP_ORD_Q
        mask = _mm512_cmp_pd_mask(a, b, _CMP_ORD_Q);
        if (mask != 0) counts.ordered++;
        
        // CMP_NLT_UQ
        mask = _mm512_cmp_pd_mask(a, b, _CMP_NLT_UQ);
        if (mask != 0) counts.unge++;
        
        // CMP_NLE_UQ
        mask = _mm512_cmp_pd_mask(a, b, _CMP_NLE_UQ);
        if (mask != 0) counts.ungt++;
        
        // CMP_LE_UQ
        mask = _mm512_cmp_pd_mask(a, b, _CMP_LE_UQ);
        if (mask != 0) counts.unle++;
        
        // CMP_LT_UQ
        mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_UQ);
        if (mask != 0) counts.unlt++;
        
        // CMP_NEQ_UQ
        mask = _mm512_cmp_pd_mask(a, b, _CMP_NEQ_UQ);
        if (mask != 0) counts.uneq++;
        
        // CMP_NEQ_OQ (LTGT)
        mask = _mm512_cmp_pd_mask(a, b, _CMP_NEQ_OQ);
        if (mask != 0) counts.ltgt++;
        
        counts.total++;
    }
    
    return counts;
}
#endif

// Merge counts from multiple test runs
void merge_counts(ConditionCounts* total, const ConditionCounts* new_counts) {
    total->unordered += new_counts->unordered;
    total->ordered += new_counts->ordered;
    total->uneq += new_counts->uneq;
    total->unge += new_counts->unge;
    total->ungt += new_counts->ungt;
    total->unle += new_counts->unle;
    total->unlt += new_counts->unlt;
    total->ltgt += new_counts->ltgt;
    total->total += new_counts->total;
}

int main() {
    const int DATA_SIZE = 64;
    
    // Allocate and initialize test data
    double* scalar_data = (double*)aligned_alloc(64, DATA_SIZE * sizeof(double));
    __m128d* vector_data = (__m128d*)aligned_alloc(64, DATA_SIZE * sizeof(__m128d));
    
    init_test_data(scalar_data, vector_data, DATA_SIZE);
    
    ConditionCounts total_counts = {0};
    
    // Run all test suites
    printf("Running scalar condition tests...\n");
    ConditionCounts scalar_counts = test_scalar_conditions(scalar_data, DATA_SIZE);
    merge_counts(&total_counts, &scalar_counts);
    
    printf("Running vector condition tests...\n");
    ConditionCounts vector_counts = test_vector_conditions(vector_data, DATA_SIZE);
    merge_counts(&total_counts, &vector_counts);
    
    printf("Running inline assembly constraint tests...\n");
    ConditionCounts asm_counts = test_asm_constraints(scalar_data, DATA_SIZE);
    merge_counts(&total_counts, &asm_counts);
    
#ifdef __AVX512F__
    printf("Running AVX-512 condition tests...\n");
    __m512d* avx512_data = (__m512d*)aligned_alloc(64, (DATA_SIZE/8) * sizeof(__m512d));
    for (int i = 0; i < DATA_SIZE/8; i++) {
        avx512_data[i] = _mm512_set_pd(
            scalar_data[i*8], scalar_data[i*8+1], scalar_data[i*8+2], scalar_data[i*8+3],
            scalar_data[i*8+4], scalar_data[i*8+5], scalar_data[i*8+6], scalar_data[i*8+7]
        );
    }
    ConditionCounts avx512_counts = test_avx512_conditions(avx512_data, DATA_SIZE/8);
    merge_counts(&total_counts, &avx512_counts);
    free(avx512_data);
#endif
    
    // Print summary
    printf("\n=== Condition Code Test Results ===\n");
    printf("UNORDERED:  %d\n", total_counts.unordered);
    printf("ORDERED:    %d\n", total_counts.ordered);
    printf("UNEQ:       %d\n", total_counts.uneq);
    printf("UNGE:       %d\n", total_counts.unge);
    printf("UNGT:       %d\n", total_counts.ungt);
    printf("UNLE:       %d\n", total_counts.unle);
    printf("UNLT:       %d\n", total_counts.unlt);
    printf("LTGT:       %d\n", total_counts.ltgt);
    printf("Total comparisons: %d\n", total_counts.total);
    
    // Verify we hit all condition codes
    int all_hit = (total_counts.unordered > 0) &&
                  (total_counts.ordered > 0) &&
                  (total_counts.uneq > 0) &&
                  (total_counts.unge > 0) &&
                  (total_counts.ungt > 0) &&
                  (total_counts.unle > 0) &&
                  (total_counts.unlt > 0) &&
                  (total_counts.ltgt > 0);
    
    printf("\nAll condition codes triggered: %s\n", all_hit ? "YES" : "NO");
    
    // Cleanup
    free(scalar_data);
    free(vector_data);
    
    return all_hit ? 0 : 1;
}
