/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding and optimization */
extern volatile double get_double_input(void) __attribute__((noinline));
extern volatile float get_float_input(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Opaque function to prevent constant propagation */
volatile double get_double_input(void) {
    static volatile double counter = 0.0;
    return counter += 1.23456789;
}

volatile float get_float_input(void) {
    static volatile float counter = 0.0f;
    return counter += 0.987654321f;
}

void use_result(int val) {
    /* Prevent dead code elimination */
    static volatile int sink;
    sink = val;
}

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = get_double_input();
    double d3 = 0.0 / 0.0;  /* Potential NaN with -ffast-math assumptions */
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_float_input();
    float f2 = get_float_input();
    
    /* 1. Standard floating-point comparisons with -ffast-math */
    /* These should generate various condition codes */
    
    /* UNORDERED: x unord y (either is NaN) */
    if (__builtin_isunordered(d1, d3)) {
        checksum += 1;
    }
    
    /* UNEQ: x == y or unordered */
    if (f1 != f2) {  /* With -ffast-math, may use UNEQ or LTGT */
        checksum += 2;
    }
    
    /* UNGE: !(x < y) (x >= y or unordered) */
    if (vd1 >= vd2) {
        checksum += 4;
    }
    
    /* UNGT: !(x <= y) (x > y or unordered) */
    if (d1 > d2) {
        checksum += 8;
    }
    
    /* UNLE: !(x > y) (x <= y or unordered) */
    if (vf1 <= vf2) {
        checksum += 16;
    }
    
    /* UNLT: !(x >= y) (x < y or unordered) */
    if (f1 < f2) {
        checksum += 32;
    }
    
    /* LTGT: x < y or x > y (but not equal and not unordered) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 64;
    }
    
    /* ORDERED: both are not NaN */
    if (__builtin_isordered(d1, d2)) {
        checksum += 128;
    }
    
    /* 2. Conditional moves based on FP comparisons */
    /* Force materialization of condition codes */
    double cmov_result = (d1 == d2) ? d1 : d2;  /* May use UNEQ */
    checksum += (int)(cmov_result * 10);
    
    float f_cmov = (f1 >= f2) ? f1 : f2;  /* May use UNGE */
    checksum += (int)(f_cmov * 20);
    
    /* 3. Vector (SIMD) comparisons */
    /* These often generate the target condition codes */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector comparisons - each generates mask based on condition */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_c);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_c);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_c);    /* May use UNGE */
    
    /* Extract results to affect checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_ptr[i] != 0.0f) ? 1 : 0;
    }
    
    /* 4. SSE intrinsics for explicit unordered comparisons */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q - unordered comparison */
    __m128 unord_mask = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* _CMP_NEQ_UQ - not equal or unordered */
    __m128 neq_uq_mask = _mm_cmpneq_ps(sse_a, sse_b);
    
    /* Extract mask bits */
    int mask = _mm_movemask_ps(unord_mask);
    checksum += mask;
    
    mask = _mm_movemask_ps(neq_uq_mask);
    checksum += mask;
    
    /* 5. Double precision vector comparisons */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    
    v2df dcmp = (dvec_a == dvec_b);  /* May use UNEQ */
    v2df dcmp_lt = (dvec_a < dvec_b); /* May use UNLT */
    
    double* dptr = (double*)&dcmp;
    checksum += (dptr[0] != 0.0) ? 100 : 0;
    checksum += (dptr[1] != 0.0) ? 200 : 0;
    
    /* 6. Loop with FP condition to generate branch with condition code */
    float arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 0.5f;
    }
    
    for (int i = 0; i < 10 && (arr[i] != 0.0f); i++) {
        checksum += i;  /* Condition may use UNEQ or LTGT */
    }
    
    /* 7. Switch based on multiple FP comparisons */
    /* Create multiple condition code emission sites */
    int fp_switch = 0;
    
    if (d1 < d2) fp_switch = 1;      /* UNLT */
    else if (d1 > d2) fp_switch = 2; /* UNGT */
    else if (d1 == d2) fp_switch = 3; /* UNEQ */
    else if (d1 != d2) fp_switch = 4; /* LTGT */
    
    switch (fp_switch) {
        case 1: checksum += 1000; break;
        case 2: checksum += 2000; break;
        case 3: checksum += 3000; break;
        case 4: checksum += 4000; break;
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = 5;
    if (f1 < int_val) {  /* May generate special pattern */
        checksum += 5000;
    }
    
    if (d1 > int_val) {
        checksum += 6000;
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum > 0 ? 0 : 1;
}
