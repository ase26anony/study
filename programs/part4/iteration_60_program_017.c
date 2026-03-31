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

/* Function to accumulate results */
static void accumulate(int val) {
    checksum ^= val;
    checksum += 1;
}

/* Test scalar unordered comparisons with operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf) accumulate(1);      /* UNORDERED? */
    if (nan == nan) accumulate(2);     /* UNORDERED/UNEQ? */
    if (inf != nan) accumulate(3);     /* ORDERED/LTGT? */
    if (nan > zero) accumulate(4);     /* UNORDERED? */
    if (inf >= nan) accumulate(5);     /* UNORDERED/UNGE? */
    if (nan <= one) accumulate(6);     /* UNORDERED/UNLE? */
    
    /* More complex expressions */
    volatile double inf_minus_inf = inf - inf;  /* NaN */
    if (inf_minus_inf == zero) accumulate(7);
    if (inf_minus_inf != inf_minus_inf) accumulate(8);  /* UNORDERED */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    if (f_nan < (float)ld_inf) accumulate(9);
    if ((double)f_nan == nan) accumulate(10);
}

/* Test built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* Direct use of built-ins that map to condition codes */
    if (__builtin_isunordered(nan, inf)) accumulate(11);      /* UNORDERED */
    if (!__builtin_isunordered(inf, zero)) accumulate(12);    /* ORDERED */
    if (__builtin_islessgreater(nan, inf)) accumulate(13);    /* LTGT */
    if (__builtin_isless(nan, inf)) accumulate(14);           /* UNLT? */
    if (__builtin_isgreater(inf, nan)) accumulate(15);        /* UNGT? */
    if (__builtin_islessequal(zero, nan)) accumulate(16);     /* UNLE? */
    if (__builtin_isgreaterequal(inf, zero)) accumulate(17);  /* UNGE? */
    
    /* Nested built-ins in ternary expressions */
    int result = __builtin_isunordered(nan, nan) ? 
                 (__builtin_islessgreater(inf, zero) ? 1 : 2) : 
                 (__builtin_isless(nan, zero) ? 3 : 4);
    accumulate(result);
    
    /* Chained comparisons */
    if (__builtin_isunordered(nan, inf) && __builtin_islessgreater(zero, inf)) {
        accumulate(18);
    }
}

/* Test vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {__builtin_inff(), __builtin_nanf(""), 1.0f, -1.0f};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result = vec_a > vec_b;      /* Element-wise greater-than */
    v4sf cmp_result2 = vec_a == vec_b;    /* Element-wise equality */
    v4sf cmp_result3 = vec_a <= vec_b;    /* Element-wise less-or-equal */
    
    /* Extract comparison masks - forces scalarization */
    int mask1, mask2, mask3;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    #else
    /* Fallback: store to memory and check */
    float store[4];
    memcpy(store, &cmp_result, sizeof(store));
    mask1 = (store[0] != 0.0f) | ((store[1] != 0.0f) << 1) |
            ((store[2] != 0.0f) << 2) | ((store[3] != 0.0f) << 3);
    #endif
    
    accumulate(mask1);
    accumulate(mask2);
    accumulate(mask3);
    
    /* Double precision vector comparisons */
    v2df dvec_a = {__builtin_nan(""), __builtin_inf()};
    v2df dvec_b = {__builtin_inf(), __builtin_nan("")};
    v2df dvec_cmp = dvec_a < dvec_b;
    
    #ifdef __SSE2__
    int dmask = __builtin_ia32_movmskpd(dvec_cmp);
    accumulate(dmask);
    #endif
}

/* Test mixed-type comparisons and arithmetic */
static void test_mixed_operations(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    /* Arithmetic that produces NaN */
    volatile double div_by_zero = 1.0 / 0.0;          /* inf */
    volatile double inf_minus_inf = inf - inf;        /* nan */
    volatile double zero_div_zero = 0.0 / 0.0;        /* nan */
    
    /* Comparisons after arithmetic */
    if (div_by_zero > nan) accumulate(19);            /* UNORDERED/UNGT? */
    if (inf_minus_inf == zero_div_zero) accumulate(20); /* UNORDERED/UNEQ? */
    if (inf_minus_inf != inf_minus_inf) accumulate(21); /* UNORDERED */
    
    /* Mixed type comparisons */
    if ((double)f_inf < ld_nan) accumulate(22);
    if ((float)nan == (float)inf) accumulate(23);
    
    /* Use math functions with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, inf, 1.0);
    if (fma_result == fma_result) accumulate(24);     /* UNORDERED check */
    
    /* Complex expression tree */
    volatile double complex_expr = (nan * inf) + (inf / nan) - (nan - inf);
    if (complex_expr < 0.0 || complex_expr >= 0.0) {
        accumulate(25);
    }
}

/* Test inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0\n\t"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    accumulate(result1);
    
    /* fucomi with conditional moves */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %b0\n\t"
        : "=r"(result2)
        : "t"(c), "u"(d)
        : "cc"
    );
    accumulate(result2);
    
    /* Multiple comparisons in one asm block */
    asm volatile (
        "ucomisd %3, %2\n\t"
        "jp 1f\n\t"                    /* Jump if unordered */
        "ucomisd %4, %2\n\t"
        "ja 2f\n\t"                    /* Jump if above */
        "movl $1, %0\n\t"
        "jmp 3f\n\t"
        "1:\n\t"
        "movl $2, %0\n\t"
        "jmp 3f\n\t"
        "2:\n\t"
        "movl $3, %0\n\t"
        "3:\n\t"
        : "=r"(result3)
        : "0"(0), "x"(a), "x"(b), "x"(c)
        : "cc"
    );
    accumulate(result3);
}

/* Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0, -1.0};
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int condition = 0;
        
        /* Determine condition code */
        if (__builtin_isunordered(vals[i], nan)) condition = 1;      /* UNORDERED */
        else if (__builtin_islessgreater(vals[i], inf)) condition = 2; /* LTGT */
        else if (!__builtin_isless(vals[i], 0.0)) condition = 3;     /* UNGE */
        else if (__builtin_isgreaterequal(vals[i], vals[(i+1)%5])) condition = 4; /* UNGE */
        else condition = 5;
        
        switch (condition) {
            case 1: accumulate(30 + i); break;  /* UNORDERED path */
            case 2: accumulate(40 + i); break;  /* LTGT path */
            case 3: accumulate(50 + i); break;  /* UNGE path */
            case 4: accumulate(60 + i); break;  /* Another UNGE path */
            case 5: accumulate(70 + i); break;  /* Default */
        }
    }
    
    /* Loop with unordered comparison as condition */
    int counter = 0;
    volatile double x = nan;
    while (!__builtin_isunordered(x, inf) && counter < 3) {
        accumulate(80 + counter);
        x = (counter == 1) ? inf : 0.0;
        counter++;
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_operations();
    test_inline_asm();
    test_control_flow();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}
#endif
