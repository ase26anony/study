#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __x86_64__ || __i386__

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function that performs various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED/UNLT */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED/UNGT */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED/UNLE */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED/UNGE */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (nan != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/NE */
    
    /* Mixed NaN comparisons */
    results[idx++] = (nan < one) ? 1 : 0;
    results[idx++] = (one > nan) ? 1 : 0;
    results[idx++] = (nan <= zero) ? 1 : 0;
    results[idx++] = (zero >= nan) ? 1 : 0;
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(one, nan);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_islessgreater(one, nan);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isless(one, nan);           /* UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNGT */
    results[idx++] = __builtin_isgreater(one, nan);        /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_islessequal(one, nan);      /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNGE */
    results[idx++] = __builtin_isgreaterequal(one, nan);   /* UNGE */
    
    /* 3. Complex expressions that could produce NaN */
    volatile double nan_prod = zero / zero;
    volatile double inf_minus_inf = inf - inf;
    
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;       /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != inf_minus_inf) ? 1 : 0; /* ORDERED/LTGT */
    
    /* Arithmetic with NaN */
    volatile double expr1 = (nan + one) * inf;
    volatile double expr2 = (inf * zero) / neg_one;
    
    results[idx++] = (expr1 < expr2) ? 1 : 0;
    results[idx++] = (expr1 > expr2) ? 1 : 0;
    results[idx++] = (expr1 <= expr2) ? 1 : 0;
    results[idx++] = (expr1 >= expr2) ? 1 : 0;
    
    /* Control flow based on unordered comparisons */
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(nan, (double)i)) {
            global_counter++;
        }
        if (!__builtin_isunordered(one, (double)i)) {
            global_counter--;
        }
    }
    
    /* Switch statement driven by comparison results */
    int switch_val = 0;
    switch_val |= __builtin_isunordered(nan, inf) ? 1 : 0;
    switch_val |= __builtin_islessgreater(one, nan) ? 2 : 0;
    switch_val |= __builtin_isless(nan, one) ? 4 : 0;
    
    switch (switch_val) {
        case 0: global_counter += 1; break;
        case 1: global_counter += 2; break;  /* UNORDERED */
        case 2: global_counter += 3; break;  /* LTGT */
        case 3: global_counter += 4; break;  /* UNORDERED | LTGT */
        case 4: global_counter += 5; break;  /* UNLT */
        case 5: global_counter += 6; break;  /* UNORDERED | UNLT */
        case 6: global_counter += 7; break;  /* LTGT | UNLT */
        case 7: global_counter += 8; break;  /* All three */
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        global_counter += results[i];
    }
}

/* Test function for vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(),
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_one = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_zero = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp1 = vec_nan > vec_inf;      /* UNORDERED/UNGT */
    v4sf cmp2 = vec_nan < vec_one;      /* UNORDERED/UNLT */
    v4sf cmp3 = vec_nan == vec_nan;     /* UNORDERED/UNEQ */
    v4sf cmp4 = vec_one != vec_nan;     /* ORDERED/NE */
    
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
    
    global_counter += mask1 + mask2 + mask3 + mask4;
    
    /* Double precision vector comparisons */
    v2df vec_dnan = (v2df){__builtin_nan(""), __builtin_nan("")};
    v2df vec_dinf = (v2df){__builtin_inf(), __builtin_inf()};
    
    v2df dcmp1 = vec_dnan > vec_dinf;
    v2df dcmp2 = vec_dnan == vec_dnan;
    
    #ifdef __SSE2__
    int dmask1 = __builtin_ia32_movmskpd(dcmp1);
    int dmask2 = __builtin_ia32_movmskpd(dcmp2);
    global_counter += dmask1 + dmask2;
    #endif
}

/* Test function with inline assembly */
void test_inline_asm_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with explicit condition codes */
    #ifdef __x86_64__
    /* Compare NaN with Inf - should set UNORDERED */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0\n\t"
        "sete %b1\n\t"
        : "=r"(result1), "=r"(result2)
        : "x"(b), "x"(a)
        : "cc"
    );
    
    /* Compare 1.0 with NaN - should set UNORDERED */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %b0\n\t"    /* set if above (greater and not unordered) */
        "setb %b1\n\t"    /* set if below (less and not unordered) */
        : "=r"(result3), "=r"(result4)
        : "x"(a), "x"(c)
        : "cc"
    );
    #else
    /* 32-bit version */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %b0\n\t"
        : "=r"(result1)
        : "0"(result1), "m"(b), "m"(a)
        : "cc"
    );
    #endif
    
    global_counter += result1 + result2 + result3 + result4;
    
    /* Test with fucomi instruction */
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    int ld_result = 0;
    
    #ifdef __x86_64__
    asm volatile (
        "fucomi %2, %1\n\t"
        "setp %b0\n\t"
        : "=r"(ld_result)
        : "t"(ld_nan), "u"(ld_inf)
        : "cc"
    );
    #endif
    
    global_counter += ld_result;
}

/* Test mixed-type comparisons */
void test_mixed_type_comparisons(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    int results[12];
    int idx = 0;
    
    /* Cross-type comparisons */
    results[idx++] = (f_nan < d_inf) ? 1 : 0;
    results[idx++] = (d_nan > f_inf) ? 1 : 0;
    results[idx++] = (ld_nan == d_nan) ? 1 : 0;
    results[idx++] = (f_inf != ld_nan) ? 1 : 0;
    
    /* After arithmetic operations */
    volatile float f_expr = f_inf / f_nan;
    volatile double d_expr = d_inf * d_nan;
    volatile long double ld_expr = ld_inf - ld_nan;
    
    results[idx++] = (f_expr < d_expr) ? 1 : 0;
    results[idx++] = (d_expr > ld_expr) ? 1 : 0;
    results[idx++] = (ld_expr == f_expr) ? 1 : 0;
    
    /* Use FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(d_nan, d_inf, 1.0);
    results[idx++] = (fma_result == fma_result) ? 1 : 0;
    results[idx++] = (fma_result < 0.0) ? 1 : 0;
    #endif
    
    /* Complex conditional expression */
    int complex_result = (f_nan < d_inf) ? 
                         (__builtin_isunordered(d_nan, ld_nan) ? 1 : 2) :
                         (__builtin_islessgreater(f_inf, f_nan) ? 3 : 4);
    
    results[idx++] = complex_result;
    
    /* Nested ternary with unordered comparisons */
    double test_val = __builtin_isunordered(f_nan, d_nan) ? 
                     (__builtin_isless(d_nan, ld_inf) ? 1.0 : 2.0) :
                     (__builtin_isgreaterequal(ld_nan, f_inf) ? 3.0 : 4.0);
    
    results[idx++] = (test_val > 0.0) ? 1 : 0;
    
    for (int i = 0; i < idx; i++) {
        global_counter += results[i];
    }
}

int main(void) {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm_comparisons();
    test_mixed_type_comparisons();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is designed for x86 architecture only.\n");
    return 0;
}
#endif
