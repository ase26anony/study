/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static const double values[] = {1.0, -1.0, 0.0, 2.5, -2.5, 3.14159};
    return values[idx % 6];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static const float values[] = {1.0f, -1.0f, 0.0f, 2.5f, -2.5f, 3.14159f};
    return values[idx % 6];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int result) {
    /* Create side effect */
    static int accumulator = 0;
    accumulator += result;
    if (accumulator > 1000000) accumulator = 0;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - Test for unordered (NaN) */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* ORDERED - Test for ordered (not NaN) */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* UNEQ - Unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* UNGE - Unordered or greater than or equal */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - Unordered or greater than */
    if (d1 > d2) {
        checksum += 16;
    }
    
    /* UNLE - Unordered or less than or equal */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* UNLT - Unordered or less than */
    if (vf1 < vf2) {
        checksum += 64;
    }
    
    /* LTGT - Less than or greater than (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* 2. Mixed integer/floating comparisons */
    int i = 42;
    if (d1 < i) {
        checksum += 256;
    }
    
    if (i >= f1) {
        checksum += 512;
    }
    
    /* 3. Conditional moves based on FP comparisons */
    double cmov_result = (d1 != d2) ? d1 : d2;  /* May use UNEQ or LTGT */
    checksum += (int)(cmov_result * 10);
    
    float fcmov_result = (f1 == f2) ? f1 : f2;  /* May use UNEQ */
    checksum += (int)(fcmov_result * 10);
    
    /* 4. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    v4sf cmp_ne = (vec_a != vec_b);    /* May use LTGT */
    
    /* Extract results from vector comparisons */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 1024;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED test */
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(sse_cmp);
    checksum += mask;
    
    /* 5. Double precision vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    v2df dbl_cmp = (vec_da > vec_db);  /* May use UNGT */
    double* dbl_ptr = (double*)&dbl_cmp;
    if (dbl_ptr[0] != 0.0) checksum += 2048;
    
    /* 6. Loop with FP condition */
    for (int j = 0; j < 10 && (get_float_input(j) != 0.0f); ++j) {
        checksum += j * 10;
    }
    
    /* 7. Switch based on comparison results */
    int cmp_results = 0;
    cmp_results |= (d1 < d2) ? 1 : 0;
    cmp_results |= (d1 > d2) ? 2 : 0;
    cmp_results |= (d1 == d2) ? 4 : 0;
    cmp_results |= (d1 != d2) ? 8 : 0;
    
    switch (cmp_results & 3) {
        case 0: checksum += 4096; break;
        case 1: checksum += 8192; break;
        case 2: checksum += 16384; break;
        case 3: checksum += 32768; break;
    }
    
    /* 8. More builtin usage */
    checksum += __builtin_isunordered(f1, f2) ? 65536 : 0;
    checksum += __builtin_islessgreater(f1, f2) ? 131072 : 0;
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    /* Output result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
