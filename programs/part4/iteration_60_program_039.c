#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int sink;

/* Test function that performs various unordered comparisons */
void test_unordered_comparisons(void) {
    /* Volatile to prevent constant folding */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[64];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;        /* UNORDERED case */
    results[idx++] = (nan > inf) ? 1 : 0;        /* UNORDERED case */
    results[idx++] = (nan <= inf) ? 1 : 0;       /* UNORDERED case */
    results[idx++] = (nan >= inf) ? 1 : 0;       /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;       /* UNORDERED/UNEQ case */
    results[idx++] = (inf != nan) ? 1 : 0;       /* ORDERED/LTGT case */
    results[idx++] = (nan != nan) ? 1 : 0;       /* UNORDERED case */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);      /* UNORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);    /* LTGT */
    results[idx++] = __builtin_islessgreater(one, neg_one); /* LTGT */
    
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isless(neg_inf, inf);       /* UNLT */
    
    results[idx++] = __builtin_isgreater(inf, nan);        /* UNGT */
    results[idx++] = __builtin_isgreater(inf, neg_inf);    /* UNGT */
    
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_islessequal(neg_inf, inf);  /* UNLE */
    
    results[idx++] = __builtin_isgreaterequal(inf, nan);   /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf); /* UNGE */
    
    /* 3. Complex expressions that may produce NaN */
    volatile double nan_prod = zero / zero;
    volatile double inf_minus_inf = inf - inf;
    volatile double inf_div_inf = inf / inf;
    
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;       /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != inf_minus_inf) ? 1 : 0; /* ORDERED */
    results[idx++] = (inf_div_inf < one) ? 1 : 0;          /* UNORDERED */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan == (float)nan) ? 1 : 0;        /* UNORDERED/UNEQ */
    results[idx++] = (ld_nan != (long double)inf) ? 1 : 0; /* ORDERED/LTGT */
    
    /* 5. Arithmetic operations that could affect condition codes */
    volatile double complex_expr = (nan * inf) + (inf / nan);
    results[idx++] = (complex_expr == complex_expr) ? 1 : 0; /* UNORDERED */
    
    /* 6. Using __builtin_fma with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, one, inf);
    results[idx++] = (fma_result > zero) ? 1 : 0;          /* UNORDERED/UNGT */
    
    /* Store results to prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        sink = results[i];
    }
}

/* Test function using vector extensions */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {__builtin_inff(), -1.0f, __builtin_nanf(""), 1.0f};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result;
    
    /* Various comparison operations */
    cmp_result = vec_a > vec_b;    /* May generate UNGT/UNORDERED */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    sink = mask1;
    
    cmp_result = vec_a < vec_b;    /* May generate UNLT/UNORDERED */
    int mask2 = __builtin_ia32_movmskps(cmp_result);
    sink = mask2;
    
    cmp_result = vec_a == vec_b;   /* May generate UNEQ/UNORDERED */
    int mask3 = __builtin_ia32_movmskps(cmp_result);
    sink = mask3;
    
    cmp_result = vec_a != vec_b;   /* May generate LTGT/ORDERED */
    int mask4 = __builtin_ia32_movmskps(cmp_result);
    sink = mask4;
    
    cmp_result = vec_a >= vec_b;   /* May generate UNGE/UNORDERED */
    int mask5 = __builtin_ia32_movmskps(cmp_result);
    sink = mask5;
    
    cmp_result = vec_a <= vec_b;   /* May generate UNLE/UNORDERED */
    int mask6 = __builtin_ia32_movmskps(cmp_result);
    sink = mask6;
    
    /* Double precision vector comparisons */
    v2df dvec_a = {__builtin_nan(""), __builtin_inf()};
    v2df dvec_b = {__builtin_inf(), __builtin_nan("")};
    v2df dvec_c = {0.0, 0.0};
    
    dvec_c = dvec_a > dvec_b;
    int dmask = __builtin_ia32_movmskpd(dvec_c);
    sink = dmask;
}

/* Test function with inline assembly */
void test_asm_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result1, result2, result3, result4;
    
    /* Inline assembly using ucomisd with explicit condition code handling */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result2)
        : "x"(c), "x"(d)
        : "al", "cc"
    );
    
    /* Using fucomi instruction */
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "t"(a), "u"(b)
        : "al", "cc"
    );
    
    sink = result1 + result2 + result3;
}

/* Control flow based on unordered comparison results */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, -1.0, 0.0};
    
    int checksum = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch on combination of comparison results */
            int cmp_results = 0;
            
            if (__builtin_isunordered(vals[i], vals[j])) {
                cmp_results |= 1;  /* UNORDERED */
            }
            if (__builtin_isless(vals[i], vals[j])) {
                cmp_results |= 2;  /* UNLT */
            }
            if (__builtin_isgreater(vals[i], vals[j])) {
                cmp_results |= 4;  /* UNGT */
            }
            if (__builtin_islessequal(vals[i], vals[j])) {
                cmp_results |= 8;  /* UNLE */
            }
            if (__builtin_isgreaterequal(vals[i], vals[j])) {
                cmp_results |= 16; /* UNGE */
            }
            if (__builtin_islessgreater(vals[i], vals[j])) {
                cmp_results |= 32; /* LTGT */
            }
            
            /* Use switch to force generation of condition codes */
            switch (cmp_results & 0x3F) {
                case 0:  /* ORDERED and equal? */
                    checksum += 1;
                    break;
                case 1:  /* UNORDERED */
                    checksum += 2;
                    break;
                case 2:  /* UNLT */
                    checksum += 3;
                    break;
                case 4:  /* UNGT */
                    checksum += 4;
                    break;
                case 8:  /* UNLE */
                    checksum += 5;
                    break;
                case 16: /* UNGE */
                    checksum += 6;
                    break;
                case 32: /* LTGT */
                    checksum += 7;
                    break;
                case 33: /* UNORDERED | LTGT */
                    checksum += 8;
                    break;
                default:
                    checksum += 9;
                    break;
            }
        }
    }
    
    sink = checksum;
}

int main(void) {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    /* Call all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_asm_comparisons();
    test_control_flow();
    
    /* Compute final checksum to prevent optimization */
    int final_result = sink;
    printf("Result: %d\n", final_result);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
