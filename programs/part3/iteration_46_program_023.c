/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double __attribute__((weak));
extern volatile float external_float __attribute__((weak));

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.5;
    counter = counter * 1.1 + 0.1;
    return counter;
}

float get_input_float(void) {
    static float counter = 0.3f;
    counter = counter * 1.2f + 0.05f;
    return counter;
}

/* Dummy function to prevent optimization */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Create side effect */
    static volatile int sink;
    sink = val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* 1. Standard floating-point comparisons with -ffast-math */
    
    /* UNORDERED - should generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* ORDERED - should generate "ord" */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* UNGE - not less than (unordered or greater/equal) */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - not less or equal (unordered or greater) */
    if (d1 > d2) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less/equal */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* UNLT - unordered or less than */
    if (vd1 < vd2) {
        checksum += 64;
    }
    
    /* LTGT - less or greater (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* 2. Built-in functions for explicit condition codes */
    checksum += __builtin_isunordered(f1, f2) ? 256 : 0;
    checksum += __builtin_islessgreater(f1, f2) ? 512 : 0;
    
    /* 3. Conditional moves based on FP comparisons */
    double cmov_result = (d1 != d2) ? d1 : d2;  /* May use UNEQ or LTGT */
    float cmov_float = (f1 >= f2) ? f1 : f2;    /* May use UNGE */
    checksum += (int)(cmov_result + cmov_float);
    
    /* 4. Vector (SIMD) comparisons */
    v4sf vec_a = {f1, f2, 3.14f, -2.5f};
    v4sf vec_b = {f2, f1, 3.14f, 0.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results from vector comparisons */
    int vec_mask = 0;
    for (int i = 0; i < 4; i++) {
        vec_mask += (cmp_eq[i] != 0.0f) ? (1 << i) : 0;
        vec_mask += (cmp_neq[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    checksum += vec_mask;
    
    /* 5. SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(f1, f2, 1.0f, 2.0f);
    __m128 sse_b = _mm_set_ps(f2, f1, 2.0f, 1.0f);
    __m128 sse_cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    __m128 sse_cmp_ord = _mm_cmpord_ps(sse_a, sse_b);      /* Direct ORDERED */
    
    /* Use the results */
    float sse_result[4];
    _mm_storeu_ps(sse_result, sse_cmp_unord);
    checksum += (int)(sse_result[0] + sse_result[1]);
    
    /* 6. Loop with FP condition */
    double arr[4] = {d1, d2, 3.14, -1.0};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {  /* May use UNEQ/LTGT */
        checksum += i * 10;
    }
    
    /* 7. Switch based on comparison results */
    int cmp_case = 0;
    if (__builtin_isunordered(d1, d2)) cmp_case = 1;
    else if (d1 == d2) cmp_case = 2;
    else if (d1 < d2) cmp_case = 3;
    else if (d1 > d2) cmp_case = 4;
    
    switch (cmp_case) {
        case 1: checksum += 1000; break;  /* UNORDERED */
        case 2: checksum += 2000; break;  /* UNEQ */
        case 3: checksum += 3000; break;  /* UNLT */
        case 4: checksum += 4000; break;  /* UNGT */
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = checksum % 10;
    if ((float)int_val != f1) {  /* Mixed type comparison */
        checksum += 5000;
    }
    
    /* 9. Double precision vector comparisons */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    v2df dvec_cmp = (dvec_a > dvec_b);  /* May use UNGT */
    checksum += (int)(dvec_cmp[0] + dvec_cmp[1]);
    
    /* 10. Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum > 0 ? 0 : 1;
}
