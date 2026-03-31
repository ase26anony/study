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

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators */
    results[idx++] = (nan < inf);      /* UNORDERED case */
    results[idx++] = (nan > inf);      /* UNORDERED case */
    results[idx++] = (nan <= inf);     /* UNORDERED case */
    results[idx++] = (nan >= inf);     /* UNORDERED case */
    results[idx++] = (nan == nan);     /* UNORDERED/UNEQ case */
    results[idx++] = (nan != nan);     /* ORDERED/LTGT case */
    results[idx++] = (inf != nan);     /* ORDERED case */
    results[idx++] = (inf == inf);     /* Normal ordered comparison */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);      /* UNORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);    /* LTGT */
    results[idx++] = __builtin_islessgreater(one, zero);   /* Normal */
    
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isless(inf, nan);           /* UNORDERED */
    results[idx++] = __builtin_isless(neg_inf, inf);       /* Normal */
    
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNGT */
    results[idx++] = __builtin_isgreater(inf, nan);        /* UNORDERED */
    results[idx++] = __builtin_isgreater(inf, neg_inf);    /* Normal */
    
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_islessequal(inf, nan);      /* UNORDERED */
    results[idx++] = __builtin_islessequal(neg_inf, inf);  /* Normal */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, nan);   /* UNORDERED */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf); /* Normal */
    
    /* 3. Complex expressions with arithmetic */
    volatile double nan_arith = inf - inf;  /* Creates NaN */
    volatile double div_by_zero = one / zero; /* Creates Inf */
    
    results[idx++] = (nan_arith < div_by_zero);    /* UNORDERED */
    results[idx++] = (nan_arith == nan_arith);     /* UNORDERED/UNEQ */
    results[idx++] = (div_by_zero > nan_arith);    /* UNORDERED */
    
    /* Use results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
    }
    sink = checksum;
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {1.0f, __builtin_nanf(""), 2.0f, __builtin_inff()};
    v4sf vec_c = {3.0f, 4.0f, 5.0f, 6.0f};
    
    /* Vector comparisons that may generate unordered condition codes */
    v4sf cmp1 = vec_a > vec_b;    /* May generate UNGT/UNORDERED */
    v4sf cmp2 = vec_a < vec_b;    /* May generate UNLT/UNORDERED */
    v4sf cmp3 = vec_a == vec_b;   /* May generate UNEQ/UNORDERED */
    v4sf cmp4 = vec_a != vec_b;   /* May generate LTGT/ORDERED */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3, mask4;
    
    /* Use inline assembly for movmskps if available */
    #ifdef __SSE__
    asm volatile ("movmskps %1, %0" : "=r"(mask1) : "x"(cmp1));
    asm volatile ("movmskps %1, %0" : "=r"(mask2) : "x"(cmp2));
    asm volatile ("movmskps %1, %0" : "=r"(mask3) : "x"(cmp3));
    asm volatile ("movmskps %1, %0" : "=r"(mask4) : "x"(cmp4));
    #else
    /* Fallback: store to memory and check */
    float temp[4];
    memcpy(temp, &cmp1, sizeof(cmp1));
    mask1 = (temp[0] != 0.0f) | ((temp[1] != 0.0f) << 1) | 
            ((temp[2] != 0.0f) << 2) | ((temp[3] != 0.0f) << 3);
    #endif
    
    sink = mask1 ^ mask2 ^ mask3 ^ mask4;
}

/* Test function with inline assembly using explicit condition codes */
void test_inline_asm_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    uint8_t result1, result2, result3, result4;
    
    /* Inline assembly with ucomisd and condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0\n\t"
        : "=r"(result2)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0\n\t"
        : "=r"(result3)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* Using fucomi instruction */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %0\n\t"
        : "=r"(result4)
        : "t"(c), "t"(d)
        : "cc"
    );
    
    sink = result1 + result2 + result3 + result4;
}

/* Control flow based on unordered comparison results */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 2.0, -inf};
    
    int counter = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch on comparison results */
            int cmp_result = 0;
            cmp_result |= __builtin_isunordered(vals[i], vals[j]) ? 1 : 0;
            cmp_result |= __builtin_isless(vals[i], vals[j]) ? 2 : 0;
            cmp_result |= __builtin_isgreater(vals[i], vals[j]) ? 4 : 0;
            cmp_result |= __builtin_islessequal(vals[i], vals[j]) ? 8 : 0;
            cmp_result |= __builtin_isgreaterequal(vals[i], vals[j]) ? 16 : 0;
            cmp_result |= __builtin_islessgreater(vals[i], vals[j]) ? 32 : 0;
            
            switch (cmp_result & 0x3F) {
                case 0:  counter += 1; break;  /* All false */
                case 1:  counter += 2; break;  /* UNORDERED only */
                case 2:  counter += 3; break;  /* UNLT only */
                case 4:  counter += 4; break;  /* UNGT only */
                case 8:  counter += 5; break;  /* UNLE only */
                case 16: counter += 6; break;  /* UNGE only */
                case 32: counter += 7; break;  /* LTGT only */
                case 33: counter += 8; break;  /* UNORDERED | LTGT */
                default: counter += 9; break;  /* Other combinations */
            }
        }
    }
    
    sink = counter;
}

/* Mixed-type comparisons */
void test_mixed_type_comparisons(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    int results = 0;
    
    /* Cross-type comparisons */
    results += (f_nan < d_inf) ? 1 : 0;      /* UNORDERED */
    results += (d_nan == ld_nan) ? 2 : 0;    /* UNORDERED/UNEQ */
    results += (ld_inf > f_nan) ? 4 : 0;     /* UNORDERED */
    
    /* Arithmetic producing NaN */
    volatile double complex_expr = (d_inf / f_nan) * (ld_inf - ld_inf);
    results += (complex_expr == complex_expr) ? 8 : 0;  /* UNORDERED/UNEQ */
    
    /* Using __builtin_fma with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(d_nan, 2.0, 3.0);
    results += (fma_result > 0.0) ? 16 : 0;  /* UNORDERED */
    #endif
    
    sink = results;
}

int main(void) {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    /* Execute all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm_comparisons();
    test_control_flow();
    test_mixed_type_comparisons();
    
    printf("Tests completed (sink = %d)\n", sink);
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
