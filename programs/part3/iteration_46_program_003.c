/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static const double values[] = {1.0, 2.0, 0.0, -1.0, 3.14, -2.71};
    return values[idx % 6];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static const float values[] = {1.0f, 2.0f, 0.0f, -1.0f, 3.14f, -2.71f};
    return values[idx % 6];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
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
    
    /* UNGE - unordered or greater than or equal */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - unordered or greater than */
    if (d1 > d2) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less than or equal */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* UNLT - unordered or less than */
    if (vf1 < vf2) {
        checksum += 64;
    }
    
    /* LTGT - less than or greater than (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* 2. More complex comparisons in conditional expressions */
    double d3 = (d1 != d2) ? d1 : d2;  /* May generate UNEQ or LTGT */
    checksum += (int)(d3 * 10);
    
    float f3 = (f1 >= f2) ? f1 : f2;  /* May generate UNGE */
    checksum += (int)(f3 * 10);
    
    /* 3. Vector comparisons using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector equality - may use UNEQ */
    v4sf cmp_eq = (vec_a == vec_b);
    int cmp_mask = __builtin_ia32_movmskps((__v4sf)cmp_eq);
    checksum += cmp_mask;
    
    /* Vector less than - may use UNLT */
    v4sf cmp_lt = (vec_a < vec_b);
    cmp_mask = __builtin_ia32_movmskps((__v4sf)cmp_lt);
    checksum += cmp_mask * 2;
    
    /* Vector greater than - may use UNGT */
    v4sf cmp_gt = (vec_a > vec_b);
    cmp_mask = __builtin_ia32_movmskps((__v4sf)cmp_gt);
    checksum += cmp_mask * 3;
    
    /* 4. Vector comparisons using x86 intrinsics */
    __m128 sse_a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Explicit unordered comparison - should generate UNORDERED */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    cmp_mask = _mm_movemask_ps(cmp_unord);
    checksum += cmp_mask * 4;
    
    /* Ordered comparison - should generate ORDERED */
    __m128 cmp_ord = _mm_cmpord_ps(sse_a, sse_b);
    cmp_mask = _mm_movemask_ps(cmp_ord);
    checksum += cmp_mask * 5;
    
    /* 5. Double precision vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    v2df cmp_dle = (vec_da <= vec_db);  /* May generate UNLE */
    long long dmask;
    __asm__("movmskpd %1, %0" : "=r"(dmask) : "x"(cmp_dle));
    checksum += (int)dmask * 6;
    
    /* 6. Loop with floating-point condition */
    for (int i = 0; i < 10 && (get_float_input(i) != 0.0f); ++i) {
        checksum += i * 10;
    }
    
    /* 7. Switch based on comparison results */
    int cmp_result = 0;
    cmp_result |= (d1 < d2) ? 1 : 0;   /* UNLT */
    cmp_result |= (d1 > d2) ? 2 : 0;   /* UNGT */
    cmp_result |= (d1 == d2) ? 4 : 0;  /* UNEQ */
    cmp_result |= (d1 != d2) ? 8 : 0;  /* LTGT or UNEQ */
    
    switch (cmp_result) {
        case 0: checksum += 1000; break;
        case 1: checksum += 2000; break;
        case 2: checksum += 3000; break;
        case 3: checksum += 4000; break;
        case 4: checksum += 5000; break;
        case 5: checksum += 6000; break;
        case 6: checksum += 7000; break;
        case 7: checksum += 8000; break;
        default: checksum += 9000; break;
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = 2;
    if (f1 < int_val) {  /* May generate different condition code */
        checksum += 10000;
    }
    
    if (d1 > int_val) {
        checksum += 20000;
    }
    
    /* 9. Additional builtin usage */
    checksum += __builtin_islessequal(f1, f2) ? 30000 : 0;    /* UNLE */
    checksum += __builtin_isgreaterequal(d1, d2) ? 40000 : 0; /* UNGE */
    checksum += __builtin_isless(f1, f2) ? 50000 : 0;         /* UNLT */
    checksum += __builtin_isgreater(d1, d2) ? 60000 : 0;      /* UNGT */
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum > 0 ? 0 : 1;
}
