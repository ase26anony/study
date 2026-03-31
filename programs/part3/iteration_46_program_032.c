/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.1;
    counter += 0.3;
    return counter;
}

float get_input_float(void) {
    static float counter = 0.2f;
    counter += 0.4f;
    return counter;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile int dummy = val;
    (void)dummy;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 3.0f;
    volatile float vf2 = 4.0f;
    
    /* Dynamic values */
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* Mixed integer/float comparisons */
    int int_val = 5;
    
    /* ========== SCALAR FLOATING-POINT COMPARISONS ========== */
    
    /* 1. UNORDERED - should generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* 2. ORDERED - should generate "ord" */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* 3. UNEQ - unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* 4. UNGE - unordered or greater-or-equal */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* 5. UNGT - unordered or greater-than */
    if (d1 > d2) {
        checksum += 16;
    }
    
    /* 6. UNLE - unordered or less-or-equal */
    if (f1 <= f2) {
        checksum += 32;
    }
    
    /* 7. UNLT - unordered or less-than */
    if (vf1 < vf2) {
        checksum += 64;
    }
    
    /* 8. LTGT - less-than or greater-than (not equal, not unordered) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* Not equal comparison - may use UNEQ or LTGT depending on context */
    if (d1 != d2) {
        checksum += 256;
    }
    
    /* Mixed type comparisons */
    if (d1 < int_val) {
        checksum += 512;
    }
    
    if (f1 >= int_val) {
        checksum += 1024;
    }
    
    /* Conditional move based on FP comparison */
    double cond_result = (d1 <= d2) ? d1 : d2;  /* May use UNLE */
    checksum += (int)(cond_result * 100);
    
    /* ========== VECTOR (SIMD) COMPARISONS ========== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons - these generate mask results */
    v4sf vec_cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf vec_cmp_neq = (vec_a != vec_b);   /* May use LTGT or UNEQ */
    v4sf vec_cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf vec_cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf vec_cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf vec_cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to affect checksum */
    float* feq = (float*)&vec_cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (feq[i] != 0.0f) checksum += 1 << i;
    }
    
    /* Double vector comparisons */
    v2df vec_dcmp = (vec_da > vec_db);     /* May use UNGT */
    double* dgt = (double*)&vec_dcmp;
    if (dgt[0] != 0.0) checksum += 2048;
    if (dgt[1] != 0.0) checksum += 4096;
    
    /* Use SSE intrinsics for explicit unordered check */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct unordered */
    
    /* ========== LOOP WITH FP CONDITION ========== */
    
    /* Loop with floating-point condition */
    double arr[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    for (int i = 0; i < 5 && (arr[i] != 0.0); ++i) {
        checksum += i * 10;
    }
    
    /* Switch based on comparison results */
    int cmp_result = 0;
    if (d1 < d2) cmp_result = 1;
    else if (d1 > d2) cmp_result = 2;
    else if (d1 == d2) cmp_result = 3;
    
    switch (cmp_result) {
        case 1: checksum += 8192; break;  /* UNLT */
        case 2: checksum += 16384; break; /* UNGT */
        case 3: checksum += 32768; break; /* UNEQ */
    }
    
    /* ========== FINAL OUTPUT ========== */
    
    /* Create observable output */
    printf("Condition code checksum: %d\n", checksum);
    
    /* Use all results to prevent dead code elimination */
    use_result(checksum);
    use_result((int)cond_result);
    
    /* Access volatile variables */
    external_double = d1;
    external_float = f1;
    
    return checksum & 0xFF;  /* Return non-constant */
}
