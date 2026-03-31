#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to create checksum to prevent optimization */
static volatile int checksum = 0;

/* Force generation of specific condition codes */
void test_scalar_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[32];
    int idx = 0;
    
    /* Direct comparisons with NaN - should generate UNORDERED cases */
    
    /* UNORDERED: nan < inf (unordered because nan is NaN) */
    results[idx++] = (nan < inf) ? 1 : 0;
    
    /* UNORDERED: nan > inf */
    results[idx++] = (nan > inf) ? 2 : 0;
    
    /* UNORDERED: nan <= inf */
    results[idx++] = (nan <= inf) ? 3 : 0;
    
    /* UNORDERED: nan >= inf */
    results[idx++] = (nan >= inf) ? 4 : 0;
    
    /* UNEQ: nan == nan (both are NaN, unordered equal) */
    results[idx++] = (nan == nan) ? 5 : 0;
    
    /* LTGT: inf != nan (not equal and ordered) */
    results[idx++] = (inf != nan) ? 6 : 0;
    
    /* ORDERED: inf < 100.0 (both are ordered) */
    results[idx++] = (inf < 100.0) ? 7 : 0;
    
    /* UNGE: !(nan < inf) -> nlt (not less than, unordered) */
    results[idx++] = !(nan < inf) ? 8 : 0;
    
    /* UNGT: !(nan <= inf) -> nle (not less or equal, unordered) */
    results[idx++] = !(nan <= inf) ? 9 : 0;
    
    /* UNLE: nan <= nan (unordered less or equal) */
    results[idx++] = (nan <= nan) ? 10 : 0;
    
    /* UNLT: nan < nan (unordered less than) */
    results[idx++] = (nan < nan) ? 11 : 0;
    
    /* Built-in functions that map directly to condition codes */
    
    /* __builtin_isunordered generates UNORDERED condition */
    results[idx++] = __builtin_isunordered(nan, inf) ? 12 : 0;
    
    /* __builtin_islessgreater generates LTGT condition */
    results[idx++] = __builtin_islessgreater(inf, nan) ? 13 : 0;
    
    /* __builtin_isless generates ordered LT */
    results[idx++] = __builtin_isless(one, inf) ? 14 : 0;
    
    /* __builtin_isgreater generates ordered GT */
    results[idx++] = __builtin_isgreater(inf, one) ? 15 : 0;
    
    /* __builtin_islessequal generates ordered LE */
    results[idx++] = __builtin_islessequal(one, one) ? 16 : 0;
    
    /* __builtin_isgreaterequal generates ordered GE */
    results[idx++] = __builtin_isgreaterequal(inf, one) ? 17 : 0;
    
    /* Complex expressions that may generate multiple condition codes */
    volatile double nan2 = zero / zero;  /* Another way to get NaN */
    volatile double inf2 = one / zero;   /* Another way to get INF */
    
    /* Mixed comparisons */
    results[idx++] = (nan2 == inf2) ? 18 : 0;      /* UNEQ? */
    results[idx++] = (nan2 != inf2) ? 19 : 0;      /* LTGT? */
    results[idx++] = (nan2 < inf2) ? 20 : 0;       /* UNORDERED */
    results[idx++] = (inf2 > nan2) ? 21 : 0;       /* UNORDERED */
    
    /* Arithmetic that produces NaN */
    volatile double nan_arith = inf - inf;
    volatile double nan_div = inf / inf;
    
    results[idx++] = (nan_arith == nan_div) ? 22 : 0;  /* UNEQ */
    results[idx++] = (nan_arith != one) ? 23 : 0;      /* LTGT */
    
    /* Update checksum */
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
    }
}

void test_vector_comparisons(void) {
    /* Vector comparisons using GCC extensions */
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), 2.0f, 1.0f, __builtin_nanf("")};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons - may generate multiple condition codes */
    v4sf cmp_gt = vec_a > vec_b;    /* Element-wise greater than */
    v4sf cmp_lt = vec_a < vec_b;    /* Element-wise less than */
    v4sf cmp_eq = vec_a == vec_b;   /* Element-wise equal */
    v4sf cmp_ne = vec_a != vec_b;   /* Element-wise not equal */
    v4sf cmp_ge = vec_a >= vec_b;   /* Element-wise greater or equal */
    v4sf cmp_le = vec_a <= vec_b;   /* Element-wise less or equal */
    
    /* Extract comparison masks to force code generation */
    int mask_gt, mask_lt, mask_eq, mask_ne, mask_ge, mask_le;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask_gt = __builtin_ia32_movmskps(cmp_gt);
    mask_lt = __builtin_ia32_movmskps(cmp_lt);
    mask_eq = __builtin_ia32_movmskps(cmp_eq);
    mask_ne = __builtin_ia32_movmskps(cmp_ne);
    mask_ge = __builtin_ia32_movmskps(cmp_ge);
    mask_le = __builtin_ia32_movmskps(cmp_le);
    #else
    /* Fallback: store to memory and check */
    float store_gt[4], store_lt[4], store_eq[4], store_ne[4], store_ge[4], store_le[4];
    memcpy(store_gt, &cmp_gt, sizeof(cmp_gt));
    memcpy(store_lt, &cmp_lt, sizeof(cmp_lt));
    memcpy(store_eq, &cmp_eq, sizeof(cmp_eq));
    memcpy(store_ne, &cmp_ne, sizeof(cmp_ne));
    memcpy(store_ge, &cmp_ge, sizeof(cmp_ge));
    memcpy(store_le, &cmp_le, sizeof(cmp_le));
    
    mask_gt = (store_gt[0] != 0) | ((store_gt[1] != 0) << 1) | 
              ((store_gt[2] != 0) << 2) | ((store_gt[3] != 0) << 3);
    mask_lt = (store_lt[0] != 0) | ((store_lt[1] != 0) << 1) | 
              ((store_lt[2] != 0) << 2) | ((store_lt[3] != 0) << 3);
    mask_eq = (store_eq[0] != 0) | ((store_eq[1] != 0) << 1) | 
              ((store_eq[2] != 0) << 2) | ((store_eq[3] != 0) << 3);
    mask_ne = (store_ne[0] != 0) | ((store_ne[1] != 0) << 1) | 
              ((store_ne[2] != 0) << 2) | ((store_ne[3] != 0) << 3);
    mask_ge = (store_ge[0] != 0) | ((store_ge[1] != 0) << 1) | 
              ((store_ge[2] != 0) << 2) | ((store_ge[3] != 0) << 3);
    mask_le = (store_le[0] != 0) | ((store_le[1] != 0) << 1) | 
              ((store_le[2] != 0) << 2) | ((store_le[3] != 0) << 3);
    #endif
    
    checksum ^= mask_gt ^ mask_lt ^ mask_eq ^ mask_ne ^ mask_ge ^ mask_le;
}

void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result1, result2, result3, result4;
    
    /* Inline assembly with explicit condition codes */
    
    /* UNORDERED test using ucomisd */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result1)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "x" (c), "x" (d)
        : "al", "cc"
    );
    
    /* UNEQ test (unordered or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setpe %%al\n\t"
        "sete %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result3)
        : "x" (a), "x" (a)  /* nan == nan */
        : "al", "dl", "cc"
    );
    
    /* LTGT test (less or greater, ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "setp %%dl\n\t"
        "andn %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result4)
        : "x" (c), "x" (d)  /* 1.0 != 2.0, both ordered */
        : "al", "dl", "cc"
    );
    
    checksum ^= result1 ^ result2 ^ result3 ^ result4;
}

void test_control_flow_unordered(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 2.0, 0.0};
    
    int switch_result = 0;
    
    /* Control flow based on unordered comparisons */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch on combination of comparison results */
            int cmp_mask = 0;
            
            if (__builtin_isunordered(vals[i], vals[j])) cmp_mask |= 1;
            if (__builtin_isless(vals[i], vals[j])) cmp_mask |= 2;
            if (__builtin_isgreater(vals[i], vals[j])) cmp_mask |= 4;
            if (__builtin_islessequal(vals[i], vals[j])) cmp_mask |= 8;
            if (__builtin_isgreaterequal(vals[i], vals[j])) cmp_mask |= 16;
            if (__builtin_islessgreater(vals[i], vals[j])) cmp_mask |= 32;
            
            /* Switch statement to force jump table generation */
            switch (cmp_mask & 0x3F) {
                case 0:  /* All false - ORDERED and EQUAL? */
                    switch_result += 1;
                    break;
                case 1:  /* UNORDERED only */
                    switch_result += 2;
                    break;
                case 2:  /* LT only */
                    switch_result += 3;
                    break;
                case 4:  /* GT only */
                    switch_result += 4;
                    break;
                case 32: /* LTGT only */
                    switch_result += 5;
                    break;
                case 33: /* UNORDERED | LTGT */
                    switch_result += 6;
                    break;
                case 10: /* UNORDERED | LT (UNLT) */
                    switch_result += 7;
                    break;
                case 9:  /* UNORDERED | LE (UNLE) */
                    switch_result += 8;
                    break;
                case 24: /* GE | LE (implies EQ) */
                    switch_result += 9;
                    break;
                default:
                    switch_result += 10;
                    break;
            }
        }
    }
    
    checksum ^= switch_result;
}

void test_mixed_type_comparisons(void) {
    /* Mixed floating-point type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    int mixed_results = 0;
    
    /* Cross-type comparisons */
    mixed_results += (f_nan == d_nan) ? 1 : 0;      /* UNEQ */
    mixed_results += (f_inf != ld_nan) ? 2 : 0;     /* LTGT */
    mixed_results += (d_nan < f_inf) ? 4 : 0;       /* UNORDERED */
    mixed_results += (ld_inf > f_nan) ? 8 : 0;      /* UNORDERED */
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_nan = __builtin_fma(d_nan, d_inf, 1.0);
    mixed_results += (fma_nan == fma_nan) ? 16 : 0; /* UNEQ */
    #endif
    
    /* Complex arithmetic expressions */
    volatile double expr1 = (d_inf * 0.0) / 0.0;  /* NaN */
    volatile double expr2 = d_inf - d_inf;        /* NaN */
    
    mixed_results += (expr1 < expr2) ? 32 : 0;    /* UNORDERED */
    mixed_results += !(expr1 >= expr2) ? 64 : 0;  /* UNGE -> nlt */
    mixed_results += !(expr1 > expr2) ? 128 : 0;  /* UNGT -> nle */
    
    checksum ^= mixed_results;
}

int main(void) {
    printf("Testing x86 floating-point unordered condition codes...\n");
    
    /* Run all tests */
    test_scalar_unordered_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow_unordered();
    test_mixed_type_comparisons();
    
    printf("Checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    return 0;
}

#else /* Non-x86 target */

int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}

#endif
