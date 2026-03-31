#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection for x86 */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to accumulate results */
static void accumulate(int val) {
    checksum ^= val;
    checksum += 1;
}

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    /* Volatile to prevent optimization */
    volatile double nan_d = __builtin_nan("");
    volatile double inf_d = __builtin_inf();
    volatile double zero_d = 0.0;
    volatile double one_d = 1.0;
    volatile double neg_inf_d = -__builtin_inf();
    
    volatile float nan_f = __builtin_nanf("");
    volatile float inf_f = __builtin_inff();
    volatile float zero_f = 0.0f;
    
    volatile long double nan_ld = __builtin_nanl("");
    volatile long double inf_ld = __builtin_infl();
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators */
    /* These should generate various condition codes */
    
    /* UNORDERED case: nan < inf (unordered) */
    if (nan_d < inf_d) {
        results[idx++] = 1;  /* false branch */
    } else {
        results[idx++] = 2;  /* true branch - UNORDERED */
    }
    
    /* UNORDERED case: nan == nan */
    if (nan_d == nan_d) {
        results[idx++] = 3;  /* false branch */
    } else {
        results[idx++] = 4;  /* true branch - UNORDERED */
    }
    
    /* ORDERED case: inf > zero (ordered) */
    if (inf_d > zero_d) {
        results[idx++] = 5;  /* true branch - ORDERED */
    } else {
        results[idx++] = 6;
    }
    
    /* UNEQ case: !(nan < inf) && !(nan > inf) */
    if (!(nan_d < inf_d) && !(nan_d > inf_d)) {
        results[idx++] = 7;  /* true branch - UNEQ */
    } else {
        results[idx++] = 8;
    }
    
    /* LTGT case: (inf != nan) */
    if (inf_d != nan_d) {
        results[idx++] = 9;  /* true branch - LTGT */
    } else {
        results[idx++] = 10;
    }
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to the condition codes */
    
    /* __builtin_isunordered - UNORDERED */
    if (__builtin_isunordered(nan_d, inf_d)) {
        results[idx++] = 11;
    } else {
        results[idx++] = 12;
    }
    
    /* __builtin_islessgreater - LTGT */
    if (__builtin_islessgreater(inf_d, nan_d)) {
        results[idx++] = 13;
    } else {
        results[idx++] = 14;
    }
    
    /* __builtin_isless - UNLT */
    if (__builtin_isless(nan_d, inf_d)) {
        results[idx++] = 15;  /* false - unordered */
    } else {
        results[idx++] = 16;  /* true - UNLT/UNORDERED */
    }
    
    /* __builtin_isgreater - UNGT */
    if (__builtin_isgreater(inf_d, nan_d)) {
        results[idx++] = 17;  /* true - UNGT */
    } else {
        results[idx++] = 18;
    }
    
    /* __builtin_islessequal - UNLE */
    if (__builtin_islessequal(nan_d, inf_d)) {
        results[idx++] = 19;  /* false - unordered */
    } else {
        results[idx++] = 20;  /* true - UNLE/UNORDERED */
    }
    
    /* __builtin_isgreaterequal - UNGE */
    if (__builtin_isgreaterequal(inf_d, nan_d)) {
        results[idx++] = 21;  /* true - UNGE */
    } else {
        results[idx++] = 22;
    }
    
    /* 3. Complex expressions with mixed types */
    /* Create NaN through arithmetic */
    volatile double nan_arith = inf_d - inf_d;
    volatile double complex_expr = (inf_d / zero_d) * 0.0;
    
    /* UNORDERED with arithmetic-generated NaN */
    if (nan_arith == 0.0) {
        results[idx++] = 23;
    } else {
        results[idx++] = 24;  /* UNORDERED */
    }
    
    /* Mixed float/double comparisons */
    if (nan_f < inf_d) {
        results[idx++] = 25;
    } else {
        results[idx++] = 26;  /* UNORDERED */
    }
    
    /* Long double comparisons */
    if (nan_ld > inf_ld) {
        results[idx++] = 27;
    } else {
        results[idx++] = 28;  /* UNORDERED */
    }
    
    /* 4. Ternary operators with unordered comparisons */
    int ternary_result = (nan_d != nan_d) ? 29 : 30;
    results[idx++] = ternary_result;  /* Should be 30 (UNORDERED) */
    
    ternary_result = (inf_d > neg_inf_d) ? 31 : 32;
    results[idx++] = ternary_result;  /* Should be 31 (ORDERED) */
    
    /* 5. Control flow based on unordered results */
    /* Switch statement driven by comparison results */
    int switch_val = 0;
    if (__builtin_isunordered(nan_d, zero_d)) switch_val |= 1;
    if (__builtin_islessgreater(one_d, nan_d)) switch_val |= 2;
    if (!__builtin_isless(nan_d, one_d)) switch_val |= 4;
    
    switch (switch_val) {
        case 0:
            results[idx++] = 33;
            break;
        case 1:
            results[idx++] = 34;  /* UNORDERED */
            break;
        case 3:
            results[idx++] = 35;  /* UNORDERED + LTGT */
            break;
        case 5:
            results[idx++] = 36;  /* UNORDERED + UNLT */
            break;
        default:
            results[idx++] = 37;
            break;
    }
    
    /* 6. Vector comparisons using GCC extensions */
    v4sf vec_a = {nan_f, inf_f, zero_f, 1.0f};
    v4sf vec_b = {inf_f, nan_f, 1.0f, zero_f};
    
    /* Vector comparison generates multiple condition checks */
    v4sf vec_cmp = vec_a > vec_b;
    
    /* Extract comparison results - forces scalar code gen */
    float vec_results[4];
    memcpy(vec_results, &vec_cmp, sizeof(vec_results));
    
    for (int i = 0; i < 4; i++) {
        results[idx++] = vec_results[i] != 0.0f ? 38 + i : 42 + i;
    }
    
    /* Double precision vector */
    v2df vec_da = {nan_d, inf_d};
    v2df vec_db = {inf_d, nan_d};
    v2df vec_dcmp = vec_da < vec_db;
    
    double dvec_results[2];
    memcpy(dvec_results, &vec_dcmp, sizeof(dvec_results));
    
    for (int i = 0; i < 2; i++) {
        results[idx++] = dvec_results[i] != 0.0 ? 46 + i : 48 + i;
    }
    
    /* 7. Inline assembly with explicit condition codes */
    /* These directly use the condition mnemonics */
    double a = nan_d;
    double b = inf_d;
    int asm_result;
    
    /* UNORDERED test with ucomisd */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    results[idx++] = asm_result ? 50 : 51;
    
    /* ORDERED test */
    a = one_d;
    b = zero_d;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    results[idx++] = asm_result ? 52 : 53;
    
    /* LTGT test (unequal and ordered) */
    a = one_d;
    b = zero_d;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    results[idx++] = asm_result ? 54 : 55;
    
    /* 8. FMA operations that might produce NaN */
    volatile double fma_nan = __builtin_fma(inf_d, one_d, -inf_d);
    if (fma_nan == fma_nan) {
        results[idx++] = 56;
    } else {
        results[idx++] = 57;  /* UNORDERED */
    }
    
    /* 9. Loop with unordered comparison condition */
    for (int i = 0; i < 3; i++) {
        volatile double x = (i == 0) ? nan_d : (i == 1) ? inf_d : one_d;
        volatile double y = (i == 0) ? inf_d : (i == 1) ? nan_d : zero_d;
        
        if (__builtin_isunordered(x, y)) {
            results[idx++] = 58 + i;
        } else if (__builtin_islessgreater(x, y)) {
            results[idx++] = 61 + i;
        } else if (!__builtin_isless(x, y)) {
            results[idx++] = 64 + i;
        } else {
            results[idx++] = 67 + i;
        }
    }
    
    /* Accumulate all results to prevent elimination */
    for (int i = 0; i < idx; i++) {
        accumulate(results[i]);
    }
}

#else
/* Non-x86 fallback */
void test_unordered_comparisons(void) {
    printf("Not an x86 target - skipping unordered comparison tests\n");
}
#endif

int main(void) {
    test_unordered_comparisons();
    
    /* Print checksum to ensure code isn't eliminated */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
