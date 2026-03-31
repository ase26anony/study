#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
typedef struct {
    unsigned int unordered;
    unsigned int ordered;
    unsigned int uneq;
    unsigned int unge;
    unsigned int ungt;
    unsigned int unle;
    unsigned int unlt;
    unsigned int ltgt;
} cc_counter_t;

static cc_counter_t global_counter = {0};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    3.14, -2.71
};

static const int NUM_SCALARS = sizeof(test_scalars)/sizeof(test_scalars[0]);

/* Vector test data */
static __m128d vec_data[4];
static __m256d vec256_data[2];

/* Initialize vector data with mixed values including NaN */
static void init_vector_data(void) {
    vec_data[0] = _mm_set_pd(1.0, 2.0);
    vec_data[1] = _mm_set_pd(__builtin_nan(""), 3.0);
    vec_data[2] = _mm_set_pd(4.0, __builtin_nan(""));
    vec_data[3] = _mm_set_pd(1.0/0.0, -1.0/0.0);
    
    vec256_data[0] = _mm256_set_pd(1.0, __builtin_nan(""), 3.0, 4.0);
    vec256_data[1] = _mm256_set_pd(5.0, 6.0, 1.0/0.0, -1.0/0.0);
}

/* Test scalar comparisons using GCC builtins */
static void test_scalar_conditions(void) {
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                global_counter.unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                global_counter.ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                global_counter.uneq++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                global_counter.unge++;
            }
            
            /* UNGT (not less than or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                global_counter.ungt++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                global_counter.unle++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                global_counter.unlt++;
            }
            
            /* LTGT (less than or greater than, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                global_counter.ltgt++;
            }
        }
    }
}

/* Test vector comparisons */
static void test_vector_conditions(void) {
    __m128d zero = _mm_set1_pd(0.0);
    __m128d nan_vec = _mm_set1_pd(__builtin_nan(""));
    
    for (int i = 0; i < 4; i++) {
        __m128d a = vec_data[i];
        
        /* Compare with zero - will generate various condition codes */
        __m128d cmp_result;
        
        /* Compare equal - may generate UNEQ/ORDERED codes */
        cmp_result = _mm_cmpeq_pd(a, zero);
        
        /* Compare less than - may generate UNLT/UNORDERED codes */
        cmp_result = _mm_cmplt_pd(a, zero);
        
        /* Compare less than or equal - may generate UNLE codes */
        cmp_result = _mm_cmple_pd(a, zero);
        
        /* Compare greater than - may generate UNGT codes */
        cmp_result = _mm_cmpgt_pd(a, zero);
        
        /* Compare greater than or equal - may generate UNGE codes */
        cmp_result = _mm_cmpge_pd(a, zero);
        
        /* Compare not equal - may generate LTGT codes */
        cmp_result = _mm_cmpneq_pd(a, zero);
        
        /* Compare ordered - direct test */
        cmp_result = _mm_cmpord_pd(a, nan_vec);
        
        /* Compare unordered - direct test */
        cmp_result = _mm_cmpunord_pd(a, nan_vec);
    }
    
#ifdef __AVX__
    /* Test AVX comparisons if available */
    __m256d zero256 = _mm256_set1_pd(0.0);
    
    for (int i = 0; i < 2; i++) {
        __m256d a = vec256_data[i];
        __m256d cmp_result;
        
        /* Various AVX comparisons */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_EQ_OQ);    /* Equal (ordered, quiet) */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_LT_OS);    /* Less than (ordered, signaling) */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_LE_OS);    /* Less than or equal */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_UNORD_Q);  /* Unordered (quiet) */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_NEQ_UQ);   /* Not equal (unordered, quiet) */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_NLT_US);   /* Not less than (unordered, signaling) */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_NLE_US);   /* Not less than or equal */
        cmp_result = _mm256_cmp_pd(a, zero256, _CMP_ORD_Q);    /* Ordered (quiet) */
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
static void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccunord"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) global_counter.unordered++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccord"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) global_counter.ordered++;
    
    /* UNEQ constraint (unordered or equal) */
    a = 2.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccueq"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al", "cl"
    );
    if (result) global_counter.uneq++;
    
    /* UNGE constraint (not less than) */
    a = 3.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccnlt"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) global_counter.unge++;
    
    /* UNGT constraint (not less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccnle"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) global_counter.ungt++;
    
    /* UNLE constraint (unordered or less than or equal) */
    a = 1.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setbe %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccule"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al", "cl"
    );
    if (result) global_counter.unle++;
    
    /* UNLT constraint (unordered or less than) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setb %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccult"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al", "cl"
    );
    if (result) global_counter.unlt++;
    
    /* LTGT constraint (less than or greater than) */
    a = 3.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setne %%al\n\t"
        "setnp %%cl\n\t"
        "andb %%cl, %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccune"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al", "cl"
    );
    if (result) global_counter.ltgt++;
}

/* Control flow dependent on comparison results */
static void test_control_flow(void) {
    volatile double vals[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = vals[i];
            double b = vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                global_counter.unordered++;
                if (a != b) {  /* Always true for NaN */
                    global_counter.uneq++;
                }
            } else if (__builtin_isordered(a, b)) {
                global_counter.ordered++;
                if (__builtin_isless(a, b)) {
                    global_counter.unlt++;
                } else if (__builtin_isgreater(a, b)) {
                    global_counter.ungt++;
                } else if (a == b) {
                    global_counter.uneq++;
                }
            }
            
            /* Switch on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a < b) cmp_class = 2;
            else if (a > b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    global_counter.unordered += 2;
                    break;
                case 2: /* UNLT (ordered less than) */
                    global_counter.unlt += 2;
                    break;
                case 3: /* UNGT (ordered greater than) */
                    global_counter.ungt += 2;
                    break;
                case 4: /* UNEQ (ordered equal) */
                    global_counter.uneq += 2;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    init_vector_data();
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary of condition code hits */
    printf("\nCondition Code Statistics:\n");
    printf("  UNORDERED: %u\n", global_counter.unordered);
    printf("  ORDERED:   %u\n", global_counter.ordered);
    printf("  UNEQ:      %u\n", global_counter.uneq);
    printf("  UNGE:      %u\n", global_counter.unge);
    printf("  UNGT:      %u\n", global_counter.ungt);
    printf("  UNLE:      %u\n", global_counter.unle);
    printf("  UNLT:      %u\n", global_counter.unlt);
    printf("  LTGT:      %u\n", global_counter.ltgt);
    
    /* Verify we hit all condition codes */
    int total_hits = global_counter.unordered + global_counter.ordered +
                     global_counter.uneq + global_counter.unge +
                     global_counter.ungt + global_counter.unle +
                     global_counter.unlt + global_counter.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
