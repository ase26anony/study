/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_input_double(int idx) {
    static volatile double values[] = {1.0, 2.0, 0.0, -1.0, 3.14, -2.71};
    return values[idx % 6];
}

float __attribute__((noinline)) get_input_float(int idx) {
    static volatile float values[] = {1.0f, 2.0f, 0.0f, -1.0f, 3.14f, -2.71f};
    return values[idx % 6];
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
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_input_double(0);
    double d2 = get_input_double(1);
    float f1 = get_input_float(0);
    float f2 = get_input_float(1);
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - may be generated with NaN inputs or explicit check */
    if (__builtin_isunordered(d1, d2)) checksum += 1;
    
    /* ORDERED - opposite of unordered */
    if (!__builtin_isunordered(f1, f2)) checksum += 2;
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) checksum += 4;  /* With -ffast-math may use UNEQ */
    
    /* UNGE - unordered or greater than or equal */
    if (vd1 >= vd2) checksum += 8;
    if (d1 >= d2) checksum += 16;
    
    /* UNGT - unordered or greater than */
    if (vd1 > vd2) checksum += 32;
    if (d1 > d2) checksum += 64;
    
    /* UNLE - unordered or less than or equal */
    if (vf1 <= vf2) checksum += 128;
    if (f1 <= f2) checksum += 256;
    
    /* UNLT - unordered or less than */
    if (vf1 < vf2) checksum += 512;
    if (f1 < f2) checksum += 1024;
    
    /* LTGT - less than or greater than (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) checksum += 2048;
    if (d1 != d2) checksum += 4096;  /* With -ffast-math may use LTGT */
    
    /* 2. More complex conditional expressions */
    double cond_result = (d1 == d2) ? d1 : d2;  /* May use UNEQ */
    checksum += (int)cond_result;
    
    float cond_result2 = (f1 >= f2) ? f1 : f2;  /* May use UNGE */
    checksum += (int)cond_result2;
    
    /* 3. Vector comparisons using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate mask registers that use condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 8192;
    }
    
    /* 4. Vector comparisons using x86 intrinsics */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* Explicit unordered comparison - should generate UNORDERED condition */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Ordered comparison */
    __m128 cmp_ord = _mm_cmpord_ps(sse_a, sse_b);
    
    /* Extract mask from unordered comparison */
    int unord_mask = _mm_movemask_ps(cmp_unord);
    checksum += unord_mask;
    
    /* 5. Double precision vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    v2df cmp_dneq = (vec_da != vec_db);  /* May use LTGT */
    v2df cmp_dle = (vec_da <= vec_db);   /* May use UNLE */
    
    double* dneq_ptr = (double*)&cmp_dneq;
    if (dneq_ptr[0] != 0.0) checksum += 16384;
    
    /* 6. Loop with floating-point condition */
    for (int i = 0; i < 10 && (get_input_float(i) != 0.0f); ++i) {
        checksum += i * 1000;
    }
    
    /* 7. Switch based on comparison results */
    int cmp_case = 0;
    if (d1 < d2) cmp_case = 1;      /* UNLT */
    else if (d1 > d2) cmp_case = 2; /* UNGT */
    else if (d1 == d2) cmp_case = 3; /* UNEQ */
    
    switch (cmp_case) {
        case 1: checksum += 100000; break;
        case 2: checksum += 200000; break;
        case 3: checksum += 300000; break;
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = 5;
    if (d1 < int_val) checksum += 400000;   /* May use different pattern */
    if (int_val > f1) checksum += 500000;
    
    /* 9. Additional builtin checks */
    checksum += __builtin_isunordered(f1, f2) ? 600000 : 0;
    checksum += __builtin_islessgreater(f1, f2) ? 700000 : 0;
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
