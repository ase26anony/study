#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

#ifdef __x86_64__ || __i386__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to force conditional code generation */
__attribute__((noinline))
static void record_result(int cond) {
    checksum ^= cond;
}

/* Test function with various unordered comparisons */
__attribute__((noinline))
static void test_scalar_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[32];
    int idx = 0;
    
    /* Direct comparisons with operators - should generate various condition codes */
    
    /* UNORDERED cases */
    results[idx++] = __builtin_isunordered(nan, inf);      /* true */
    results[idx++] = __builtin_isunordered(inf, nan);      /* true */
    results[idx++] = __builtin_isunordered(nan, nan);      /* true */
    
    /* ORDERED cases */
    results[idx++] = !__builtin_isunordered(inf, zero);    /* true */
    results[idx++] = !__builtin_isunordered(zero, one);    /* true */
    
    /* UNEQ (unordered or equal) */
    results[idx++] = (nan == nan) || __builtin_isunordered(nan, nan);  /* true */
    results[idx++] = (inf == inf) || __builtin_isunordered(inf, inf);  /* true */
    
    /* UNGE (unordered or greater or equal) - using nlt */
    results[idx++] = !(nan < inf) || __builtin_isunordered(nan, inf);  /* true */
    results[idx++] = !(zero < inf) || __builtin_isunordered(zero, inf); /* true */
    
    /* UNGT (unordered or greater) - using nle */
    results[idx++] = !(nan <= inf) || __builtin_isunordered(nan, inf);  /* true */
    results[idx++] = !(zero <= neg_inf) || __builtin_isunordered(zero, neg_inf); /* false */
    
    /* UNLE (unordered or less or equal) */
    results[idx++] = (nan <= inf) || __builtin_isunordered(nan, inf);   /* true */
    results[idx++] = (inf <= nan) || __builtin_isunordered(inf, nan);   /* true */
    
    /* UNLT (unordered or less) */
    results[idx++] = (nan < inf) || __builtin_isunordered(nan, inf);    /* true */
    results[idx++] = (inf < nan) || __builtin_isunordered(inf, nan);    /* true */
    
    /* LTGT (less or greater) - using une */
    results[idx++] = __builtin_islessgreater(inf, zero);   /* true */
    results[idx++] = __builtin_islessgreater(zero, inf);   /* true */
    results[idx++] = __builtin_islessgreater(nan, zero);   /* false */
    
    /* Complex expressions mixing builtins */
    results[idx++] = __builtin_isless(neg_inf, inf) && !__builtin_isunordered(neg_inf, inf);
    results[idx++] = __builtin_isgreater(inf, neg_inf) && !__builtin_isunordered(inf, neg_inf);
    results[idx++] = __builtin_islessequal(zero, one) && !__builtin_isunordered(zero, one);
    results[idx++] = __builtin_isgreaterequal(one, zero) && !__builtin_isunordered(one, zero);
    
    /* Arithmetic that could produce NaN */
    volatile double nan_prod = inf * zero;
    volatile double nan_sub = inf - inf;
    
    results[idx++] = __builtin_isunordered(nan_prod, zero);
    results[idx++] = __builtin_isunordered(nan_sub, nan);
    results[idx++] = !__builtin_islessgreater(nan_prod, nan_sub);
    
    /* Mixed type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = __builtin_isunordered(f_nan, (float)inf);
    results[idx++] = __builtin_isunordered((double)f_inf, ld_nan);
    
    /* FMA with NaN inputs */
    results[idx++] = __builtin_isunordered(
        __builtin_fma(nan, one, zero), 
        __builtin_fma(inf, zero, nan)
    );
    
    /* Store results to checksum */
    for (int i = 0; i < idx; i++) {
        record_result(results[i]);
    }
}

/* Test vector comparisons */
__attribute__((noinline))
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {__builtin_nanf(""), __builtin_inff(), 1.0f, -__builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons - may generate multiple condition checks */
    v4sf cmp_result;
    
    /* Greater than - may use UNLE/UNLT condition codes */
    cmp_result = vec_a > vec_b;
    
    /* Less than or equal */
    cmp_result = vec_a <= vec_b;
    
    /* Equal - may use UNEQ */
    cmp_result = vec_a == vec_b;
    
    /* Not equal - may use LTGT */
    cmp_result = vec_a != vec_b;
    
    /* Extract comparison mask */
    int mask = __builtin_ia32_movmskps(cmp_result);
    record_result(mask);
    
    /* Double precision vector */
    v2df dvec_a = {__builtin_nan(""), __builtin_inf()};
    v2df dvec_b = {__builtin_inf(), __builtin_nan("")};
    
    v2df dcmp = dvec_a > dvec_b;
    
    /* Force use of result */
    volatile double* ptr = (volatile double*)&dcmp;
    record_result((int)ptr[0]);
    record_result((int)ptr[1]);
}

/* Inline assembly with explicit condition codes */
__attribute__((noinline))
static void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    volatile double d = 1.0;
    
    int result_p, result_z, result_c;
    
    /* UNORDERED test with ucomisd */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result_p)
        : "x"(a), "x"(b)
        : "cc"
    );
    record_result(result_p);
    
    /* ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_p)
        : "x"(c), "x"(d)
        : "cc", "al"
    );
    record_result(result_p);
    
    /* Test with fucomi (x87) */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setz %0"
        : "=r"(result_z)
        : "m"(a), "m"(b)
        : "cc"
    );
    record_result(result_z);
    
    /* Complex condition: UNGE (nlt) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_c)
        : "x"(a), "x"(b)
        : "cc", "al"
    );
    record_result(result_c);
}

/* Control flow based on unordered comparisons */
__attribute__((noinline))
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 0.0, 1.0, -inf};
    
    int switch_result = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Complex condition determining switch case */
            int cond = 0;
            cond |= __builtin_isunordered(vals[i], vals[j]) ? 1 : 0;
            cond |= __builtin_isless(vals[i], vals[j]) ? 2 : 0;
            cond |= __builtin_isgreater(vals[i], vals[j]) ? 4 : 0;
            cond |= __builtin_islessequal(vals[i], vals[j]) ? 8 : 0;
            cond |= __builtin_isgreaterequal(vals[i], vals[j]) ? 16 : 0;
            cond |= __builtin_islessgreater(vals[i], vals[j]) ? 32 : 0;
            
            /* Switch on combined condition - forces multiple condition codes */
            switch (cond & 0x3F) {
                case 1:  /* UNORDERED only */
                    switch_result ^= 0x01;
                    break;
                case 2:  /* Less only */
                    switch_result ^= 0x02;
                    break;
                case 4:  /* Greater only */
                    switch_result ^= 0x04;
                    break;
                case 8:  /* Less or equal */
                    switch_result ^= 0x08;
                    break;
                case 16: /* Greater or equal */
                    switch_result ^= 0x10;
                    break;
                case 32: /* Less or greater (LTGT) */
                    switch_result ^= 0x20;
                    break;
                case 33: /* UNORDERED | LTGT */
                    switch_result ^= 0x40;
                    break;
                default:
                    switch_result ^= 0x80;
                    break;
            }
        }
    }
    
    record_result(switch_result);
}

#endif /* __x86_64__ || __i386__ */

int main(void) {
#ifdef __x86_64__ || __i386__
    printf("Testing x86 floating-point unordered condition codes...\n");
    
    test_scalar_unordered_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
#else
    printf("Not an x86 target - skipping unordered comparison tests.\n");
    return 0;
#endif
}
