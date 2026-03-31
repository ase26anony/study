/* test_fp_comparisons.c - Comprehensive test for x86 floating-point unordered condition codes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile results to prevent optimization */
volatile int global_results[32];
volatile int result_index = 0;

/* Helper to store results */
void store_result(int val) {
    if (result_index < 32) {
        global_results[result_index++] = val;
    }
}

/* Function to force generation of specific condition codes */
void test_scalar_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[16];
    int idx = 0;
    
    /* Direct comparisons with NaN to trigger UNORDERED cases */
    /* UNORDERED: nan < inf (unordered because nan is NaN) */
    results[idx++] = (nan < inf) ? 1 : 0;           /* Should be false (0) */
    
    /* UNORDERED: nan > inf */
    results[idx++] = (nan > inf) ? 1 : 0;           /* Should be false (0) */
    
    /* UNORDERED: nan <= inf */
    results[idx++] = (nan <= inf) ? 1 : 0;          /* Should be false (0) */
    
    /* UNORDERED: nan >= inf */
    results[idx++] = (nan >= inf) ? 1 : 0;          /* Should be false (0) */
    
    /* UNEQ: nan == nan (unordered equal) */
    results[idx++] = (nan == nan) ? 1 : 0;          /* Should be false (0) */
    
    /* LTGT: nan != nan (unordered not equal) */
    results[idx++] = (nan != nan) ? 1 : 0;          /* Should be true (1) */
    
    /* ORDERED: inf < inf */
    results[idx++] = (inf < inf) ? 1 : 0;           /* Should be false (0) */
    
    /* ORDERED: inf == inf */
    results[idx++] = (inf == inf) ? 1 : 0;          /* Should be true (1) */
    
    /* Mixed comparisons */
    results[idx++] = (inf > nan) ? 1 : 0;           /* UNORDERED */
    results[idx++] = (zero < nan) ? 1 : 0;          /* UNORDERED */
    results[idx++] = (one != nan) ? 1 : 0;          /* Should be true (1) */
    
    /* Store all results */
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}

/* Test GCC built-in unordered comparison functions */
void test_builtin_unordered_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[16];
    int idx = 0;
    
    /* __builtin_isunordered - maps to UNORDERED */
    results[idx++] = __builtin_isunordered(nan, inf);      /* Should be 1 */
    results[idx++] = __builtin_isunordered(inf, inf);      /* Should be 0 */
    results[idx++] = __builtin_isunordered(nan, nan);      /* Should be 1 */
    
    /* __builtin_islessgreater - maps to LTGT */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* Should be 0 (unordered) */
    results[idx++] = __builtin_islessgreater(inf, zero);   /* Should be 1 */
    results[idx++] = __builtin_islessgreater(zero, zero);  /* Should be 0 */
    
    /* __builtin_isless - should use UNLT or similar */
    results[idx++] = __builtin_isless(nan, inf);           /* Should be 0 */
    results[idx++] = __builtin_isless(neg_inf, inf);       /* Should be 1 */
    results[idx++] = __builtin_isless(one, zero);          /* Should be 0 */
    
    /* __builtin_isgreater - should use UNGT or similar */
    results[idx++] = __builtin_isgreater(inf, nan);        /* Should be 0 */
    results[idx++] = __builtin_isgreater(inf, neg_inf);    /* Should be 1 */
    results[idx++] = __builtin_isgreater(zero, one);       /* Should be 0 */
    
    /* __builtin_islessequal - should use UNLE */
    results[idx++] = __builtin_islessequal(nan, zero);     /* Should be 0 */
    results[idx++] = __builtin_islessequal(neg_inf, inf);  /* Should be 1 */
    results[idx++] = __builtin_islessequal(one, one);      /* Should be 1 */
    
    /* __builtin_isgreaterequal - should use UNGE */
    results[idx++] = __builtin_isgreaterequal(zero, nan);  /* Should be 0 */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf); /* Should be 1 */
    results[idx++] = __builtin_isgreaterequal(one, one);   /* Should be 1 */
    
    /* Complex nested expressions to force multiple condition codes */
    results[idx++] = __builtin_isunordered(nan, inf) && __builtin_isless(neg_inf, inf);
    results[idx++] = __builtin_islessgreater(one, zero) || __builtin_isgreaterequal(inf, zero);
    
    /* Ternary operator with unordered comparisons */
    results[idx++] = __builtin_isunordered(nan, zero) ? 
                     __builtin_isless(one, two) : 
                     __builtin_isgreater(inf, neg_inf);
    
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}

/* Test vector comparisons using GCC extensions */
#ifdef __SSE__
void test_vector_comparisons(void) {
    /* Define vector types */
    typedef float v4sf __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Initialize vectors with NaN and normal values */
    volatile v4sf vec_nan = (v4sf){__builtin_nanf(""), 1.0f, 2.0f, __builtin_nanf("")};
    volatile v4sf vec_inf = (v4sf){__builtin_inff(), 3.0f, 4.0f, __builtin_inff()};
    volatile v4sf vec_zero = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    volatile v4sf vec_one = (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
    
    volatile v2df dvec_nan = (v2df){__builtin_nan(""), 5.0};
    volatile v2df dvec_inf = (v2df){__builtin_inf(), 6.0};
    
    int results[8];
    int idx = 0;
    
    /* Vector comparisons that may generate multiple condition codes */
    v4sf cmp_result;
    
    /* UNORDERED comparisons */
    cmp_result = vec_nan < vec_inf;
    results[idx++] = ((int*)&cmp_result)[0];  /* Extract first element */
    
    /* ORDERED comparisons */
    cmp_result = vec_inf > vec_zero;
    results[idx++] = ((int*)&cmp_result)[1];  /* Extract second element */
    
    /* Mixed comparisons */
    cmp_result = vec_nan == vec_nan;
    results[idx++] = ((int*)&cmp_result)[2];  /* Extract third element */
    
    cmp_result = vec_one != vec_nan;
    results[idx++] = ((int*)&cmp_result)[3];  /* Extract fourth element */
    
    /* Double vector comparisons */
    v2df dbl_cmp = dvec_nan > dvec_inf;
    results[idx++] = ((long long*)&dbl_cmp)[0] != 0;
    
    dbl_cmp = dvec_inf == dvec_inf;
    results[idx++] = ((long long*)&dbl_cmp)[1] != 0;
    
    /* Use movmskps to extract comparison masks (x86 specific) */
#ifdef __x86_64__
    int mask;
    v4sf cmp = vec_nan < vec_inf;
    asm volatile ("movmskps %1, %0" : "=r"(mask) : "x"(cmp));
    results[idx++] = mask;
    
    cmp = vec_inf > vec_zero;
    asm volatile ("movmskps %1, %0" : "=r"(mask) : "x"(cmp));
    results[idx++] = mask;
#endif
    
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}
#endif

/* Test mixed-type comparisons and arithmetic */
void test_mixed_type_comparisons(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    /* Arithmetic that produces NaN */
    volatile double div_by_zero = 1.0 / 0.0;
    volatile double inf_minus_inf = d_inf - d_inf;
    volatile double zero_times_inf = 0.0 * d_inf;
    
    int results[12];
    int idx = 0;
    
    /* Cross-type comparisons */
    results[idx++] = (f_nan < d_inf) ? 1 : 0;      /* UNORDERED */
    results[idx++] = (d_nan == ld_nan) ? 1 : 0;    /* UNEQ */
    results[idx++] = (ld_inf > f_nan) ? 1 : 0;     /* UNORDERED */
    
    /* Comparisons with arithmetic results */
    results[idx++] = (inf_minus_inf == inf_minus_inf) ? 1 : 0;  /* UNEQ */
    results[idx++] = (zero_times_inf != zero_times_inf) ? 1 : 0; /* LTGT */
    results[idx++] = (div_by_zero > inf_minus_inf) ? 1 : 0;     /* UNORDERED */
    
    /* Use FMA to create complex expressions */
#ifdef __FMA__
    volatile double fma_result = __builtin_fma(d_nan, d_inf, 1.0);
    results[idx++] = (fma_result < d_inf) ? 1 : 0;
    results[idx++] = (fma_result == fma_result) ? 1 : 0;
#endif
    
    /* Long double comparisons (x87 specific) */
    results[idx++] = (ld_nan < ld_inf) ? 1 : 0;
    results[idx++] = (ld_inf > ld_nan) ? 1 : 0;
    results[idx++] = (ld_nan != ld_nan) ? 1 : 0;
    results[idx++] = (ld_inf == ld_inf) ? 1 : 0;
    
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}

/* Inline assembly with explicit condition codes */
#ifdef __x86_64__
void test_inline_assembly_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int results[8];
    int idx = 0;
    int cc_result;
    
    /* ucomisd with setp (parity flag for UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 1 (unordered) */
    
    /* ucomisd with seta (above for UNGT/UNGE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(c), "x"(d)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 0 (1.0 not above 2.0) */
    
    /* ucomisd with setb (below for UNLT/UNLE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(c), "x"(d)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 1 (1.0 below 2.0) */
    
    /* fucomi instruction (x87) */
    volatile long double ld_a = __builtin_nanl("");
    volatile long double ld_b = __builtin_infl();
    
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "m"(ld_a), "m"(ld_b)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 1 (unordered) */
    
    /* Compare with equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(c), "x"(c)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 1 (equal) */
    
    /* Compare with not equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(a)
        : "cc"
    );
    results[idx++] = cc_result;  /* Should be 1 (NaN != NaN) */
    
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}
#endif

/* Control flow based on unordered comparison results */
void test_control_flow_unordered(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {1.0, 2.0, nan, inf, -inf, 0.0};
    
    int results[16];
    int idx = 0;
    
    /* Switch statement driven by comparison results */
    for (int i = 0; i < 6; i++) {
        int condition = 0;
        
        /* Determine condition code */
        if (__builtin_isunordered(values[i], nan)) condition = 1;  /* UNORDERED */
        else if (__builtin_isless(values[i], 0.0)) condition = 2;  /* UNLT */
        else if (__builtin_isgreater(values[i], 0.0)) condition = 3; /* UNGT */
        else if (values[i] == 0.0) condition = 4;                  /* UNEQ for ordered zero */
        else condition = 5;                                        /* Other */
        
        switch (condition) {
            case 1:  /* UNORDERED */
                results[idx++] = 100 + i;
                break;
            case 2:  /* UNLT */
                results[idx++] = 200 + i;
                break;
            case 3:  /* UNGT */
                results[idx++] = 300 + i;
                break;
            case 4:  /* Ordered equal */
                results[idx++] = 400 + i;
                break;
            default: /* Other */
                results[idx++] = 500 + i;
                break;
        }
    }
    
    /* Loop with unordered comparison as condition */
    int count = 0;
    volatile double test_val = __builtin_nan("");
    while (count < 3) {
        if (__builtin_isunordered(test_val, inf)) {
            results[idx++] = 600 + count;
        } else {
            results[idx++] = 700 + count;
        }
        count++;
        test_val = (count == 2) ? 1.0 : __builtin_nan("");  /* Change on second iteration */
    }
    
    /* Complex if-else chain with mixed comparisons */
    volatile double a = nan;
    volatile double b = inf;
    volatile double c = 0.0;
    
    if (__builtin_isunordered(a, b)) {
        if (__builtin_islessgreater(c, a)) {
            results[idx++] = 800;
        } else if (__builtin_isgreaterequal(b, c)) {
            results[idx++] = 801;
        }
    } else if (__builtin_isless(a, b)) {
        results[idx++] = 802;
    } else if (__builtin_isgreater(b, a)) {
        results[idx++] = 803;
    }
    
    for (int i = 0; i < idx; i++) {
        store_result(results[i]);
    }
}

int main(void) {
    /* Only run x86-specific tests on x86 platforms */
#if defined(__i386__) || defined(__x86_64__)
    printf("Running x86 floating-point unordered comparison tests...\n");
    
    /* Reset result index */
    result_index = 0;
    
    /* Run all test suites */
    test_scalar_unordered_comparisons();
    test_builtin_unordered_functions();
    
#ifdef __SSE__
    test_vector_comparisons();
#endif
    
    test_mixed_type_comparisons();
    
#ifdef __x86_64__
    test_inline_assembly_comparisons();
#endif
    
    test_control_flow_unordered();
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < result_index; i++) {
        checksum ^= global_results[i];
        checksum += global_results[i];
    }
    
    printf("Tests completed. Result count: %d, Checksum: %d\n", 
           result_index, checksum);
    
    return checksum != 0 ? 0 : 1;
#else
    printf("Not an x86 platform, skipping tests.\n");
    return 0;
#endif
}
