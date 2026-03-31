#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#if defined(__x86_64__) || defined(__i386__) || defined(__i686__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to accumulate results */
static void accumulate(int cond) {
    checksum ^= (cond << 8) | (checksum & 0xFF);
}

/* Test function with various unordered comparisons */
__attribute__((noinline))
static void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct comparisons with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED/ORDERED */
    results[idx++] = (nan > inf) ? 1 : 0;
    results[idx++] = (nan <= inf) ? 1 : 0;
    results[idx++] = (nan >= inf) ? 1 : 0;
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNEQ */
    results[idx++] = (nan != nan) ? 1 : 0;     /* LTGT */
    results[idx++] = (inf != nan) ? 1 : 0;
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);   /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf); /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);        /* UNLT? */
    results[idx++] = __builtin_isgreater(nan, inf);
    results[idx++] = __builtin_islessequal(nan, inf);   /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNGE */
    
    /* 3. Ordered comparisons that may become unordered */
    results[idx++] = __builtin_isless(inf, nan);
    results[idx++] = __builtin_isgreater(inf, nan);
    results[idx++] = __builtin_islessequal(inf, nan);
    results[idx++] = __builtin_isgreaterequal(inf, nan);
    
    /* 4. Mixed NaN comparisons */
    volatile float nanf = __builtin_nanf("");
    volatile float inff = __builtin_inff();
    results[idx++] = (nanf < inff) ? 1 : 0;
    results[idx++] = (nanf == nanf) ? 1 : 0;
    results[idx++] = (nanf != nanf) ? 1 : 0;
    
    /* 5. Long double comparisons */
    volatile long double nanl = __builtin_nanl("");
    volatile long double infl = __builtin_infl();
    results[idx++] = (nanl < infl) ? 1 : 0;
    results[idx++] = (nanl == nanl) ? 1 : 0;
    
    /* 6. Arithmetic producing NaN followed by comparison */
    volatile double nan_prod = inf * zero;
    results[idx++] = (nan_prod < one) ? 1 : 0;
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;
    
    volatile double nan_div = inf / inf;
    results[idx++] = (nan_div > zero) ? 1 : 0;
    
    volatile double nan_sub = inf - inf;
    results[idx++] = (nan_sub <= one) ? 1 : 0;
    
    /* Accumulate all results */
    for (int i = 0; i < idx; i++) {
        accumulate(results[i]);
    }
}

/* Test with vector comparisons */
__attribute__((noinline))
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -__builtin_inff()};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -__builtin_inff(), __builtin_inff()};
    v4sf vec_c = {2.0f, 2.0f, 2.0f, 2.0f};
    
    /* Vector comparisons that may generate unordered condition codes */
    v4sf cmp1 = vec_a > vec_b;   /* May generate UNGT/UNLT */
    v4sf cmp2 = vec_a < vec_b;
    v4sf cmp3 = vec_a == vec_b;  /* May generate UNEQ */
    v4sf cmp4 = vec_a != vec_b;  /* May generate LTGT */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3, mask4;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp1);
    mask2 = __builtin_ia32_movmskps(cmp2);
    mask3 = __builtin_ia32_movmskps(cmp3);
    mask4 = __builtin_ia32_movmskps(cmp4);
    #else
    /* Fallback: store to memory and check */
    float store1[4], store2[4], store3[4], store4[4];
    memcpy(store1, &cmp1, sizeof(cmp1));
    memcpy(store2, &cmp2, sizeof(cmp2));
    memcpy(store3, &cmp3, sizeof(cmp3));
    memcpy(store4, &cmp4, sizeof(cmp4));
    mask1 = (store1[0] != 0) | ((store1[1] != 0) << 1) | 
            ((store1[2] != 0) << 2) | ((store1[3] != 0) << 3);
    mask2 = (store2[0] != 0) | ((store2[1] != 0) << 1) | 
            ((store2[2] != 0) << 2) | ((store2[3] != 0) << 3);
    mask3 = (store3[0] != 0) | ((store3[1] != 0) << 1) | 
            ((store3[2] != 0) << 2) | ((store3[3] != 0) << 3);
    mask4 = (store4[0] != 0) | ((store4[1] != 0) << 1) | 
            ((store4[2] != 0) << 2) | ((store4[3] != 0) << 3);
    #endif
    
    accumulate(mask1);
    accumulate(mask2);
    accumulate(mask3);
    accumulate(mask4);
    
    /* Double vector comparisons */
    v2df dvec_a = {__builtin_nan(""), __builtin_inf()};
    v2df dvec_b = {__builtin_inf(), __builtin_nan("")};
    v2df dcmp = dvec_a > dvec_b;
    
    #ifdef __SSE2__
    int dmask = __builtin_ia32_movmskpd(dcmp);
    #else
    double dstore[2];
    memcpy(dstore, &dcmp, sizeof(dcmp));
    dmask = (dstore[0] != 0) | ((dstore[1] != 0) << 1);
    #endif
    
    accumulate(dmask);
}

/* Test with inline assembly */
__attribute__((noinline))
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -__builtin_inf();
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with explicit condition codes */
    #if defined(__x86_64__)
    /* Using ucomisd and checking parity flag (unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result1)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* Checking for UNEQ (ZF=1 or PF=1) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "x" (a), "x" (a)  /* nan == nan */
        : "al", "bl", "cc"
    );
    
    /* Checking for LTGT (unequal and ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "setnp %%bl\n\t"
        "andb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result3)
        : "x" (b), "x" (c)  /* inf != 1.0 */
        : "al", "bl", "cc"
    );
    
    /* Checking for UNGE (not less than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result4)
        : "x" (b), "x" (d)  /* inf >= -inf */
        : "al", "cc"
    );
    #endif
    
    accumulate(result1);
    accumulate(result2);
    accumulate(result3);
    accumulate(result4);
}

/* Control flow based on unordered comparisons */
__attribute__((noinline))
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, -inf, 0.0};
    
    int switch_result = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Complex condition that may use various unordered codes */
            if (__builtin_isunordered(vals[i], vals[j])) {
                switch_result |= 1;
            }
            if (__builtin_isless(vals[i], vals[j])) {
                switch_result |= 2;
            }
            if (__builtin_isgreater(vals[i], vals[j])) {
                switch_result |= 4;
            }
            if (__builtin_islessequal(vals[i], vals[j])) {
                switch_result |= 8;
            }
            if (__builtin_isgreaterequal(vals[i], vals[j])) {
                switch_result |= 16;
            }
            if (__builtin_islessgreater(vals[i], vals[j])) {
                switch_result |= 32;
            }
        }
    }
    
    /* Switch on combined comparison results */
    switch (switch_result & 0x3F) {
        case 0:  accumulate(0); break;
        case 1:  accumulate(1); break;  /* UNORDERED */
        case 2:  accumulate(2); break;  /* UNLT */
        case 4:  accumulate(3); break;  /* UNGT */
        case 8:  accumulate(4); break;  /* UNLE */
        case 16: accumulate(5); break;  /* UNGE */
        case 32: accumulate(6); break;  /* LTGT */
        case 3:  accumulate(7); break;  /* Combination */
        case 63: accumulate(8); break;  /* All bits */
        default: accumulate(9); break;
    }
}

/* Test with FMA and math functions */
__attribute__((noinline))
static void test_math_operations(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* FMA operations that might produce NaN */
    #ifdef __FMA__
    volatile double fma1 = __builtin_fma(nan, inf, zero);
    volatile double fma2 = __builtin_fma(inf, zero, inf);
    #else
    volatile double fma1 = nan * inf + zero;
    volatile double fma2 = inf * zero + inf;
    #endif
    
    /* Comparisons after math ops */
    int r1 = (fma1 < fma2) ? 1 : 0;
    int r2 = (fma1 == fma1) ? 1 : 0;
    int r3 = (fma2 != fma2) ? 1 : 0;
    int r4 = __builtin_islessgreater(fma1, fma2);
    int r5 = __builtin_isunordered(fma1, fma2);
    
    accumulate(r1);
    accumulate(r2);
    accumulate(r3);
    accumulate(r4);
    accumulate(r5);
    
    /* sqrt of negative number */
    volatile double neg = -1.0;
    volatile double sqrt_result = __builtin_sqrt(neg);
    if (__builtin_isunordered(sqrt_result, sqrt_result)) {
        accumulate(10);
    }
}

int main(void) {
    printf("Starting unordered comparison tests...\n");
    
    /* Run all test suites */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm();
    test_control_flow();
    test_math_operations();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
}
#endif
