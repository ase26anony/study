/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static const double values[] = {1.0, -1.0, 0.0, 3.14, -2.71, 0.0/0.0};
    return values[idx % 6];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static const float values[] = {1.0f, -1.0f, 0.0f, 3.14f, -2.71f, 0.0f/0.0f};
    return values[idx % 6];
}

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with -ffast-math */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - triggered by NaN comparisons */
    if (d1 != d1) checksum |= 1;  /* NaN != NaN is true */
    if (!(d1 == d1)) checksum |= 2; /* NaN == NaN is false */
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) checksum += 4;  /* May generate UNEQ with -ffast-math */
    
    /* UNGE - unordered or greater-or-equal */
    if (vd1 >= d2) checksum += 8;
    
    /* UNGT - unordered or greater-than */
    if (d1 > vd2) checksum += 16;
    
    /* UNLE - unordered or less-or-equal */
    if (f1 <= f2) checksum += 32;
    
    /* UNLT - unordered or less-than */
    if (vf1 < f2) checksum += 64;
    
    /* LTGT - less-than or greater-than (ordered and not equal) */
    if (d1 != d2) checksum += 128;  /* May generate LTGT with -ffast-math */
    
    /* ORDERED - both operands are ordered (not NaN) */
    if (d1 == d1 && d2 == d2) checksum += 256;
    
    /* 2. Explicit built-in functions for condition codes */
    checksum += __builtin_isunordered(d1, d2) ? 512 : 0;
    checksum += __builtin_islessgreater(d1, d2) ? 1024 : 0;
    checksum += __builtin_islessequal(f1, f2) ? 2048 : 0;
    checksum += __builtin_isgreaterequal(vd1, d2) ? 4096 : 0;
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* cmp_ptr = (float*)&cmp_eq;
    checksum += (cmp_ptr[0] != 0.0f) ? 8192 : 0;
    checksum += (cmp_ptr[1] != 0.0f) ? 16384 : 0;
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result1 = (d1 > d2) ? d1 : d2;      /* May use UNGT */
    float cmov_result2 = (f1 <= f2) ? f1 : f2;      /* May use UNLE */
    checksum += (int)(cmov_result1 + cmov_result2);
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (get_double_input(i) != 0.0); ++i) {
        checksum += i;  /* May generate LTGT in loop condition */
    }
    
    /* Switch with FP-derived values */
    int fp_switch = (d1 < d2) ? 1 : ((d1 > d2) ? 2 : 3);
    switch (fp_switch) {
        case 1: checksum += 32768; break;  /* UNLT */
        case 2: checksum += 65536; break;  /* UNGT */
        case 3: checksum += 131072; break; /* UNEQ */
    }
    
    /* 5. Mixed integer/float comparisons */
    int int_val = 5;
    if (d1 > int_val) checksum += 262144;    /* Mixed comparison */
    if (int_val < f2) checksum += 524288;    /* Reverse mixed comparison */
    
    /* 6. Prevent optimization through external use */
    use_result(checksum);
    
    /* Final observable output */
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
