#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int sink;

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED case */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ case */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT case */
    results[idx++] = (nan != nan) ? 1 : 0;     /* UNORDERED case */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);  /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);         /* UNORDERED/UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);      /* UNORDERED/UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);    /* UNORDERED/UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNORDERED/UNGE */
    
    /* 3. Complex expressions with arithmetic that could produce NaN */
    volatile double nan_prod = (inf - inf);  /* Creates NaN */
    volatile double div_by_zero = one / zero; /* Creates inf */
    
    results[idx++] = (nan_prod < div_by_zero) ? 1 : 0;   /* UNORDERED */
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (div_by_zero > nan_prod) ? 1 : 0;   /* UNORDERED */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    
    results[idx++] = (f_nan < (float)ld_inf) ? 1 : 0;    /* UNORDERED */
    results[idx++] = ((double)f_nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    
    /* 5. Control flow based on unordered comparisons */
    for (int i = 0; i < 4; i++) {
        volatile double a = (i & 1) ? nan : inf;
        volatile double b = (i & 2) ? nan : zero;
        
        if (__builtin_isunordered(a, b)) {
            results[idx++] = 1;  /* UNORDERED */
        } else if (__builtin_isless(a, b)) {
            results[idx++] = 2;  /* UNLT */
        } else if (__builtin_isgreater(a, b)) {
            results[idx++] = 3;  /* UNGT */
        } else if (__builtin_islessequal(a, b)) {
            results[idx++] = 4;  /* UNLE */
        } else if (__builtin_isgreaterequal(a, b)) {
            results[idx++] = 5;  /* UNGE */
        } else if (__builtin_islessgreater(a, b)) {
            results[idx++] = 6;  /* LTGT */
        } else {
            results[idx++] = 7;  /* UNEQ or ORDERED */
        }
    }
    
    /* 6. Ternary operators with unordered comparisons */
    results[idx++] = (__builtin_isunordered(one, nan)) ? 8 : 9;
    results[idx++] = (!__builtin_isunordered(zero, inf)) ? 10 : 11;
    
    /* Store results to prevent elimination */
    for (int i = 0; i < idx; i++) {
        sink = results[i];
    }
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result = vec_a > vec_b;   /* May generate UNORDERED/UNGT checks */
    v4sf cmp_result2 = vec_a < vec_b;  /* May generate UNORDERED/UNLT checks */
    v4sf cmp_result3 = vec_a == vec_b; /* May generate UNORDERED/UNEQ checks */
    v4sf cmp_result4 = vec_a != vec_b; /* May generate UNORDERED/LTGT checks */
    
    /* Extract comparison masks - forces actual comparison instructions */
    int mask1, mask2, mask3, mask4;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    mask4 = __builtin_ia32_movmskps(cmp_result4);
    #else
    /* Fallback: store to memory and check */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(temp));
    mask1 = (temp[0] != 0) | ((temp[1] != 0) << 1) | 
            ((temp[2] != 0) << 2) | ((temp[3] != 0) << 3);
    #endif
    
    sink = mask1 + mask2 + mask3 + mask4;
    
    /* Double precision vector comparisons */
    v2df vec_d = {__builtin_nan(""), __builtin_inf()};
    v2df vec_e = {__builtin_inf(), __builtin_nan("")};
    
    v2df dbl_cmp = vec_d > vec_e;  /* UNORDERED/UNGT */
    v2df dbl_cmp2 = vec_d < vec_e; /* UNORDERED/UNLT */
    
    #ifdef __SSE2__
    int dbl_mask = __builtin_ia32_movmskpd(dbl_cmp);
    int dbl_mask2 = __builtin_ia32_movmskpd(dbl_cmp2);
    sink += dbl_mask + dbl_mask2;
    #endif
}

/* Test with inline assembly */
void test_inline_asm(void) {
    double a = __builtin_nan("");
    double b = __builtin_inf();
    double c = 1.0;
    int result1, result2, result3, result4;
    
    /* Inline assembly with explicit condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result2)
        : "x"(a), "x"(c)
        : "al", "cc"
    );
    
    /* Test with fucomi instruction */
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "t"(a), "u"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result4)
        : "t"(c), "u"(c)
        : "al", "cc"
    );
    
    sink = result1 + result2 + result3 + result4;
}

/* Complex function with FMA and comparisons */
void test_fma_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    
    /* Use FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, inf, 1.0);
    volatile double fma_result2 = __builtin_fma(inf, neg_inf, 0.0);  /* NaN */
    
    int fma_cmp_results[8];
    int fma_idx = 0;
    
    /* Comparisons involving FMA results */
    fma_cmp_results[fma_idx++] = (fma_result < fma_result2) ? 1 : 0;  /* UNORDERED */
    fma_cmp_results[fma_idx++] = (fma_result > 0.0) ? 1 : 0;          /* UNORDERED */
    fma_cmp_results[fma_idx++] = (fma_result2 == fma_result2) ? 1 : 0; /* UNORDERED/UNEQ */
    fma_cmp_results[fma_idx++] = (fma_result != fma_result) ? 1 : 0;   /* UNORDERED/LTGT */
    
    /* Nested comparisons in complex expressions */
    volatile double complex_expr = (fma_result < inf) ? 
                                   (fma_result2 > neg_inf ? 1.0 : 2.0) : 
                                   (__builtin_isunordered(fma_result, fma_result2) ? 3.0 : 4.0);
    
    fma_cmp_results[fma_idx++] = (complex_expr == complex_expr) ? 5 : 6;
    
    /* Switch statement based on comparison results */
    int cmp_flags = 0;
    cmp_flags |= __builtin_isunordered(fma_result, 0.0) ? 0x1 : 0;
    cmp_flags |= __builtin_isless(fma_result, fma_result2) ? 0x2 : 0;
    cmp_flags |= __builtin_isgreater(fma_result, fma_result2) ? 0x4 : 0;
    cmp_flags |= __builtin_islessequal(fma_result, fma_result2) ? 0x8 : 0;
    cmp_flags |= __builtin_isgreaterequal(fma_result, fma_result2) ? 0x10 : 0;
    cmp_flags |= __builtin_islessgreater(fma_result, fma_result2) ? 0x20 : 0;
    
    switch (cmp_flags & 0x3F) {
        case 0x01:  /* UNORDERED only */
            fma_cmp_results[fma_idx++] = 100;
            break;
        case 0x22:  /* UNORDERED | LTGT */
            fma_cmp_results[fma_idx++] = 101;
            break;
        case 0x09:  /* UNORDERED | UNLE */
            fma_cmp_results[fma_idx++] = 102;
            break;
        case 0x11:  /* UNORDERED | UNGE */
            fma_cmp_results[fma_idx++] = 103;
            break;
        default:
            fma_cmp_results[fma_idx++] = 104;
            break;
    }
    
    for (int i = 0; i < fma_idx; i++) {
        sink = fma_cmp_results[i];
    }
}

int main(void) {
    /* Run all tests */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm();
    test_fma_comparisons();
    
    /* Print something to prevent complete optimization */
    printf("Test completed. sink = %d\n", sink);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
