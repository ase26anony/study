/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.5;
    counter = counter * 1.1 + 0.3;
    return counter;
}

float get_input_float(void) {
    static float counter = 0.3f;
    counter = counter * 1.2f + 0.1f;
    return counter;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink = 0;
    sink += val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Mixed float/double variables from different sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED/ORDERED patterns - likely with -ffast-math */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (vd1 <= vd2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (f1 >= f2) {  /* May generate UNGE */
        checksum += 4;
    }
    
    if (d1 < d2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    if (d1 > d2) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* Mixed type comparisons */
    if ((double)f1 == d2) {  /* May generate UNEQ */
        checksum += 32;
    }
    
    /* ===== BUILTIN FUNCTIONS FOR EXPLICIT CONDITION CODES ===== */
    
    /* Direct unordered check - should generate UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* Ordered check - should generate ORDERED */
    if (__builtin_isordered(f1, f2)) {
        checksum += 128;
    }
    
    /* Less-greater check - should generate LTGT */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 256;
    }
    
    /* ===== CONDITIONAL MOVES BASED ON FP COMPARISONS ===== */
    
    /* Conditional move with UNGE pattern */
    double cmov_result = (vd1 >= vd2) ? d1 : d2;
    checksum += (int)(cmov_result * 10);
    
    /* Conditional move with UNLE pattern */
    float f_cmov = (f1 <= f2) ? f1 : f2;
    checksum += (int)(f_cmov * 100);
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate mask predicates */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to affect checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 512;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 unord_mask = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Check if any element is unordered */
    int unord_test = _mm_movemask_ps(unord_mask);
    if (unord_test) {
        checksum += 1024;
    }
    
    /* _CMP_NEQ_UQ generates UNEQ condition */
    __m128 neq_mask = _mm_cmpneq_ps(sse_a, sse_b);
    int neq_test = _mm_movemask_ps(neq_mask);
    checksum += neq_test * 2;
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    /* Loop condition with FP comparison - prevents dead code elimination */
    double arr[4] = {d1, d2, vd1, vd2};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {
        checksum += i * 10;
    }
    
    /* ===== SWITCH BASED ON FP COMPARISON RESULTS ===== */
    
    /* Create multiple condition code emission sites */
    int fp_flags = 0;
    fp_flags |= (d1 < d2) ? 1 : 0;    /* UNLT */
    fp_flags |= (d1 > d2) ? 2 : 0;    /* UNGT */
    fp_flags |= (f1 == f2) ? 4 : 0;   /* UNEQ */
    fp_flags |= (f1 != f2) ? 8 : 0;   /* LTGT or UNEQ */
    
    switch (fp_flags & 3) {
        case 0: checksum += 2048; break;
        case 1: checksum += 4096; break;
        case 2: checksum += 8192; break;
        case 3: checksum += 16384; break;
    }
    
    /* ===== FINAL OUTPUT ===== */
    
    /* Use results to prevent elimination */
    use_result(checksum);
    
    printf("Condition code test checksum: %d\n", checksum);
    
    /* Return value depends on comparisons */
    return (checksum > 10000) ? 0 : 1;
}
