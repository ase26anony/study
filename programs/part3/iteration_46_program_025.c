/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return external_double;
}

float __attribute__((noinline)) get_float_input(void) {
    return external_float;
}

/* Dummy function to create side effects */
void __attribute__((noinline)) use_result(int val) {
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(val) : "memory");
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    /* Get dynamic values */
    double d1 = get_double_input();
    double d2 = d1 + 1.0;
    float f1 = get_float_input();
    float f2 = f1 + 1.0f;
    
    /* ===== SCALAR FLOATING-POINT COMPARISONS ===== */
    
    /* 1. UNORDERED - Test with potential NaN values */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;  /* May generate "unord" */
    }
    
    /* 2. ORDERED - Inverse of unordered */
    if (__builtin_islessgreater(d1, d2)) {  /* This is LTGT */
        checksum += 2;
    }
    
    /* 3. UNEQ - Unordered or equal */
    if (!(d1 == d2)) {  /* With -ffast-math, may generate UNEQ */
        checksum += 4;
    }
    
    /* 4. UNGE - Unordered or greater than or equal */
    if (vd1 >= vd2) {  /* May generate "nlt" (not less than) */
        checksum += 8;
    }
    
    /* 5. UNGT - Unordered or greater than */
    if (d1 > d2) {  /* May generate "nle" (not less than or equal) */
        checksum += 16;
    }
    
    /* 6. UNLE - Unordered or less than or equal */
    if (f1 <= f2) {  /* May generate "ule" */
        checksum += 32;
    }
    
    /* 7. UNLT - Unordered or less than */
    if (vf1 < vf2) {  /* May generate "ult" */
        checksum += 64;
    }
    
    /* 8. LTGT - Less than or greater than (ordered and not equal) */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;  /* Should generate "une" */
    }
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (d1 != d2) ? d1 : d2;  /* May use UNEQ or LTGT */
    checksum += (int)cmov_result;
    
    float cmov_float = (f1 >= f2) ? f1 : f2;  /* May use UNGE */
    checksum += (int)cmov_float;
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons - these often generate condition codes */
    v4sf vec_cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf vec_cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf vec_cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf vec_cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf vec_cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf vec_cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to affect checksum */
    float extract_result = vec_cmp_eq[0] + vec_cmp_neq[1] + 
                          vec_cmp_lt[2] + vec_cmp_le[3];
    checksum += (int)extract_result;
    
    /* Double vector comparisons */
    v2df dbl_cmp = (vec_da > vec_db);        /* May use UNGT */
    checksum += (int)dbl_cmp[0];
    
    /* ===== USING INTRINSICS FOR EXPLICIT UNORDERED CHECKS ===== */
    
#ifdef __SSE__
    __m128 mm_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 mm_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* Explicit unordered comparison */
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);  /* Direct unordered test */
    
    /* Compare with different predicates */
    __m128 mm_eq = _mm_cmpeq_ps(mm_a, mm_b);
    __m128 mm_lt = _mm_cmplt_ps(mm_a, mm_b);
    __m128 mm_le = _mm_cmple_ps(mm_a, mm_b);
    __m128 mm_gt = _mm_cmpgt_ps(mm_a, mm_b);
    __m128 mm_ge = _mm_cmpge_ps(mm_a, mm_b);
    __m128 mm_neq = _mm_cmpneq_ps(mm_a, mm_b);
    
    /* Extract mask to affect checksum */
    int mask = _mm_movemask_ps(mm_unord);
    checksum += mask;
#endif
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    float arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = (float)i + f1;
    }
    
    /* Loop condition with FP comparison */
    for (int i = 0; i < 10 && (arr[i] != 0.0f); i++) {
        checksum += i;  /* May generate UNEQ in loop condition */
    }
    
    /* ===== SWITCH-LIKE BEHAVIOR WITH FP COMPARISONS ===== */
    
    /* Multiple comparisons in sequence */
    if (d1 < d2) checksum += 256;
    if (d1 > d2) checksum += 512;
    if (d1 == d2) checksum += 1024;
    if (d1 != d2) checksum += 2048;
    if (d1 <= d2) checksum += 4096;
    if (d1 >= d2) checksum += 8192;
    
    /* Mixed float/double comparisons */
    if ((double)f1 < d2) checksum += 16384;
    if (f1 > (float)d2) checksum += 32768;
    
    /* ===== FINAL OUTPUT ===== */
    
    printf("Checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;  /* Return non-constant result */
}

/* External volatile definitions to prevent optimization */
volatile double external_double = 3.14159;
volatile float external_float = 2.71828f;
