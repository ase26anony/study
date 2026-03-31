#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#ifdef __x86_64__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to process comparison results */
static void process_result(int cond) {
    checksum ^= cond;
}

/* Main test function for x86 targets */
void test_x86_unordered_comparisons(void) {
    /* Volatile to prevent optimization */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    /* Create NaN through arithmetic */
    volatile double nan_arith = zero / zero;
    volatile double nan_inf = inf - inf;
    
    /* Array to store comparison results */
    int results[64];
    int idx = 0;
    
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    /* ============================================
       1. Direct unordered comparisons with operators
       ============================================ */
    
    /* UNORDERED cases: comparisons involving NaN */
    results[idx++] = (nan < inf) ? 1 : 0;           /* UNORDERED */
    results[idx++] = (nan > inf) ? 2 : 0;           /* UNORDERED */
    results[idx++] = (nan <= inf) ? 3 : 0;          /* UNORDERED */
    results[idx++] = (nan >= inf) ? 4 : 0;          /* UNORDERED */
    results[idx++] = (nan == nan) ? 5 : 0;          /* UNORDERED/UNEQ */
    results[idx++] = (nan != nan) ? 6 : 0;          /* ORDERED/LTGT */
    
    /* ORDERED cases: normal comparisons */
    results[idx++] = (inf > one) ? 7 : 0;           /* ORDERED */
    results[idx++] = (neg_inf < one) ? 8 : 0;       /* ORDERED */
    results[idx++] = (one == one) ? 9 : 0;          /* ORDERED/EQ */
    results[idx++] = (one != neg_one) ? 10 : 0;     /* ORDERED/NE */
    
    /* Mixed comparisons */
    results[idx++] = (nan_arith < one) ? 11 : 0;    /* UNORDERED */
    results[idx++] = (nan_inf > neg_one) ? 12 : 0;  /* UNORDERED */
    
    /* ============================================
       2. Built-in unordered comparison functions
       ============================================ */
    
    /* These built-ins directly map to condition codes */
    results[idx++] = __builtin_isunordered(nan, inf) ? 13 : 0;      /* UNORDERED */
    results[idx++] = __builtin_isunordered(one, nan) ? 14 : 0;      /* UNORDERED */
    results[idx++] = !__builtin_isunordered(one, inf) ? 15 : 0;     /* ORDERED */
    
    /* UNEQ: unordered or equal */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_one = 1.0f;
    results[idx++] = (f_nan == f_nan) ? 16 : 0;     /* UNORDERED/UNEQ */
    
    /* UNGE: unordered or greater-or-equal (nlt) */
    results[idx++] = __builtin_isgreaterequal(nan, one) ? 17 : 0;   /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, one) ? 18 : 0;   /* ORDERED/GE */
    
    /* UNGT: unordered or greater-than (nle) */
    results[idx++] = __builtin_isgreater(nan, one) ? 19 : 0;        /* UNGT */
    results[idx++] = __builtin_isgreater(inf, one) ? 20 : 0;        /* ORDERED/GT */
    
    /* UNLE: unordered or less-or-equal (ule) */
    results[idx++] = __builtin_islessequal(nan, one) ? 21 : 0;      /* UNLE */
    results[idx++] = __builtin_islessequal(neg_inf, one) ? 22 : 0;  /* ORDERED/LE */
    
    /* UNLT: unordered or less-than (ult) */
    results[idx++] = __builtin_isless(nan, one) ? 23 : 0;           /* UNLT */
    results[idx++] = __builtin_isless(neg_inf, one) ? 24 : 0;       /* ORDERED/LT */
    
    /* LTGT: less-than or greater-than (une) */
    results[idx++] = __builtin_islessgreater(nan, nan) ? 25 : 0;    /* UNORDERED */
    results[idx++] = __builtin_islessgreater(one, neg_one) ? 26 : 0;/* ORDERED/LTGT */
    results[idx++] = __builtin_islessgreater(one, one) ? 27 : 0;    /* ORDERED/EQ */
    
    /* ============================================
       3. Complex expressions with ternary operators
       ============================================ */
    
    /* Nested comparisons to force multiple condition codes */
    results[idx++] = (__builtin_isunordered(nan, one) || 
                      __builtin_islessgreater(one, two)) ? 28 : 0;
    
    results[idx++] = (!__builtin_isunordered(one, inf) && 
                       __builtin_isgreaterequal(inf, one)) ? 29 : 0;
    
    /* Chained comparisons */
    double a = nan, b = one, c = inf;
    results[idx++] = (a < b && b < c) ? 30 : 0;     /* UNORDERED (short-circuit) */
    results[idx++] = (a > b || b < c) ? 31 : 0;     /* UNORDERED (short-circuit) */
    
    /* ============================================
       4. Vector comparisons with GCC extensions
       ============================================ */
    
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(),
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_one = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_neg = (v4sf){-1.0f, -2.0f, -3.0f, -4.0f};
    
    /* Vector comparisons generate multiple condition checks */
    v4sf cmp_result;
    
    /* UNORDERED vector comparisons */
    cmp_result = vec_nan < vec_one;
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    results[idx++] = mask1;
    
    /* ORDERED vector comparisons */
    cmp_result = vec_one > vec_neg;
    int mask2 = __builtin_ia32_movmskps(cmp_result);
    results[idx++] = mask2;
    
    /* Mixed vector comparisons */
    cmp_result = vec_nan == vec_nan;
    int mask3 = __builtin_ia32_movmskps(cmp_result);
    results[idx++] = mask3;
    
    /* Double precision vectors */
    v2df dvec_nan = (v2df){__builtin_nan(""), __builtin_nan("")};
    v2df dvec_one = (v2df){1.0, 2.0};
    
    v2df dcmp = dvec_nan > dvec_one;
    /* Extract mask for double vector (emulated) */
    double dtemp[2];
    memcpy(dtemp, &dcmp, sizeof(dtemp));
    results[idx++] = (dtemp[0] != 0.0) ? 32 : 0;
    results[idx++] = (dtemp[1] != 0.0) ? 33 : 0;
    
    /* ============================================
       5. Inline assembly with explicit condition codes
       ============================================ */
    
    double asm_a = nan;
    double asm_b = one;
    int asm_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    results[idx++] = asm_result ? 34 : 0;
    
    /* fucomi with conditional move */
    double asm_c = inf;
    double asm_d = one;
    double asm_out;
    
    asm volatile (
        "fucomi %2, %3\n\t"
        "fcmovnu %1, %0\n\t"  /* not unordered */
        : "=t" (asm_out)
        : "u" (asm_c), "u" (asm_d), "0" (0.0)
        : "cc"
    );
    results[idx++] = (asm_out != 0.0) ? 35 : 0;
    
    /* ============================================
       6. Control flow based on unordered results
       ============================================ */
    
    /* Switch statement driven by comparison results */
    int switch_var = 0;
    
    if (__builtin_isunordered(nan, one)) switch_var |= 1;
    if (__builtin_islessgreater(one, two)) switch_var |= 2;
    if (!__builtin_isunordered(one, inf)) switch_var |= 4;
    if (__builtin_isgreaterequal(inf, one)) switch_var |= 8;
    
    switch (switch_var) {
        case 0:  results[idx++] = 36; break;  /* All false */
        case 1:  results[idx++] = 37; break;  /* UNORDERED only */
        case 2:  results[idx++] = 38; break;  /* LTGT only */
        case 4:  results[idx++] = 39; break;  /* ORDERED only */
        case 8:  results[idx++] = 40; break;  /* GE only */
        case 5:  results[idx++] = 41; break;  /* UNORDERED | ORDERED */
        case 9:  results[idx++] = 42; break;  /* UNORDERED | GE */
        case 10: results[idx++] = 43; break;  /* LTGT | GE */
        default: results[idx++] = 44; break;  /* Other combinations */
    }
    
    /* Loop with unordered condition */
    for (int i = 0; i < 3; i++) {
        volatile double x = (i == 0) ? nan : (i == 1) ? inf : one;
        volatile double y = (i == 0) ? one : (i == 1) ? nan : inf;
        
        if (__builtin_isunordered(x, y)) {
            results[idx++] = 45 + i;
        } else if (__builtin_isless(x, y)) {
            results[idx++] = 48 + i;
        } else if (__builtin_isgreater(x, y)) {
            results[idx++] = 51 + i;
        }
    }
    
    /* ============================================
       7. Math functions that can produce NaN
       ============================================ */
    
    /* Division by zero */
    volatile double div_zero = one / zero;
    results[idx++] = __builtin_isunordered(div_zero, nan) ? 54 : 0;
    
    /* FMA with NaN input */
    volatile double fma_result = __builtin_fma(nan, one, one);
    results[idx++] = __builtin_isunordered(fma_result, one) ? 55 : 0;
    
    /* sqrt of negative */
    volatile double sqrt_neg = __builtin_sqrt(neg_one);
    results[idx++] = __builtin_isunordered(sqrt_neg, sqrt_neg) ? 56 : 0;
    
    /* ============================================
       Process all results to prevent elimination
       ============================================ */
    
    for (int i = 0; i < idx; i++) {
        process_result(results[i]);
    }
    
    printf("Processed %d comparison results\n", idx);
    printf("Checksum: %d\n", checksum);
}

#else
/* Non-x86 fallback */
void test_x86_unordered_comparisons(void) {
    printf("x86-specific unordered comparison tests skipped (not x86)\n");
}
#endif

int main(void) {
    test_x86_unordered_comparisons();
    return 0;
}
