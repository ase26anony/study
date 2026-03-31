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

/* Function to accumulate results */
static void accumulate(int val) {
    checksum ^= val;
    checksum += 1;
}

/* Test function with various unordered comparisons */
static void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    /* These should generate UNORDERED/ORDERED condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan > inf) ? 2 : 0;      /* UNORDERED case */
    results[idx++] = (nan <= inf) ? 3 : 0;     /* UNORDERED case */
    results[idx++] = (nan >= inf) ? 4 : 0;     /* UNORDERED case */
    results[idx++] = (nan == nan) ? 5 : 0;     /* UNEQ case */
    results[idx++] = (inf != nan) ? 6 : 0;     /* LTGT case */
    results[idx++] = (nan != nan) ? 7 : 0;     /* ORDERED case (false) */
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to the condition codes */
    results[idx++] = __builtin_isunordered(nan, inf) ? 8 : 0;      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf) ? 9 : 0;    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf) ? 10 : 0;          /* UNLT */
    results[idx++] = __builtin_isgreater(nan, inf) ? 11 : 0;       /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf) ? 12 : 0;     /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf) ? 13 : 0;  /* UNGE */
    
    /* 3. Complex expressions with mixed types */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    volatile float f_inf = __builtin_inff();
    
    /* Mixed type comparisons */
    results[idx++] = (f_nan < (float)inf) ? 14 : 0;
    results[idx++] = ((double)ld_nan > f_inf) ? 15 : 0;
    
    /* Arithmetic that produces NaN */
    volatile double nan_prod = inf * zero;
    volatile double nan_diff = inf - inf;
    
    results[idx++] = (nan_prod == nan_prod) ? 16 : 0;  /* UNEQ */
    results[idx++] = (nan_diff != nan_diff) ? 17 : 0;  /* ORDERED */
    
    /* 4. Nested comparisons in control flow */
    if (__builtin_isunordered(nan, one) && !__builtin_isless(nan, one)) {
        results[idx++] = 18;  /* UNORDERED + !UNLT = UNGE? */
    } else {
        results[idx++] = 0;
    }
    
    /* Ternary operator with unordered comparison */
    results[idx++] = __builtin_islessgreater(inf, neg_inf) ? 19 : 20;
    
    /* Store all results to checksum */
    for (int i = 0; i < idx; i++) {
        accumulate(results[i]);
    }
}

/* Test function with vector comparisons */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {0.0f, __builtin_nanf(""), 2.0f, 3.0f};
    
    /* Vector comparisons - may generate multiple condition checks */
    v4sf cmp_result = vec_a > vec_b;      /* UNGT/UNLE cases */
    v4sf cmp_result2 = vec_a == vec_b;    /* UNEQ cases */
    v4sf cmp_result3 = vec_a != vec_b;    /* LTGT cases */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    #else
    /* Fallback: store to memory and check */
    float store1[4], store2[4], store3[4];
    memcpy(store1, &cmp_result, sizeof(cmp_result));
    memcpy(store2, &cmp_result2, sizeof(cmp_result2));
    memcpy(store3, &cmp_result3, sizeof(cmp_result3));
    mask1 = (store1[0] != 0) | ((store1[1] != 0) << 1) | 
            ((store1[2] != 0) << 2) | ((store1[3] != 0) << 3);
    mask2 = (store2[0] != 0) | ((store2[1] != 0) << 1) | 
            ((store2[2] != 0) << 2) | ((store2[3] != 0) << 3);
    mask3 = (store3[0] != 0) | ((store3[1] != 0) << 1) | 
            ((store3[2] != 0) << 2) | ((store3[3] != 0) << 3);
    #endif
    
    accumulate(mask1);
    accumulate(mask2);
    accumulate(mask3);
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {0.0, __builtin_nan("")};
    
    v2df d_cmp = vec_da > vec_db;
    v2df d_cmp2 = vec_da == vec_db;
    
    #ifdef __SSE2__
    int mask4 = __builtin_ia32_movmskpd(d_cmp);
    int mask5 = __builtin_ia32_movmskpd(d_cmp2);
    #else
    double dstore1[2], dstore2[2];
    memcpy(dstore1, &d_cmp, sizeof(d_cmp));
    memcpy(dstore2, &d_cmp2, sizeof(d_cmp2));
    mask4 = (dstore1[0] != 0) | ((dstore1[1] != 0) << 1);
    mask5 = (dstore2[0] != 0) | ((dstore2[1] != 0) << 1);
    #endif
    
    accumulate(mask4);
    accumulate(mask5);
}

/* Test with inline assembly */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -__builtin_inf();
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with explicit condition codes */
    /* These force the compiler to handle the condition mnemonics */
    
    /* Compare NaN with Inf - should set parity flag (unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Compare Inf with NaN - test different order */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result2)
        : "x"(b), "x"(a)
        : "cc"
    );
    
    /* Compare normal numbers */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(result3)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* Compare with fucomi instruction (x87) */
    #ifdef __i386__
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %0"
        : "=r"(result4)
        : "m"(a), "m"(b)
        : "cc"
    );
    #endif
    
    accumulate(result1);
    accumulate(result2);
    accumulate(result3);
    accumulate(result4);
}

/* Control flow based on unordered comparisons */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double vals[] = {1.0, -1.0, 0.0, __builtin_inf(), nan};
    int results = 0;
    
    /* Switch-like behavior using comparison results */
    for (int i = 0; i < 5; i++) {
        volatile double x = vals[i];
        
        /* Complex condition that uses multiple unordered comparisons */
        if (__builtin_isunordered(x, nan)) {
            results |= (1 << 0);  /* UNORDERED */
        }
        if (!__builtin_isless(x, 0.0) && !__builtin_isgreater(x, 0.0)) {
            results |= (1 << 1);  /* UNGE & UNLE -> UNEQ? */
        }
        if (__builtin_islessgreater(x, 0.0)) {
            results |= (1 << 2);  /* LTGT */
        }
        
        /* Nested ternary with unordered comparison */
        int val = __builtin_isunordered(x, 0.0) ? 1 : 
                 (__builtin_isless(x, 0.0) ? 2 : 
                 (__builtin_isgreater(x, 0.0) ? 3 : 4));
        results ^= val;
    }
    
    accumulate(results);
    
    /* Loop controlled by unordered comparison */
    volatile double test_val = nan;
    int counter = 0;
    
    while (counter < 10) {
        /* Break condition based on ordered comparison */
        if (!__builtin_isunordered(test_val, (double)counter)) {
            break;
        }
        counter++;
        accumulate(counter);
    }
}

/* Main test function */
int main(void) {
    /* Only run x86-specific tests on x86 */
    #ifdef __x86_64__ || __i386__
    printf("Running x86 floating-point comparison tests...\n");
    
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm();
    test_control_flow();
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
    #else
    /* Non-x86 fallback */
    printf("Not an x86 target - skipping tests\n");
    return 0;
    #endif
}

#else
/* Non-x86 version */
int main(void) {
    printf("Not an x86 target - skipping tests\n");
    return 0;
}
#endif
