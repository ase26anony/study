#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to accumulate results without optimization removal */
static void accumulate(int val) {
    checksum ^= val;
    checksum += 1;
}

/* Test 1: Direct unordered comparisons with operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf) accumulate(1);  /* UNORDERED case */
    if (nan == nan) accumulate(2); /* UNEQ case */
    if (inf != nan) accumulate(3); /* LTGT case */
    if (nan <= inf) accumulate(4); /* UNLE case */
    if (nan >= inf) accumulate(5); /* UNGE case */
    if (nan > inf) accumulate(6);  /* UNGT case */
    if (!(nan < inf)) accumulate(7); /* ORDERED case (inverted) */
    
    /* More complex expressions */
    volatile double result = (inf - inf); /* Creates NaN */
    if (result == result) accumulate(8); /* Should be false for NaN */
    if (result != result) accumulate(9); /* Should be true for NaN */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    if (f_nan < ld_inf) accumulate(10);
    if (f_nan == f_nan) accumulate(11);
}

/* Test 2: Built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* Direct use of built-ins that map to condition codes */
    if (__builtin_isunordered(nan, inf)) accumulate(100); /* UNORDERED */
    if (__builtin_islessgreater(nan, inf)) accumulate(101); /* LTGT */
    if (__builtin_isless(nan, inf)) accumulate(102); /* UNLT */
    if (__builtin_isgreater(nan, inf)) accumulate(103); /* UNGT */
    if (__builtin_islessequal(nan, inf)) accumulate(104); /* UNLE */
    if (__builtin_isgreaterequal(nan, inf)) accumulate(105); /* UNGE */
    
    /* Nested built-ins in ternary expressions */
    int res1 = __builtin_isunordered(zero, nan) ? 1 : 0;
    int res2 = __builtin_islessgreater(inf, nan) ? 2 : 0;
    int res3 = __builtin_isless(nan, zero) ? 3 : 0;
    
    accumulate(res1 + res2 + res3);
    
    /* Complex expression with multiple built-ins */
    if (__builtin_isunordered(nan, inf) && 
        !__builtin_islessgreater(zero, zero)) {
        accumulate(106);
    }
}

/* Test 3: Vector comparisons with GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result = vec_a > vec_b;  /* Should involve UNGT/UNLT */
    v4sf cmp_result2 = vec_a == vec_b; /* Should involve UNEQ */
    v4sf cmp_result3 = vec_a <= vec_b; /* Should involve UNLE */
    
    /* Extract comparison masks - forces actual comparison code gen */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    int mask2 = __builtin_ia32_movmskps(cmp_result2);
    int mask3 = __builtin_ia32_movmskps(cmp_result3);
    
    accumulate(mask1 + mask2 + mask3);
    
    /* Store to memory and check individual elements */
    float result_array[4];
    memcpy(result_array, &cmp_result, sizeof(result_array));
    
    for (int i = 0; i < 4; i++) {
        if (result_array[i] != 0.0f) {
            accumulate(200 + i);
        }
    }
    
    /* Double precision vector comparisons */
    v2df vec_d1 = {__builtin_nan(""), __builtin_inf()};
    v2df vec_d2 = {__builtin_inf(), __builtin_nan("")};
    v2df cmp_d = vec_d1 < vec_d2;  /* UNORDERED/UNLT */
    
    double d_result[2];
    memcpy(d_result, &cmp_d, sizeof(d_result));
    if (d_result[0] == 0.0) accumulate(210);
    if (d_result[1] == 0.0) accumulate(211);
}

/* Test 4: Mixed-type comparisons and arithmetic */
static void test_mixed_operations(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    /* Arithmetic that produces NaN */
    volatile double nan1 = inf / inf;
    volatile double nan2 = inf - inf;
    volatile double nan3 = 0.0 / 0.0;
    
    /* Comparisons after arithmetic */
    if (nan1 < nan2) accumulate(300); /* UNORDERED */
    if (nan1 == nan3) accumulate(301); /* UNEQ */
    if (inf != nan1) accumulate(302); /* LTGT */
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, inf, 1.0);
    if (fma_result == fma_result) accumulate(303);
    
    /* Mixed precision comparisons */
    if ((double)f_inf > nan) accumulate(304);
    if ((float)ld_nan == (float)ld_nan) accumulate(305);
    
    /* Complex conditional expression */
    int complex_result = (nan < inf) ? 
                         (nan == nan ? 1 : 2) : 
                         (inf != nan ? 3 : 4);
    accumulate(complex_result);
}

/* Test 5: Inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* fucomi with conditional moves */
    double asm_result = 0.0;
    asm volatile (
        "fucomi %2, %1\n\t"
        "fcmovnu %3, %0"
        : "=t"(asm_result)
        : "t"(a), "u"(b), "t"(c)
        : "cc"
    );
    
    /* Multiple comparisons in sequence */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %b0\n\t"
        "ucomisd %4, %3\n\t"
        "setp %b2"
        : "=r"(result2), "=r"(result3)
        : "x"(c), "x"(d), "x"(a), "0"(result2), "2"(result3)
        : "cc"
    );
    
    accumulate(result1 + (int)asm_result + result2 + result3);
}

/* Test 6: Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 0.0, 1.0, -1.0};
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int condition_code = 0;
        
        /* Determine condition code through comparisons */
        if (__builtin_isunordered(vals[i], nan)) condition_code = 1; /* UNORDERED */
        else if (__builtin_islessgreater(vals[i], inf)) condition_code = 2; /* LTGT */
        else if (!__builtin_isless(vals[i], vals[(i+1)%5])) condition_code = 3; /* UNGE */
        else if (__builtin_isgreaterequal(vals[i], vals[(i+2)%5])) condition_code = 4; /* UNGE */
        
        switch (condition_code) {
            case 1: accumulate(400); break; /* UNORDERED path */
            case 2: accumulate(401); break; /* LTGT path */
            case 3: accumulate(402); break; /* UNGE path */
            case 4: accumulate(403); break; /* Another UNGE path */
            default: accumulate(404); break;
        }
    }
    
    /* Loop controlled by unordered comparison */
    int counter = 0;
    while (__builtin_isunordered(nan, vals[counter % 5]) && counter < 10) {
        accumulate(410 + counter);
        counter++;
    }
    
    /* Conditional function calls */
    void (*funcs[4])(void) = {
        test_scalar_operators,
        test_builtin_functions,
        test_vector_comparisons,
        test_mixed_operations
    };
    
    int idx = __builtin_islessgreater(nan, inf) ? 0 : 
              __builtin_isunordered(inf, nan) ? 1 : 2;
    if (idx < 4) {
        accumulate(420);
    }
}

int main(void) {
#if defined(__x86_64__) || defined(__i386__)
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_operations();
    test_inline_asm();
    test_control_flow();
    
    /* Print checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
#else
    printf("Non-x86 target - minimal test\n");
    volatile double x = 1.0;
    volatile double y = 2.0;
    if (x < y) {
        printf("Simple comparison worked\n");
    }
    return 0;
#endif
}
