/* test_condcodes.c - Target x86 condition code mnemonics for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return external_double;
}

float __attribute__((noinline)) get_float_input(void) {
    return external_float;
}

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int result) {
    /* Create side effect */
    volatile int dummy = result;
    (void)dummy;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    /* Get dynamic values */
    double d1 = get_double_input();
    double d2 = d1 + 1.0;
    float f1 = get_float_input();
    float f2 = f1 + 1.0f;
    
    /* ============================================
       SCALAR FLOATING-POINT COMPARISONS WITH -ffast-math
       ============================================ */
    
    /* UNORDERED - Test for unordered (either NaN) */
    if (__builtin_isunordered(d1, d2)) {
        checksum |= 1;
    }
    
    /* ORDERED - Test for ordered (neither NaN) */
    if (__builtin_isless(d1, d2)) {  /* Implies ordered */
        checksum |= 2;
    }
    
    /* UNEQ - Unordered or equal */
    if (!(d1 == d2)) {  /* With -ffast-math, may generate UNEQ */
        checksum |= 4;
    }
    
    /* UNGE - Unordered or greater than or equal */
    if (d1 >= d2) {  /* May generate UNGE with -ffast-math */
        checksum |= 8;
    }
    
    /* UNGT - Unordered or greater than */
    if (d1 > d2) {  /* May generate UNGT */
        checksum |= 16;
    }
    
    /* UNLE - Unordered or less than or equal */
    if (f1 <= f2) {  /* Mix float types */
        checksum |= 32;
    }
    
    /* UNLT - Unordered or less than */
    if (f1 < f2) {
        checksum |= 64;
    }
    
    /* LTGT - Less than or greater than (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum |= 128;
    }
    
    /* More comparisons with mixed types and volatile */
    if (vd1 != vd2) {  /* != may use UNEQ or LTGT */
        checksum |= 256;
    }
    
    /* Conditional move based on FP comparison */
    double cmov_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)cmov_result;
    
    /* ============================================
       VECTOR (SIMD) COMPARISONS
       ============================================ */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons using GCC vector extensions */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to scalar checksum */
    for (int i = 0; i < 4; i++) {
        if (cmp_eq[i]) checksum += 512;
        if (cmp_lt[i]) checksum += 1024;
        if (cmp_le[i]) checksum += 2048;
        if (cmp_gt[i]) checksum += 4096;
        if (cmp_ge[i]) checksum += 8192;
    }
    
    /* Vector comparisons with double */
    v2df cmp_dneq = (vec_da != vec_db);  /* May use UNEQ or LTGT */
    if (cmp_dneq[0] || cmp_dneq[1]) {
        checksum |= 16384;
    }
    
    /* ============================================
       X86 INTRINSICS FOR EXPLICIT UNORDERED CHECKS
       ============================================ */
    
    /* Use SSE/AVX intrinsics for explicit unordered comparisons */
    __m128 mm_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 mm_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q - unordered comparison */
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);
    
    /* _CMP_NEQ_UQ - not equal or unordered */
    __m128 mm_neq_uq = _mm_cmpneq_ps(mm_a, mm_b);
    
    /* Extract mask from results */
    int mask_unord = _mm_movemask_ps(mm_unord);
    int mask_neq_uq = _mm_movemask_ps(mm_neq_uq);
    
    checksum += mask_unord + mask_neq_uq;
    
    /* ============================================
       LOOP WITH FP COMPARISON CONDITION
       ============================================ */
    
    /* Create a loop where the exit condition uses FP comparison */
    double arr[4] = {d1, d2, d1 + 2.0, d2 + 2.0};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {  /* != may use UNEQ */
        checksum += i * 100;
    }
    
    /* Switch based on FP comparison results */
    int case_selector = 0;
    if (d1 < d2) case_selector = 1;      /* May use UNLT */
    if (d1 == d2) case_selector = 2;     /* May use UNEQ */
    if (d1 > d2) case_selector = 3;      /* May use UNGT */
    
    switch (case_selector) {
        case 1: checksum += 10000; break;
        case 2: checksum += 20000; break;
        case 3: checksum += 30000; break;
        default: checksum += 40000; break;
    }
    
    /* ============================================
       FINAL OUTPUT TO PREVENT DEAD CODE ELIMINATION
       ============================================ */
    
    /* Use results to create observable output */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-constant value */
}

/* External volatile definitions to prevent optimization */
volatile double external_double = 3.14159;
volatile float external_float = 2.71828f;
