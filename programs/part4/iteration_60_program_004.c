/* test_i386_float_conds.c
 * 
 * This program is designed to trigger the uncovered condition code
 * mnemonics in GCC's i386 backend (i386.cc lines 13992-14017).
 * It uses various techniques to force generation of unordered
 * floating-point comparison condition codes.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Function to use comparison results, preventing dead code elimination */
static void use_result(int cond) {
    global_counter += cond;
}

/* Function to perform various unordered comparisons */
static void test_scalar_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct comparisons with operators - these may generate various condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* Likely UNORDERED or UNLT */
    results[idx++] = (nan > inf) ? 1 : 0;      /* Likely UNORDERED or UNGT */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* Likely UNORDERED or UNLE */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* Likely UNORDERED or UNGE */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNEQ or UNORDERED */
    results[idx++] = (nan != nan) ? 1 : 0;     /* LTGT or ORDERED */
    results[idx++] = (inf == inf) ? 1 : 0;     /* EQ or ORDERED */
    results[idx++] = (inf != inf) ? 1 : 0;     /* NEQ or UNORDERED */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* LT or UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);        /* GT or UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);      /* LE or UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* GE or UNGE */
    
    /* 3. Complex expressions that may produce NaN */
    volatile double nan_prod = nan * inf;
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;
    results[idx++] = (inf_minus_inf > zero) ? 1 : 0;
    results[idx++] = (zero_div_zero <= one) ? 1 : 0;
    
    /* 4. Nested comparisons using ternary operators */
    results[idx++] = __builtin_isunordered(nan, inf) ? 
                     (__builtin_isless(nan, zero) ? 2 : 3) : 
                     (__builtin_isgreater(inf, zero) ? 4 : 5);
    
    /* 5. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;
    results[idx++] = ((double)ld_inf > nan) ? 1 : 0;
    
    /* Use all results to prevent optimization */
    for (int i = 0; i < idx; i++) {
        use_result(results[i]);
    }
}

/* Vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    typedef float v4sf __attribute__((vector_size(16)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v4sf vec_a = { __builtin_nanf(""), 1.0f, __builtin_inff(), -__builtin_inff() };
    v4sf vec_b = { 2.0f, __builtin_nanf(""), __builtin_inff(), 0.0f };
    
    /* Vector comparison - may generate multiple condition code checks */
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
    
    use_result(mask1 + mask2 + mask3);
    
    /* Additional vector operations that might generate condition codes */
    v4sf vec_c = vec_a + vec_b;
    v4sf vec_d = vec_a * vec_b;
    v4sf cmp_result4 = vec_c != vec_d;
    
    #ifdef __SSE__
    int mask4 = __builtin_ia32_movmskps(cmp_result4);
    use_result(mask4);
    #endif
}

/* Inline assembly with explicit condition codes */
static void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result_unord = 0, result_ord = 0, result_ltgt = 0;
    int result_uneq = 0, result_unlt = 0, result_unle = 0;
    
    /* Various inline assembly blocks using different x86 compare instructions */
    
    /* Test UNORDERED (setp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_unord)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* Test ORDERED (setnp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_ord)
        : "x" (c), "x" (d)
        : "al", "cc"
    );
    
    /* Test LTGT (setne) after ordered comparison */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_ltgt)
        : "x" (a), "x" (c)
        : "al", "cc"
    );
    
    /* Test UNEQ (sete) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_uneq)
        : "x" (b), "x" (b)  /* inf == inf */
        : "al", "cc"
    );
    
    /* Use the results */
    use_result(result_unord + result_ord + result_ltgt + result_uneq + 
               result_unlt + result_unle);
}

/* Control flow based on unordered comparisons */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, 1.0, 0.0, -inf};
    
    int switch_result = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Complex condition that may use various unordered codes */
        int cond = 0;
        
        if (__builtin_isunordered(values[i], nan)) {
            cond = 1;  /* UNORDERED */
        } else if (__builtin_islessgreater(values[i], inf)) {
            cond = 2;  /* LTGT */
        } else if (!__builtin_isless(values[i], 0.0)) {
            cond = 3;  /* UNLT or GE */
        } else if (__builtin_isgreaterequal(values[i], nan)) {
            cond = 4;  /* UNGE */
        }
        
        /* Switch on the condition code result */
        switch (cond) {
            case 1: switch_result += 10; break;  /* UNORDERED path */
            case 2: switch_result += 20; break;  /* LTGT path */
            case 3: switch_result += 30; break;  /* UNLT/GE path */
            case 4: switch_result += 40; break;  /* UNGE path */
            default: switch_result += 1; break;
        }
    }
    
    use_result(switch_result);
}

/* Math functions that may produce NaN results */
static void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA operations with NaN inputs */
    double fma_result = __builtin_fma(nan, inf, 1.0);
    double fma_result2 = __builtin_fma(inf, 0.0, -inf);
    
    /* Comparisons of math function results */
    int cmp1 = (fma_result == fma_result) ? 0 : 1;  /* May be UNEQ or UNORDERED */
    int cmp2 = (fma_result2 > 0.0) ? 1 : 0;         /* May be UNGT or ORDERED */
    int cmp3 = (fma_result <= fma_result2) ? 1 : 0; /* May be UNLE or ORDERED */
    
    /* Trigonometric functions with NaN */
    double sin_nan = __builtin_sin(nan);
    double cos_inf = __builtin_cos(inf);
    
    int cmp4 = (sin_nan == cos_inf) ? 1 : 0;  /* UNEQ or UNORDERED */
    
    use_result(cmp1 + cmp2 + cmp3 + cmp4);
}

int main(void) {
    printf("Testing i386 floating-point condition codes...\n");
    
    /* Run all test suites */
    test_scalar_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow();
    test_math_functions();
    
    /* Print result to prevent complete optimization */
    printf("Result checksum: %d\n", global_counter);
    
    return global_counter != 0 ? 0 : 1;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is for x86/x86-64 architectures only.\n");
    return 0;
}

#endif
