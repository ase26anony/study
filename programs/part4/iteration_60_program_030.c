#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to create a checksum to prevent optimization */
static volatile int checksum = 0;

/* Force side effects to prevent optimization */
static void use_result(int result) {
    checksum ^= result;
}

/* Test scalar unordered comparisons with operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf) use_result(1);      /* UNORDERED? */
    if (nan > inf) use_result(2);      /* UNORDERED? */
    if (nan <= inf) use_result(3);     /* UNORDERED? */
    if (nan >= inf) use_result(4);     /* UNORDERED? */
    if (nan == nan) use_result(5);     /* UNEQ or UNORDERED? */
    if (nan != nan) use_result(6);     /* LTGT or ORDERED? */
    if (inf == inf) use_result(7);     /* EQ */
    if (inf != inf) use_result(8);     /* UNORDERED? */
    
    /* Arithmetic that may produce NaN */
    volatile double nan_result = inf - inf;
    if (nan_result == nan_result) use_result(9);
    if (nan_result != nan_result) use_result(10);
    
    /* Mixed comparisons */
    if (zero < nan) use_result(11);
    if (nan < zero) use_result(12);
    if (one > nan) use_result(13);
    if (nan > one) use_result(14);
}

/* Test built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* These built-ins map directly to condition codes */
    if (__builtin_isunordered(nan, inf)) use_result(101);      /* UNORDERED */
    if (__builtin_islessgreater(nan, inf)) use_result(102);    /* LTGT */
    if (__builtin_isless(nan, inf)) use_result(103);           /* UNLT? */
    if (__builtin_isgreater(nan, inf)) use_result(104);        /* UNGT? */
    if (__builtin_islessequal(nan, inf)) use_result(105);      /* UNLE? */
    if (__builtin_isgreaterequal(nan, inf)) use_result(106);   /* UNGE? */
    
    /* Nested in ternary operators */
    int result1 = __builtin_isunordered(zero, nan) ? 1 : 0;
    int result2 = __builtin_islessgreater(zero, zero) ? 2 : 0;
    int result3 = __builtin_isless(nan, zero) ? 3 : 0;
    int result4 = __builtin_isgreater(zero, nan) ? 4 : 0;
    
    use_result(result1 + result2 + result3 + result4);
    
    /* Complex expression with built-ins */
    if (__builtin_isunordered(nan, nan) && __builtin_islessgreater(inf, zero)) {
        use_result(107);
    }
}

/* Test vector comparisons with GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {0.0f, __builtin_nanf(""), 2.0f, __builtin_inff()};
    v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result;
    
    /* Various comparison operations */
    cmp_result = vec_a > vec_b;  /* May generate UNGT/UNLT conditions */
    cmp_result = vec_a < vec_b;  /* May generate UNLT/UNGT conditions */
    cmp_result = vec_a == vec_b; /* May generate UNEQ conditions */
    cmp_result = vec_a != vec_b; /* May generate LTGT conditions */
    
    /* Extract comparison masks */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    use_result(mask1);
    
    /* Double precision vector comparisons */
    v2df vec_d1 = {__builtin_nan(""), __builtin_inf()};
    v2df vec_d2 = {0.0, __builtin_nan("")};
    v2df cmp_d = vec_d1 > vec_d2;
    
    /* Store to memory to force computation */
    volatile float mem_store[4];
    __builtin_memcpy((void*)mem_store, (void*)&cmp_result, sizeof(cmp_result));
    use_result((int)mem_store[0]);
}

/* Test mixed-type comparisons and arithmetic */
static void test_mixed_types(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    /* Cross-type comparisons */
    if (f_nan < d_nan) use_result(201);
    if (d_nan > ld_nan) use_result(202);
    if (ld_nan == f_nan) use_result(203);
    if (f_inf != d_inf) use_result(204);
    
    /* Arithmetic that produces NaN */
    volatile double div_zero = d_inf / 0.0;
    volatile double inf_minus_inf = d_inf - d_inf;
    volatile double sqrt_neg = __builtin_sqrt(-1.0);
    
    /* Compare results of NaN-producing arithmetic */
    if (div_zero == d_nan) use_result(205);
    if (inf_minus_inf != sqrt_neg) use_result(206);
    
    /* Use FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(d_nan, d_inf, 1.0);
    if (fma_result > 0.0) use_result(207);
    if (__builtin_isunordered(fma_result, d_nan)) use_result(208);
}

/* Test inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    volatile double d = 1.0;
    
    int result_p, result_z, result_c;
    
    /* ucomisd with NaN - should set parity flag (unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result_p)
        : "x"(a), "x"(b)
        : "cc"
    );
    use_result(result_p);
    
    /* fucomi with normal numbers */
    asm volatile (
        "fucomi %2, %1\n\t"
        "setz %0"
        : "=r"(result_z)
        : "t"(c), "u"(d)
        : "cc"
    );
    use_result(result_z);
    
    /* Compare and set multiple condition codes */
    asm volatile (
        "ucomisd %3, %2\n\t"
        "seta %0\n\t"
        "setb %1"
        : "=r"(result_c), "=r"(result_p)
        : "x"(b), "x"(c)
        : "cc"
    );
    use_result(result_c + result_p);
}

/* Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {0.0, 1.0, nan, inf, -inf};
    
    int result = 0;
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int cmp_result = 0;
        
        /* Perform comparison that may generate various condition codes */
        if (__builtin_isunordered(vals[i], nan)) {
            cmp_result = 1;  /* UNORDERED */
        } else if (__builtin_islessgreater(vals[i], inf)) {
            cmp_result = 2;  /* LTGT */
        } else if (vals[i] == vals[i]) {
            cmp_result = 3;  /* EQ or UNEQ */
        } else if (vals[i] != vals[i]) {
            cmp_result = 4;  /* NEQ or LTGT */
        }
        
        /* Nested switch to force code generation */
        switch (cmp_result) {
            case 1: result += 10; break;  /* UNORDERED path */
            case 2: result += 20; break;  /* LTGT path */
            case 3: result += 30; break;  /* EQ/UNEQ path */
            case 4: result += 40; break;  /* NEQ path */
            default: result += 50; break;
        }
    }
    
    use_result(result);
    
    /* Loop controlled by unordered comparison */
    int count = 0;
    volatile double test_val = nan;
    while (__builtin_isunordered(test_val, test_val) && count < 3) {
        use_result(300 + count);
        count++;
        test_val = (count % 2 == 0) ? nan : inf;
    }
}

int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    /* Run all tests */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_types();
    test_inline_asm();
    test_control_flow();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

#else /* Non-x86 target fallback */

int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}

#endif
