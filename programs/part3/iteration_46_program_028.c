/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double __attribute__((weak));
extern volatile float external_float __attribute__((weak));

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static volatile double values[] = {1.0, -1.0, 0.0, 3.14, -2.71};
    return values[idx % 5];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static volatile float values[] = {1.0f, -1.0f, 0.0f, 2.5f, -3.5f};
    return values[idx % 5];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile static int sink;
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    volatile double vd1 = 1.0;
    volatile double vd2 = -1.0;
    volatile float vf1 = 2.0f;
    volatile float vf2 = -2.0f;
    
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - may appear with NaN inputs or explicit checks */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;  /* unord */
    }
    
    /* ORDERED - opposite of unordered */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;  /* ord */
    }
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, == may use UNEQ */
        checksum += 3;
    }
    
    /* UNGE - unordered or greater-or-equal */
    if (vd1 >= vd2) {
        checksum += 4;  /* nlt */
    }
    
    /* UNGT - unordered or greater-than */
    if (d1 > d2) {
        checksum += 5;  /* nle */
    }
    
    /* UNLE - unordered or less-or-equal */
    if (f1 <= f2) {
        checksum += 6;  /* ule */
    }
    
    /* UNLT - unordered or less-than */
    if (vf1 < vf2) {
        checksum += 7;  /* ult */
    }
    
    /* LTGT - less-than or greater-than (not equal, not unordered) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 8;  /* une */
    }
    
    /* 2. More complex comparisons in conditional expressions */
    double d3 = (d1 != d2) ? d1 : d2;  /* != may generate UNEQ or LTGT */
    checksum += (int)(d3 * 10);
    
    float f3 = (f1 >= f2) ? f1 : f2;  /* >= may generate UNGE */
    checksum += (int)(f3 * 10);
    
    /* 3. Vector comparisons using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);   /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);  /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);   /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);  /* May use UNGE */
    
    /* Extract results to affect checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 100 + i;
    }
    
    /* 4. Vector comparisons using x86 intrinsics */
    __m128 m128_a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 m128_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Explicit unordered comparison - directly targets UNORDERED */
    __m128 cmp_unord = _mm_cmpunord_ps(m128_a, m128_b);
    
    /* Ordered comparison */
    __m128 cmp_ord = _mm_cmpord_ps(m128_a, m128_b);
    
    /* Not-equal (ordered, not equal) - may use LTGT */
    __m128 cmp_neq = _mm_cmpneq_ps(m128_a, m128_b);
    
    /* Extract mask bits */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq = _mm_movemask_ps(cmp_neq);
    
    checksum += mask_unord + mask_ord * 10 + mask_neq * 100;
    
    /* 5. Double precision vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    v2df cmp_d_eq = (vec_da == vec_db);
    v2df cmp_d_lt = (vec_da < vec_db);
    v2df cmp_d_le = (vec_da <= vec_db);
    
    double* d_eq_ptr = (double*)&cmp_d_eq;
    for (int i = 0; i < 2; i++) {
        if (d_eq_ptr[i] != 0.0) checksum += 200 + i;
    }
    
    /* 6. Loop with floating-point condition */
    for (int i = 0; i < 10 && (get_float_input(i) != 0.0f); i++) {
        checksum += i * 10;
    }
    
    /* 7. Switch based on comparison results */
    int cmp_result = 0;
    cmp_result |= (d1 < d2) ? 1 : 0;
    cmp_result |= (f1 > f2) ? 2 : 0;
    cmp_result |= (d1 == d2) ? 4 : 0;
    cmp_result |= (f1 != f2) ? 8 : 0;
    
    switch (cmp_result & 3) {
        case 0: checksum += 1000; break;
        case 1: checksum += 2000; break;
        case 2: checksum += 3000; break;
        case 3: checksum += 4000; break;
    }
    
    /* 8. Conditional move based on FP comparison */
    double final_result;
    if (vd1 >= vd2) {          /* UNGE */
        final_result = vd1 * 2.0;
    } else if (vf1 <= vf2) {   /* UNLE */
        final_result = vf1 * 3.0;
    } else if (d1 != d2) {     /* UNEQ or LTGT */
        final_result = d1 - d2;
    } else {
        final_result = 0.0;
    }
    
    checksum += (int)final_result;
    
    /* Ensure all code paths are potentially reachable */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
