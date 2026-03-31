#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __x86_64__ || __i386__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int sink;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Helper to update checksum */
static void update_checksum(int val) {
    checksum = (checksum << 1) ^ (uint32_t)val;
}

/* Test function with various unordered comparisons */
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
    results[idx++] = (nan < inf);      /* UNORDERED case */
    results[idx++] = (nan > inf);      /* UNORDERED case */
    results[idx++] = (nan <= inf);     /* UNORDERED case */
    results[idx++] = (nan >= inf);     /* UNORDERED case */
    results[idx++] = (nan == nan);     /* UNORDERED/UNEQ case */
    results[idx++] = (inf != nan);     /* ORDERED/LTGT case */
    results[idx++] = (nan != nan);     /* UNORDERED case */
    
    /* 2. Comparisons that might produce ORDERED results */
    results[idx++] = (inf > zero);     /* ORDERED case */
    results[idx++] = (zero < inf);     /* ORDERED case */
    results[idx++] = (neg_inf < zero); /* ORDERED case */
    
    /* 3. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);       /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);       /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);       /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, zero);      /* ORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);     /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, nan);     /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, zero);    /* ORDERED/LTGT */
    results[idx++] = __builtin_islessgreater(zero, inf);    /* ORDERED/LTGT */
    
    results[idx++] = __builtin_isless(nan, inf);            /* UNORDERED */
    results[idx++] = __builtin_isless(inf, nan);            /* UNORDERED */
    results[idx++] = __builtin_isless(neg_inf, inf);        /* ORDERED/UNLT */
    
    results[idx++] = __builtin_isgreater(nan, inf);         /* UNORDERED */
    results[idx++] = __builtin_isgreater(inf, nan);         /* UNORDERED */
    results[idx++] = __builtin_isgreater(inf, neg_inf);     /* ORDERED/UNGT */
    
    results[idx++] = __builtin_islessequal(nan, inf);       /* UNORDERED */
    results[idx++] = __builtin_islessequal(inf, nan);       /* UNORDERED */
    results[idx++] = __builtin_islessequal(neg_inf, inf);   /* ORDERED/UNLE */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_isgreaterequal(inf, nan);    /* UNORDERED */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf);/* ORDERED/UNGE */
    
    /* 4. Complex expressions with arithmetic that might produce NaN */
    volatile double nan_prod = zero / zero;  /* Produces NaN */
    volatile double inf_minus_inf = inf - inf;  /* Produces NaN */
    
    results[idx++] = (nan_prod == inf_minus_inf);  /* UNORDERED/UNEQ */
    results[idx++] = (nan_prod != inf_minus_inf);  /* UNORDERED */
    results[idx++] = (nan_prod < one);             /* UNORDERED */
    results[idx++] = (inf_minus_inf > neg_one);    /* UNORDERED */
    
    /* 5. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan == (float)nan);        /* UNORDERED/UNEQ */
    results[idx++] = (ld_nan != (long double)inf); /* UNORDERED */
    results[idx++] = ((double)f_nan < inf);        /* UNORDERED */
    results[idx++] = (nan > (double)ld_nan);       /* UNORDERED */
    
    /* Update checksum with all results */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
    
    sink = idx; /* Prevent optimization */
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that generate condition codes */
    v4sf cmp_result;
    
    /* Various comparison operations */
    cmp_result = vec_a > vec_b;   /* May generate UNORDERED/UNGT */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask1);
    
    cmp_result = vec_a < vec_b;   /* May generate UNORDERED/UNLT */
    int mask2 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask2);
    
    cmp_result = vec_a == vec_b;  /* May generate UNORDERED/UNEQ */
    int mask3 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask3);
    
    cmp_result = vec_a != vec_b;  /* May generate UNORDERED/LTGT */
    int mask4 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask4);
    
    cmp_result = vec_a >= vec_b;  /* May generate UNORDERED/UNGE */
    int mask5 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask5);
    
    cmp_result = vec_a <= vec_b;  /* May generate UNORDERED/UNLE */
    int mask6 = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask6);
    
    /* Double precision vector comparisons */
    v2df dvec_a = {__builtin_nan(""), 1.0};
    v2df dvec_b = {1.0, __builtin_nan("")};
    v2df dvec_c = {0.0, 0.0};
    
    dvec_c = dvec_a > dvec_b;
    /* Extract results - force evaluation */
    volatile double d0 = dvec_c[0];
    volatile double d1 = dvec_c[1];
    update_checksum((int)(d0 != 0.0));
    update_checksum((int)(d1 != 0.0));
    
    sink = mask1 + mask2 + mask3 + mask4 + mask5 + mask6;
}

/* Test with inline assembly */
void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result1, result2, result3, result4;
    
    /* Inline assembly with explicit condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    update_checksum(result1);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result2)
        : "x"(b), "x"(c)
        : "al", "cc"
    );
    update_checksum(result2);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "x"(c), "x"(d)
        : "al", "cc"
    );
    update_checksum(result3);
    
    /* Test with fucomi instruction */
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result4)
        : "t"(a), "u"(b)
        : "al", "cc"
    );
    update_checksum(result4);
    
    sink = result1 + result2 + result3 + result4;
}

/* Control flow based on unordered comparisons */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, -inf, 0.0, 1.0, -1.0};
    
    int switch_result = 0;
    
    /* Complex switch based on comparison results */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int cmp_result = 0;
            
            if (__builtin_isunordered(values[i], values[j])) {
                cmp_result |= 1;  /* UNORDERED */
            }
            if (__builtin_isless(values[i], values[j])) {
                cmp_result |= 2;  /* UNLT */
            }
            if (__builtin_isgreater(values[i], values[j])) {
                cmp_result |= 4;  /* UNGT */
            }
            if (__builtin_islessequal(values[i], values[j])) {
                cmp_result |= 8;  /* UNLE */
            }
            if (__builtin_isgreaterequal(values[i], values[j])) {
                cmp_result |= 16; /* UNGE */
            }
            if (__builtin_islessgreater(values[i], values[j])) {
                cmp_result |= 32; /* LTGT */
            }
            
            /* Switch on the combination of comparison results */
            switch (cmp_result & 0x3F) {
                case 0:  /* ORDERED and equal? */
                    switch_result += 1;
                    break;
                case 1:  /* UNORDERED only */
                    switch_result += 2;
                    break;
                case 2:  /* UNLT only */
                    switch_result += 3;
                    break;
                case 4:  /* UNGT only */
                    switch_result += 4;
                    break;
                case 6:  /* UNLT and UNGT (LTGT) */
                    switch_result += 5;
                    break;
                case 10: /* UNLT and UNLE */
                    switch_result += 6;
                    break;
                case 18: /* UNGT and UNGE */
                    switch_result += 7;
                    break;
                case 33: /* UNORDERED and LTGT */
                    switch_result += 8;
                    break;
                default:
                    switch_result += 9;
                    break;
            }
        }
    }
    
    update_checksum(switch_result);
    sink = switch_result;
}

/* Test with math functions that might produce NaN */
#include <math.h>
void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* Use math functions with NaN inputs */
    volatile double result;
    
    result = nan + inf;  /* Produces NaN */
    update_checksum(__builtin_isunordered(result, 0.0));
    
    result = inf / 0.0;  /* Produces inf */
    update_checksum(__builtin_isless(result, nan));
    
    result = 0.0 / 0.0;  /* Produces NaN */
    update_checksum(__builtin_isunordered(result, result));
    
    result = inf - inf;  /* Produces NaN */
    update_checksum(__builtin_islessgreater(result, 1.0));
    
    /* Use fma with NaN */
    result = __builtin_fma(nan, 2.0, 3.0);  /* Produces NaN */
    update_checksum(__builtin_isgreaterequal(result, inf));
    
    sink = (int)result;
}

int main(void) {
    /* Initialize checksum */
    checksum = 0;
    
    /* Run all tests */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm();
    test_control_flow();
    test_math_functions();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
}
#endif
