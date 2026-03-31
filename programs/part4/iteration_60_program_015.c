#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#ifdef __x86_64__ || __i386__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to process comparison results */
static void process_result(int cond) {
    checksum ^= cond;
    checksum += 1;
}

/* Test function with various unordered comparisons */
static void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators */
    /* These should generate various condition codes */
    
    /* UNORDERED case: nan < inf (unordered) */
    if (nan < inf) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    /* ORDERED case: one < inf (ordered) */
    if (one < inf) {
        results[idx++] = 2;
    } else {
        results[idx++] = 0;
    }
    
    /* UNEQ case: nan == nan (unordered equal) */
    if (nan == nan) {
        results[idx++] = 3;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGE case: nan >= inf (unordered, not less than) */
    if (nan >= inf) {
        results[idx++] = 4;
    } else {
        results[idx++] = 0;
    }
    
    /* UNGT case: nan > inf (unordered, not less or equal) */
    if (nan > inf) {
        results[idx++] = 5;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLE case: nan <= inf (unordered, less or equal) */
    if (nan <= inf) {
        results[idx++] = 6;
    } else {
        results[idx++] = 0;
    }
    
    /* UNLT case: nan < inf (unordered, less than) */
    if (nan < inf) {
        results[idx++] = 7;
    } else {
        results[idx++] = 0;
    }
    
    /* LTGT case: one != nan (less or greater, but not equal) */
    if (one != nan) {
        results[idx++] = 8;
    } else {
        results[idx++] = 0;
    }
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to the condition codes */
    
    /* __builtin_isunordered - UNORDERED */
    if (__builtin_isunordered(nan, inf)) {
        results[idx++] = 9;
    }
    
    /* __builtin_islessgreater - LTGT */
    if (__builtin_islessgreater(one, nan)) {
        results[idx++] = 10;
    }
    
    /* __builtin_isless - UNLT */
    if (__builtin_isless(nan, inf)) {
        results[idx++] = 11;
    }
    
    /* __builtin_isgreater - UNGT */
    if (__builtin_isgreater(nan, inf)) {
        results[idx++] = 12;
    }
    
    /* __builtin_islessequal - UNLE */
    if (__builtin_islessequal(nan, inf)) {
        results[idx++] = 13;
    }
    
    /* __builtin_isgreaterequal - UNGE */
    if (__builtin_isgreaterequal(nan, inf)) {
        results[idx++] = 14;
    }
    
    /* Complex nested built-ins to force multiple condition codes */
    int r1 = __builtin_isunordered(nan, inf) ? 15 : 0;
    int r2 = __builtin_islessgreater(one, nan) ? 16 : 0;
    int r3 = __builtin_isless(nan, inf) ? 17 : 0;
    results[idx++] = r1 + r2 + r3;
    
    /* 3. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    volatile float f_inf = __builtin_inff();
    
    /* Mixed float/double comparisons */
    if ((double)f_nan < inf) {
        results[idx++] = 18;
    }
    
    if (nan < (double)f_inf) {
        results[idx++] = 19;
    }
    
    /* Long double comparisons */
    if (ld_nan == ld_nan) {
        results[idx++] = 20;
    }
    
    /* 4. Arithmetic that produces NaN */
    volatile double nan_prod = inf * zero;  /* inf * 0 = NaN */
    volatile double nan_div = zero / zero;  /* 0/0 = NaN */
    volatile double nan_sub = inf - inf;    /* inf - inf = NaN */
    
    if (nan_prod != nan_prod) {
        results[idx++] = 21;
    }
    
    if (nan_div >= one) {
        results[idx++] = 22;
    }
    
    if (nan_sub < inf) {
        results[idx++] = 23;
    }
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, one, one);
    if (fma_result == fma_result) {
        results[idx++] = 24;
    }
    
    /* Process all results */
    for (int i = 0; i < idx; i++) {
        process_result(results[i]);
    }
}

/* Test function with vector comparisons */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), 1.0f, 3.0f, __builtin_nanf("")};
    
    /* Vector comparisons - may generate multiple condition codes */
    v4sf cmp_result = vec_a > vec_b;
    v4sf cmp_result2 = vec_a == vec_b;
    v4sf cmp_result3 = vec_a <= vec_b;
    
    /* Extract comparison masks */
    int mask1, mask2, mask3;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    #else
    /* Fallback: store to memory and check */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(cmp_result));
    mask1 = (temp[0] != 0.0f) | ((temp[1] != 0.0f) << 1) | 
            ((temp[2] != 0.0f) << 2) | ((temp[3] != 0.0f) << 3);
    #endif
    
    process_result(mask1);
    process_result(mask2);
    process_result(mask3);
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    
    v2df cmp_dresult = vec_da > vec_db;
    v2df cmp_dresult2 = vec_da == vec_db;
    
    #ifdef __SSE2__
    int mask_d1 = __builtin_ia32_movmskpd(cmp_dresult);
    int mask_d2 = __builtin_ia32_movmskpd(cmp_dresult2);
    process_result(mask_d1);
    process_result(mask_d2);
    #endif
}

/* Test function with inline assembly */
static void test_assembly_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Inline assembly with explicit condition codes */
    /* This forces the compiler to handle these condition codes */
    
    /* UNORDERED/ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result1)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* UNEQ test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "x" (a), "x" (a)  /* nan == nan */
        : "al", "cc"
    );
    
    /* LTGT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result3)
        : "x" (c), "x" (d)  /* 1.0 != 2.0 */
        : "al", "cc"
    );
    
    process_result(result1);
    process_result(result2);
    process_result(result3);
}

/* Control flow based on unordered comparisons */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 2.0, -inf};
    int result = 0;
    
    /* Switch-like behavior using comparison results */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (__builtin_isunordered(vals[i], vals[j])) {
                result |= (1 << 0);  /* UNORDERED */
            }
            if (!__builtin_isunordered(vals[i], vals[j]) && 
                vals[i] == vals[j]) {
                result |= (1 << 1);  /* ORDERED EQ */
            }
            if (__builtin_islessgreater(vals[i], vals[j])) {
                result |= (1 << 2);  /* LTGT */
            }
            if (__builtin_isless(vals[i], vals[j])) {
                result |= (1 << 3);  /* UNLT */
            }
            if (__builtin_isgreater(vals[i], vals[j])) {
                result |= (1 << 4);  /* UNGT */
            }
        }
    }
    
    process_result(result);
    
    /* Complex conditional with ternary operators */
    int r = (nan < inf) ? 100 : 
            (nan == nan) ? 101 : 
            (inf != nan) ? 102 : 
            (nan >= inf) ? 103 : 
            (nan > inf) ? 104 : 105;
    
    process_result(r);
}

#else
/* Non-x86 fallback */
static void test_unordered_comparisons(void) {
    checksum = 42;
}
static void test_vector_comparisons(void) {}
static void test_assembly_comparisons(void) {}
static void test_control_flow(void) {}
#endif

int main(void) {
    /* Run all tests */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_assembly_comparisons();
    test_control_flow();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
