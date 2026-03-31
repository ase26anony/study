/* test_fp_conditions.c - Trigger x86 floating-point unordered condition codes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int global_counter = 0;
static void use_result(int val) {
    global_counter += val;
}

/* Test scalar unordered comparisons with operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[16] = {0};
    int idx = 0;
    
    /* These should generate various condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED likely */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (nan > zero) ? 1 : 0;     /* UNORDERED */
    results[idx++] = (inf <= nan) ? 1 : 0;     /* UNORDERED/UNLE */
    results[idx++] = (nan >= neg_inf) ? 1 : 0; /* UNORDERED/UNGE */
    
    /* Arithmetic that produces NaN */
    volatile double nan_prod = inf * zero;
    volatile double nan_sub = inf - inf;
    
    results[idx++] = (nan_prod == 1.0) ? 1 : 0;  /* UNORDERED */
    results[idx++] = (nan_sub < inf) ? 1 : 0;    /* UNORDERED */
    results[idx++] = (1.0 > nan_prod) ? 1 : 0;   /* UNORDERED */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    
    results[idx++] = (f_nan == (float)nan) ? 1 : 0;
    results[idx++] = (ld_inf > nan) ? 1 : 0;
    
    /* Complex expressions */
    results[idx++] = ((nan + inf) < zero) ? 1 : 0;
    results[idx++] = ((inf / zero) == nan) ? 1 : 0;
    
    /* Control flow based on unordered results */
    if (nan == nan) {  /* Always false with NaN */
        results[idx++] = 100;
    } else {
        results[idx++] = 200;  /* This path should be taken */
    }
    
    /* Ternary operator with unordered comparison */
    results[idx++] = (inf != nan) ? 300 : 400;
    
    /* Combine results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum ^= results[i];
    }
    use_result(sum);
}

/* Test GCC built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    int results[12] = {0};
    int idx = 0;
    
    /* Direct mappings to condition codes */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, nan);  /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);         /* UNORDERED/UNLT */
    results[idx++] = __builtin_isgreater(inf, nan);      /* ORDERED/UNGT? */
    results[idx++] = __builtin_islessequal(zero, nan);   /* UNORDERED/UNLE */
    results[idx++] = __builtin_isgreaterequal(inf, nan); /* ORDERED/UNGE? */
    
    /* Nested built-ins to force multiple comparisons */
    if (__builtin_isunordered(nan, zero) && 
        __builtin_islessgreater(inf, zero)) {
        results[idx++] = 1;
    }
    
    /* Built-ins in ternary expressions */
    results[idx++] = __builtin_isunordered(nan, nan) ? 2 : 3;
    results[idx++] = __builtin_islessgreater(inf, inf) ? 4 : 5;
    
    /* Combined with arithmetic */
    volatile double a = inf * zero;  /* NaN */
    volatile double b = inf / inf;   /* NaN */
    results[idx++] = __builtin_isunordered(a, b) ? 6 : 7;
    results[idx++] = __builtin_islessgreater(a, 1.0) ? 8 : 9;
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    use_result(sum);
}

/* Test vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    /* Initialize vectors with NaN and normal values */
    v4sf vec_nan = (v4sf){__builtin_nanf(""), 1.0f, __builtin_nanf(""), 3.0f};
    v4sf vec_inf = (v4sf){__builtin_inff(), 2.0f, __builtin_inff(), 4.0f};
    v4sf vec_zero = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Perform vector comparisons - these may generate multiple condition checks */
    v4sf cmp_gt = vec_inf > vec_nan;    /* Element-wise UNORDERED/UNGT? */
    v4sf cmp_lt = vec_nan < vec_inf;    /* Element-wise UNORDERED/UNLT? */
    v4sf cmp_eq = vec_nan == vec_nan;   /* Element-wise UNORDERED/UNEQ? */
    v4sf cmp_ne = vec_inf != vec_nan;   /* Element-wise ORDERED/LTGT? */
    
    /* Extract comparison results to force code generation */
    int mask_gt, mask_lt, mask_eq, mask_ne;
    
    /* Use SSE movmskps intrinsic if available */
    #ifdef __SSE__
    mask_gt = __builtin_ia32_movmskps((__v4sf)cmp_gt);
    mask_lt = __builtin_ia32_movmskps((__v4sf)cmp_lt);
    mask_eq = __builtin_ia32_movmskps((__v4sf)cmp_eq);
    mask_ne = __builtin_ia32_movmskps((__v4sf)cmp_ne);
    #else
    /* Fallback: store to memory and check */
    float store_gt[4], store_lt[4], store_eq[4], store_ne[4];
    memcpy(store_gt, &cmp_gt, sizeof(cmp_gt));
    memcpy(store_lt, &cmp_lt, sizeof(cmp_lt));
    memcpy(store_eq, &cmp_eq, sizeof(cmp_eq));
    memcpy(store_ne, &cmp_ne, sizeof(cmp_ne));
    
    mask_gt = (store_gt[0] != 0) | ((store_gt[1] != 0) << 1) |
              ((store_gt[2] != 0) << 2) | ((store_gt[3] != 0) << 3);
    mask_lt = (store_lt[0] != 0) | ((store_lt[1] != 0) << 1) |
              ((store_lt[2] != 0) << 2) | ((store_lt[3] != 0) << 3);
    mask_eq = (store_eq[0] != 0) | ((store_eq[1] != 0) << 1) |
              ((store_eq[2] != 0) << 2) | ((store_eq[3] != 0) << 3);
    mask_ne = (store_ne[0] != 0) | ((store_ne[1] != 0) << 1) |
              ((store_ne[2] != 0) << 2) | ((store_ne[3] != 0) << 3);
    #endif
    
    /* Double precision vector tests */
    v2df vec_dbl_nan = (v2df){__builtin_nan(""), 2.0};
    v2df vec_dbl_inf = (v2df){__builtin_inf(), 3.0};
    
    v2df cmp_dbl_ge = vec_dbl_inf >= vec_dbl_nan;  /* UNORDERED/UNGE? */
    v2df cmp_dbl_le = vec_dbl_nan <= vec_dbl_inf;  /* UNORDERED/UNLE? */
    
    /* Use results to prevent elimination */
    use_result(mask_gt ^ mask_lt ^ mask_eq ^ mask_ne);
    
    #ifdef __SSE2__
    int mask_dbl_ge = __builtin_ia32_movmskpd((__v2df)cmp_dbl_ge);
    int mask_dbl_le = __builtin_ia32_movmskpd((__v2df)cmp_dbl_le);
    use_result(mask_dbl_ge + mask_dbl_le);
    #endif
}

/* Test inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    volatile double d = 1.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* ucomisd with seta (above, for ordered greater) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %b0"
        : "=r"(result2)
        : "x"(b), "x"(c)  /* inf > 0 */
        : "cc"
    );
    
    /* fucomi instruction (x87) */
    #ifdef __i386__
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %b0"
        : "=r"(result3)
        : "m"(a), "m"(d)  /* NaN vs 1.0 */
        : "cc"
    );
    #endif
    
    /* Combined comparison with multiple condition codes */
    asm volatile (
        "ucomisd %3, %2\n\t"
        "jp 1f\n\t"          /* Jump if unordered */
        "ucomisd %4, %2\n\t"
        "setne %b0\n\t"      /* Set if not equal (ordered) */
        "jmp 2f\n\t"
        "1:\n\t"
        "mov $1, %0\n\t"     /* Result for unordered */
        "2:"
        : "=r"(result4)
        : "0"(0), "x"(a), "x"(b), "x"(c)
        : "cc"
    );
    
    use_result(result1 + result2 + result3 + result4);
}

/* Test control flow driven by unordered comparisons */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 0.0, 1.0, -inf};
    
    int switch_result = 0;
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int condition_code = 0;
        
        /* Determine condition code through comparisons */
        if (__builtin_isunordered(vals[i], nan)) {
            condition_code = 1;  /* UNORDERED */
        } else if (vals[i] == vals[i]) {
            condition_code = 2;  /* EQ (ordered equal) */
        } else if (vals[i] != vals[(i+1)%5]) {
            condition_code = 3;  /* NEQ/LTGT */
        } else if (vals[i] < vals[(i+2)%5]) {
            condition_code = 4;  /* LT/UNLT */
        } else if (vals[i] > vals[(i+3)%5]) {
            condition_code = 5;  /* GT/UNGT */
        } else if (vals[i] <= vals[(i+4)%5]) {
            condition_code = 6;  /* LE/UNLE */
        } else {
            condition_code = 7;  /* GE/UNGE */
        }
        
        switch (condition_code) {
            case 1: switch_result += 1; break;  /* UNORDERED */
            case 2: switch_result += 2; break;  /* EQ */
            case 3: switch_result += 4; break;  /* NEQ/LTGT */
            case 4: switch_result += 8; break;  /* LT/UNLT */
            case 5: switch_result += 16; break; /* GT/UNGT */
            case 6: switch_result += 32; break; /* LE/UNLE */
            case 7: switch_result += 64; break; /* GE/UNGE */
        }
    }
    
    /* Loop with unordered comparison as condition */
    int loop_count = 0;
    volatile double x = nan;
    while (__builtin_islessgreater(x, 0.0) && loop_count < 10) {
        /* This loop may or may not execute depending on NaN behavior */
        loop_count++;
        x = (loop_count % 2 == 0) ? __builtin_nan("") : __builtin_inf();
    }
    
    use_result(switch_result + loop_count);
}

/* Test math functions with NaN inputs */
static void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(nan, inf, 1.0);
    #else
    volatile double fma_result = nan * inf + 1.0;
    #endif
    
    /* Comparisons after math operations */
    int results = 0;
    results |= (fma_result == fma_result) ? 0x1 : 0;
    results |= (fma_result < inf) ? 0x2 : 0;
    results |= (fma_result > -inf) ? 0x4 : 0;
    
    /* Division producing NaN */
    volatile double div_zero = inf / 0.0;  /* Should be NaN */
    volatile double inf_minus_inf = inf - inf;  /* NaN */
    
    results |= __builtin_isunordered(div_zero, inf_minus_inf) ? 0x8 : 0;
    results |= __builtin_islessgreater(div_zero, 0.0) ? 0x10 : 0;
    
    /* Complex expression tree */
    volatile double expr1 = (nan + inf) * 0.0;
    volatile double expr2 = (inf * 0.0) / 1.0;
    
    if (__builtin_isunordered(expr1, expr2)) {
        results |= 0x20;
    }
    if (expr1 != expr2) {  /* Both NaN, so not equal */
        results |= 0x40;
    }
    
    use_result(results);
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all test functions */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_inline_asm();
    test_control_flow();
    test_math_functions();
    
    /* Print something to ensure execution */
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter != 0 ? 0 : 1;
}

#else /* Non-x86 target */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}
#endif
