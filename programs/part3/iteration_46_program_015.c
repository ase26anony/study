/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double(void) {
    return external_double;
}

float __attribute__((noinline)) get_float(void) {
    return external_float;
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
    
    /* Initialize with mix of sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double();
    double d2 = 3.14159;
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_float();
    float f2 = 4.2f;
    
    /* 1. Standard floating-point comparisons with -ffast-math */
    
    /* UNORDERED - should generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* ORDERED - should generate "ord" */
    if (!__builtin_isunordered(vd1, vd2)) {
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* UNGE - unordered or greater-or-equal */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - unordered or greater-than */
    if (d1 > d2) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less-or-equal */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* UNLT - unordered or less-than */
    if (vf1 < vf2) {
        checksum += 64;
    }
    
    /* LTGT - less or greater (unordered excluded) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* 2. Built-in functions for explicit condition codes */
    checksum += __builtin_isunordered(f1, f2) ? 256 : 0;
    checksum += __builtin_islessgreater(vf1, vf2) ? 512 : 0;
    
    /* 3. Conditional moves based on FP comparisons */
    double cmov_result = (d1 != d2) ? d1 : d2;  /* May use UNEQ or LTGT */
    float cmov_float = (f1 >= f2) ? f1 : f2;    /* May use UNGE */
    
    checksum += (int)cmov_result;
    checksum += (int)cmov_float;
    
    /* 4. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to affect checksum */
    for (int i = 0; i < 4; i++) {
        checksum += cmp_eq[i] ? 1024 : 0;
        checksum += cmp_neq[i] ? 2048 : 0;
        checksum += cmp_lt[i] ? 4096 : 0;
        checksum += cmp_gt[i] ? 8192 : 0;
        checksum += cmp_le[i] ? 16384 : 0;
        checksum += cmp_ge[i] ? 32768 : 0;
    }
    
    /* 5. SSE intrinsics for explicit unordered comparisons */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 unord_mask = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Store to prevent optimization */
    float unord_store[4];
    _mm_store_ps(unord_store, unord_mask);
    checksum += (int)unord_store[0];
    
    /* 6. Loop with FP condition */
    volatile float arr[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    for (int i = 0; i < 4 && (arr[i] != 0.0f); ++i) {
        checksum += i * 65536;
    }
    
    /* 7. Switch based on FP comparison results */
    int fp_case = 0;
    fp_case += (d1 < d2) ? 1 : 0;
    fp_case += (d1 > d2) ? 2 : 0;
    fp_case += (d1 == d2) ? 4 : 0;
    fp_case += (d1 != d2) ? 8 : 0;
    
    switch (fp_case) {
        case 1: checksum += 1; break;  /* UNLT */
        case 2: checksum += 2; break;  /* UNGT */
        case 4: checksum += 4; break;  /* UNEQ */
        case 8: checksum += 8; break;  /* LTGT */
        default: checksum += 16; break;
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = 42;
    if (f1 < int_val) {    /* May generate UNLT */
        checksum += 32;
    }
    if (d1 >= int_val) {   /* May generate UNGE */
        checksum += 64;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Condition code checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* External volatile definitions to prevent optimization */
volatile double external_double = 2.71828;
volatile float external_float = 1.61803f;
