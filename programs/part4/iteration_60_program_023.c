#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#ifdef __x86_64__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

void test_scalar_comparisons() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    /* Results array to force actual comparisons */
    volatile int results[32];
    int idx = 0;
    
    /* Direct comparisons with NaN to trigger unordered conditions */
    
    /* UNORDERED: nan < inf (unordered because nan is NaN) */
    results[idx++] = (nan < inf) ? 1 : 0;
    
    /* UNORDERED: nan > inf */
    results[idx++] = (nan > inf) ? 2 : 0;
    
    /* UNORDERED: nan <= inf */
    results[idx++] = (nan <= inf) ? 3 : 0;
    
    /* UNORDERED: nan >= inf */
    results[idx++] = (nan >= inf) ? 4 : 0;
    
    /* UNEQ: nan == nan (both are NaN, so unordered equal) */
    results[idx++] = (nan == nan) ? 5 : 0;
    
    /* LTGT: inf != nan (not equal and ordered) */
    results[idx++] = (inf != nan) ? 6 : 0;
    
    /* ORDERED: 1.0 < 2.0 (normal ordered comparison) */
    results[idx++] = (1.0 < 2.0) ? 7 : 0;
    
    /* Built-in functions that map to specific condition codes */
    
    /* __builtin_isunordered -> UNORDERED */
    results[idx++] = __builtin_isunordered(nan, inf) ? 8 : 0;
    
    /* __builtin_islessgreater -> LTGT */
    results[idx++] = __builtin_islessgreater(inf, zero) ? 9 : 0;
    
    /* __builtin_isless -> UNLT or LT */
    results[idx++] = __builtin_isless(zero, inf) ? 10 : 0;
    results[idx++] = __builtin_isless(nan, inf) ? 11 : 0;
    
    /* __builtin_isgreater -> UNGT or GT */
    results[idx++] = __builtin_isgreater(inf, zero) ? 12 : 0;
    results[idx++] = __builtin_isgreater(nan, zero) ? 13 : 0;
    
    /* __builtin_islessequal -> UNLE or LE */
    results[idx++] = __builtin_islessequal(zero, zero) ? 14 : 0;
    results[idx++] = __builtin_islessequal(nan, inf) ? 15 : 0;
    
    /* __builtin_isgreaterequal -> UNGE or GE */
    results[idx++] = __builtin_isgreaterequal(inf, inf) ? 16 : 0;
    results[idx++] = __builtin_isgreaterequal(nan, inf) ? 17 : 0;
    
    /* Complex expressions that may produce NaN */
    volatile double nan_prod = zero / zero;  /* Produces NaN */
    volatile double inf_minus_inf = inf - inf;  /* Produces NaN */
    
    /* UNORDERED comparisons with generated NaN */
    results[idx++] = (nan_prod < one) ? 18 : 0;
    results[idx++] = (inf_minus_inf > neg_inf) ? 19 : 0;
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    /* These may trigger different condition code generation */
    results[idx++] = (f_nan < f_inf) ? 20 : 0;
    results[idx++] = (ld_nan == ld_nan) ? 21 : 0;
    
    /* FMA with NaN input */
    volatile double fma_result = __builtin_fma(nan, one, one);
    results[idx++] = (fma_result > zero) ? 22 : 0;
    
    /* Update checksum */
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
    }
}

void test_vector_comparisons() {
    /* Initialize vectors with NaN and normal values */
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), 2.0f, 1.0f, __builtin_nanf("")};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons - may generate multiple condition checks */
    v4sf cmp_gt = vec_a > vec_b;    /* UNGT/GT */
    v4sf cmp_lt = vec_a < vec_b;    /* UNLT/LT */
    v4sf cmp_eq = vec_a == vec_b;   /* UNEQ/EQ */
    v4sf cmp_ne = vec_a != vec_b;   /* LTGT/NE */
    
    /* Extract comparison masks */
    int mask_gt = __builtin_ia32_movmskps(cmp_gt);
    int mask_lt = __builtin_ia32_movmskps(cmp_lt);
    int mask_eq = __builtin_ia32_movmskps(cmp_eq);
    int mask_ne = __builtin_ia32_movmskps(cmp_ne);
    
    /* Use results to prevent elimination */
    checksum ^= mask_gt ^ mask_lt ^ mask_eq ^ mask_ne;
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), 1.0};
    v2df vec_db = {1.0, __builtin_nan("")};
    
    v2df cmp_d = vec_da > vec_db;
    long long mask_d;
    memcpy(&mask_d, &cmp_d, sizeof(mask_d));
    checksum ^= (int)mask_d;
}

void test_inline_assembly() {
    double a = __builtin_nan("");
    double b = __builtin_inf();
    double c = 1.0;
    double d = 2.0;
    
    int result1, result2, result3, result4;
    
    /* Inline assembly with explicit condition codes */
    
    /* UNORDERED test with ucomisd */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (result1)
        : [a] "x" (a), [b] "x" (b)
        : "al", "cc"
    );
    
    /* ORDERED test */
    asm volatile (
        "ucomisd %[d], %[c]\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (result2)
        : [c] "x" (c), [d] "x" (d)
        : "al", "cc"
    );
    
    /* UNEQ test (unordered or equal) */
    asm volatile (
        "ucomisd %[a], %[a]\n\t"  /* nan == nan */
        "sete %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (result3)
        : [a] "x" (a)
        : "al", "cc"
    );
    
    /* LTGT test (less or greater, ordered) */
    asm volatile (
        "ucomisd %[b], %[c]\n\t"  /* inf > 1.0 */
        "seta %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (result4)
        : [c] "x" (c), [b] "x" (b)
        : "al", "cc"
    );
    
    checksum ^= result1 ^ result2 ^ result3 ^ result4;
}

void test_control_flow() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0, -inf};
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int condition = 0;
            
            /* Each comparison type in control flow */
            if (__builtin_isunordered(vals[i], vals[j])) {
                condition |= 1;  /* UNORDERED */
            }
            if (__builtin_isless(vals[i], vals[j])) {
                condition |= 2;  /* UNLT/LT */
            }
            if (__builtin_isgreater(vals[i], vals[j])) {
                condition |= 4;  /* UNGT/GT */
            }
            if (__builtin_islessequal(vals[i], vals[j])) {
                condition |= 8;  /* UNLE/LE */
            }
            if (__builtin_isgreaterequal(vals[i], vals[j])) {
                condition |= 16; /* UNGE/GE */
            }
            if (__builtin_islessgreater(vals[i], vals[j])) {
                condition |= 32; /* LTGT */
            }
            
            /* Use condition in switch */
            switch (condition & 0x3F) {
                case 0:  checksum += 1; break;
                case 1:  checksum += 2; break;  /* UNORDERED only */
                case 2:  checksum += 3; break;  /* UNLT only */
                case 4:  checksum += 4; break;  /* UNGT only */
                case 8:  checksum += 5; break;  /* UNLE only */
                case 16: checksum += 6; break;  /* UNGE only */
                case 32: checksum += 7; break;  /* LTGT only */
                default: checksum += 8; break;  /* Combined conditions */
            }
        }
    }
}

int main() {
    printf("Testing x86 floating-point condition codes...\n");
    
    #if defined(__x86_64__) || defined(__i386__)
    test_scalar_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    #else
    printf("Non-x86 target - skipping x86-specific tests.\n");
    printf("Checksum: 0\n");
    #endif
    
    return 0;
}

#else
/* Fallback for non-x86 targets */
int main() {
    printf("Non-x86 architecture detected.\n");
    printf("This test requires x86 (i386 or x86-64) for full coverage.\n");
    return 0;
}
#endif
