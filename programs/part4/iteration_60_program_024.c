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

/* Function to accumulate results */
static void accumulate(int cond) {
    checksum ^= cond;
    checksum += 1;
}

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    int results[32];
    int idx = 0;
    
    /* ====== 1. Direct unordered comparisons with operators ====== */
    
    /* UNORDERED cases - comparisons involving NaN */
    results[idx++] = (nan < inf) ? 1 : 0;        /* Should be false (unordered) */
    results[idx++] = (nan > inf) ? 1 : 0;        /* Should be false (unordered) */
    results[idx++] = (nan == nan) ? 1 : 0;       /* Should be false (unordered) */
    results[idx++] = (nan != nan) ? 1 : 0;       /* Should be true (unordered) */
    results[idx++] = (inf != nan) ? 1 : 0;       /* Should be true */
    
    /* ORDERED cases - valid comparisons */
    results[idx++] = (inf > zero) ? 1 : 0;       /* Should be true (ordered) */
    results[idx++] = (zero < one) ? 1 : 0;       /* Should be true (ordered) */
    results[idx++] = (neg_inf < zero) ? 1 : 0;   /* Should be true (ordered) */
    
    /* UNEQ (unordered or equal) */
    volatile double a = nan;
    volatile double b = nan;
    results[idx++] = (a == b) ? 1 : 0;           /* UNEQ when both NaN */
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    results[idx++] = (inf != neg_inf) ? 1 : 0;   /* LTGT - not equal, ordered */
    
    /* ====== 2. Built-in unordered comparison functions ====== */
    
    /* __builtin_isunordered - directly maps to UNORDERED */
    results[idx++] = __builtin_isunordered(nan, inf) ? 1 : 0;
    results[idx++] = __builtin_isunordered(inf, nan) ? 1 : 0;
    results[idx++] = __builtin_isunordered(nan, nan) ? 1 : 0;
    results[idx++] = __builtin_isunordered(one, zero) ? 1 : 0;
    
    /* __builtin_islessgreater - maps to LTGT */
    results[idx++] = __builtin_islessgreater(inf, neg_inf) ? 1 : 0;
    results[idx++] = __builtin_islessgreater(one, zero) ? 1 : 0;
    results[idx++] = __builtin_islessgreater(nan, one) ? 1 : 0;
    
    /* __builtin_isless - should handle NaN properly */
    results[idx++] = __builtin_isless(one, two) ? 1 : 0;
    results[idx++] = __builtin_isless(nan, one) ? 1 : 0;
    results[idx++] = __builtin_isless(one, nan) ? 1 : 0;
    
    /* __builtin_isgreater */
    results[idx++] = __builtin_isgreater(inf, zero) ? 1 : 0;
    results[idx++] = __builtin_isgreater(one, nan) ? 1 : 0;
    
    /* __builtin_islessequal - may use UNLE */
    results[idx++] = __builtin_islessequal(one, one) ? 1 : 0;
    results[idx++] = __builtin_islessequal(zero, one) ? 1 : 0;
    results[idx++] = __builtin_islessequal(nan, one) ? 1 : 0;
    
    /* __builtin_isgreaterequal - may use UNGE */
    results[idx++] = __builtin_isgreaterequal(one, one) ? 1 : 0;
    results[idx++] = __builtin_isgreaterequal(one, zero) ? 1 : 0;
    results[idx++] = __builtin_isgreaterequal(one, nan) ? 1 : 0;
    
    /* ====== 3. Complex expressions with arithmetic ====== */
    
    /* Create NaN through arithmetic */
    volatile double nan_arith = inf - inf;
    volatile double nan_div = zero / zero;
    
    results[idx++] = (nan_arith == nan_arith) ? 1 : 0;   /* UNEQ/UNORDERED */
    results[idx++] = (nan_div > zero) ? 1 : 0;           /* UNORDERED */
    
    /* Mixed type comparisons */
    results[idx++] = (f_nan < f_inf) ? 1 : 0;
    results[idx++] = (ld_nan == ld_nan) ? 1 : 0;
    
    /* FMA with NaN input */
    volatile double fma_nan = __builtin_fma(nan, one, zero);
    results[idx++] = (fma_nan > zero) ? 1 : 0;
    
    /* ====== 4. Control flow based on unordered comparisons ====== */
    
    /* Switch statement driven by comparison results */
    int switch_val = 0;
    if (__builtin_isunordered(nan, inf)) switch_val |= 1;
    if (__builtin_islessgreater(inf, neg_inf)) switch_val |= 2;
    if (!__builtin_isless(nan, one)) switch_val |= 4;
    
    switch (switch_val) {
        case 0:
            results[idx++] = 0;
            break;
        case 1:
            results[idx++] = 1;  /* UNORDERED path */
            break;
        case 2:
            results[idx++] = 2;  /* LTGT path */
            break;
        case 3:
            results[idx++] = 3;  /* Combined */
            break;
        default:
            results[idx++] = 4;
    }
    
    /* Ternary operators with unordered comparisons */
    results[idx++] = __builtin_isunordered(nan, zero) ? 
                     __builtin_islessgreater(one, two) : 
                     __builtin_islessequal(zero, one);
    
    /* ====== 5. Vector comparisons ====== */
    
    v4sf vec_a = {1.0f, 2.0f, f_nan, 4.0f};
    v4sf vec_b = {2.0f, 1.0f, 3.0f, f_nan};
    v4sf vec_cmp = vec_a > vec_b;  /* Should generate multiple condition checks */
    
    /* Extract comparison mask */
    int mask = __builtin_ia32_movmskps(vec_cmp);
    results[idx++] = mask;
    
    /* Check individual elements (forces scalar codegen) */
    float vec_store[4];
    memcpy(vec_store, &vec_a, sizeof(vec_a));
    for (int i = 0; i < 4; i++) {
        results[idx++] = (vec_store[i] > ((float*)&vec_b)[i]) ? 1 : 0;
    }
    
    /* Double precision vectors */
    v2df vec_da = {nan, 2.0};
    v2df vec_db = {1.0, nan};
    v2df vec_dcmp = vec_da < vec_db;
    
    /* ====== 6. Inline assembly with explicit condition codes ====== */
    
    /* Using ucomisd instruction */
    double asm_a = nan;
    double asm_b = inf;
    int asm_result = 0;
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    results[idx++] = asm_result;
    
    /* Another with different condition */
    asm_result = 0;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    results[idx++] = asm_result;
    
    /* ====== Accumulate all results ====== */
    
    for (int i = 0; i < idx; i++) {
        accumulate(results[i]);
    }
}

int main(void) {
#ifdef __x86_64__ || __i386__
    printf("Running x86 unordered comparison tests...\n");
    
    /* Run multiple times to ensure coverage */
    for (int i = 0; i < 3; i++) {
        test_unordered_comparisons();
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
#else
    printf("Non-x86 target - minimal test\n");
    volatile double x = 1.0;
    volatile double y = 2.0;
    printf("Simple comparison: %d\n", x < y ? 1 : 0);
    return 0;
#endif
}
