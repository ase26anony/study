/* test_fp_conditions.c - Trigger x86 floating-point unordered condition codes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Feature detection for x86 */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to create checksum to prevent optimization */
static volatile int checksum = 0;

/* Helper to use results */
static void use_result(int cond) {
    checksum ^= cond;
    if (checksum & 1) {
        /* Prevent tail call optimization */
        volatile int dummy = cond;
        (void)dummy;
    }
}

/* Test scalar unordered comparisons with operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double neg_inf = -__builtin_inf();
    
    /* These should generate various condition codes */
    if (nan < inf) use_result(1);        /* UNORDERED likely */
    if (nan == nan) use_result(2);       /* UNORDERED/UNEQ */
    if (inf != nan) use_result(3);       /* ORDERED/LTGT */
    if (nan <= zero) use_result(4);      /* UNORDERED/UNLE */
    if (nan >= neg_inf) use_result(5);   /* UNORDERED/UNGE */
    if (zero > nan) use_result(6);       /* UNORDERED/UNLT */
    if (neg_inf < nan) use_result(7);    /* UNORDERED/UNGT */
    
    /* Complex expressions that could produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    if (inf_minus_inf == nan) use_result(8);     /* UNORDERED/UNEQ */
    if (zero_div_zero != inf) use_result(9);     /* UNORDERED/LTGT */
    if (inf_minus_inf < zero) use_result(10);    /* UNORDERED/UNLT */
    if (zero_div_zero >= nan) use_result(11);    /* UNORDERED/UNGE */
}

/* Test GCC built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double x = 3.14159;
    volatile double y = 2.71828;
    
    /* Direct use of built-ins that map to condition codes */
    if (__builtin_isunordered(nan, inf)) use_result(12);      /* UNORDERED */
    if (!__builtin_isunordered(x, y)) use_result(13);         /* ORDERED */
    if (__builtin_islessgreater(nan, x)) use_result(14);      /* LTGT */
    if (__builtin_isless(nan, inf)) use_result(15);           /* UNLT */
    if (__builtin_isgreater(inf, nan)) use_result(16);        /* UNGT */
    if (__builtin_islessequal(x, nan)) use_result(17);        /* UNLE */
    if (__builtin_isgreaterequal(nan, y)) use_result(18);     /* UNGE */
    
    /* Nested built-ins to force multiple comparisons */
    int result = __builtin_isunordered(nan, inf) ? 
                 __builtin_isless(x, y) : 
                 __builtin_isgreater(nan, x);
    use_result(result);
    
    /* Ternary with built-ins */
    volatile double a = __builtin_isless(nan, x) ? nan : x;
    volatile double b = __builtin_isgreater(y, nan) ? y : nan;
    if (__builtin_isunordered(a, b)) use_result(19);
}

/* Test vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), 2.0f, 1.0f, __builtin_nanf("")};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result;
    
    cmp_result = vec_a > vec_b;  /* Should generate multiple UNGT/UNLT */
    cmp_result = vec_a == vec_b; /* Should generate UNEQ/UNORDERED */
    cmp_result = vec_a <= vec_b; /* Should generate UNLE */
    cmp_result = vec_a >= vec_b; /* Should generate UNGE */
    
    /* Extract comparison masks - forces actual comparison code gen */
    int mask;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask = __builtin_ia32_movmskps(cmp_result);
    use_result(mask);
    #endif
    
    /* Alternative: store to memory and check */
    float result_array[4];
    memcpy(result_array, &cmp_result, sizeof(result_array));
    for (int i = 0; i < 4; i++) {
        if (result_array[i] != 0.0f) use_result(20 + i);
    }
    
    /* Double precision vector */
    v2df vec_d1 = {__builtin_nan(""), __builtin_inf()};
    v2df vec_d2 = {__builtin_inf(), __builtin_nan("")};
    v2df vec_d3 = vec_d1 < vec_d2;  /* UNLT comparisons */
    
    double dbl_result[2];
    memcpy(dbl_result, &vec_d3, sizeof(dbl_result));
    if (dbl_result[0] != 0.0) use_result(24);
    if (dbl_result[1] != 0.0) use_result(25);
}

/* Test mixed-type comparisons and arithmetic */
static void test_mixed_types(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_inf = __builtin_inf();
    volatile long double ld_zero = 0.0L;
    
    /* Cross-type comparisons */
    if ((double)f_nan < d_inf) use_result(26);          /* UNLT */
    if (f_nan == (float)d_inf) use_result(27);          /* UNORDERED/UNEQ */
    if (ld_zero != (long double)f_nan) use_result(28);  /* ORDERED/LTGT */
    
    /* Arithmetic that produces NaN, then comparison */
    volatile double a = d_inf / ld_zero;      /* May produce NaN */
    volatile double b = d_inf - d_inf;        /* NaN */
    
    if (__builtin_fma(a, b, d_inf) == f_nan) use_result(29);  /* UNORDERED/UNEQ */
    if (a * b < ld_zero) use_result(30);                      /* UNLT */
    if (b >= a) use_result(31);                               /* UNGE */
    
    /* Complex expression tree */
    volatile double expr1 = (a + b) * (d_inf / a);
    volatile double expr2 = (b - a) / (f_nan * 2.0);
    if (expr1 > expr2) use_result(32);        /* UNGT */
    if (expr1 <= expr2) use_result(33);       /* UNLE */
}

/* Inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    use_result(result ? 34 : 35);
    
    /* fucomi with conditional move */
    double asm_result;
    asm volatile (
        "fucomi %2, %1\n\t"
        "fcmovnu %3, %1\n\t"    /* move if ordered */
        "fstp %0"
        : "=m"(asm_result)
        : "t"(c), "u"(a), "f"(d)
        : "cc"
    );
    use_result((int)asm_result);
    
    /* Multiple comparisons in one asm block */
    int flags1, flags2;
    asm volatile (
        "ucomisd %3, %2\n\t"
        "setp %0\n\t"
        "sete %1"
        : "=r"(flags1), "=r"(flags2)
        : "x"(b), "x"(a)
        : "cc"
    );
    use_result(flags1 * 36 + flags2 * 37);
}

/* Control flow based on unordered comparison results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, 1.0, 2.0, -inf};
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int condition_code = 0;
        
        /* Determine condition code through comparisons */
        if (__builtin_isunordered(values[i], nan)) condition_code |= 1;
        if (values[i] == values[i]) condition_code |= 2;
        if (values[i] != nan) condition_code |= 4;
        if (values[i] < inf) condition_code |= 8;
        if (values[i] > -inf) condition_code |= 16;
        
        switch (condition_code) {
            case 1:  /* UNORDERED only */
                use_result(38);
                break;
            case 2:  /* UNEQ */
                use_result(39);
                break;
            case 4:  /* ORDERED/LTGT */
                use_result(40);
                break;
            case 8:  /* UNLT */
                use_result(41);
                break;
            case 16: /* UNGT */
                use_result(42);
                break;
            case 12: /* UNLE (8|4) */
                use_result(43);
                break;
            case 20: /* UNGE (16|4) */
                use_result(44);
                break;
            default:
                use_result(45);
                break;
        }
    }
    
    /* Loop controlled by unordered comparison */
    volatile double x = nan;
    int count = 0;
    while (!__builtin_isunordered(x, inf) && count < 10) {
        use_result(46 + count);
        x = x / 2.0;
        count++;
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all test suites */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_types();
    test_inline_asm();
    test_control_flow();
    
    /* Print checksum to ensure all code executes */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}

#endif
