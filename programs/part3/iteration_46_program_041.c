/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_input_double(int idx) {
    static const double values[] = {1.0, 2.0, 0.0, -1.0, 3.14, -3.14};
    return values[idx % 6];
}

float __attribute__((noinline)) get_input_float(int idx) {
    static const float values[] = {1.0f, 2.0f, 0.0f, -1.0f, 3.14f, -3.14f};
    return values[idx % 6];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    /* Create side effect */
    static int accumulator = 0;
    accumulator += val;
    if (accumulator > 1000) printf("%d", accumulator);
}

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.5;
    volatile double vd2 = 2.5;
    double d1 = get_input_double(0);
    double d2 = get_input_double(1);
    float f1 = get_input_float(2);
    float f2 = get_input_float(3);
    
    /* 2. Perform relational operator comparisons with -ffast-math */
    
    /* UNORDERED/ORDERED patterns - using NaN-like behavior with fast-math */
    if (d1 != d1) checksum += 1;  /* May generate UNORDERED check */
    if (d1 == d1) checksum += 2;  /* May generate ORDERED check */
    
    /* UNEQ (unordered or equal) */
    if (d1 == d2) checksum += 4;  /* With -ffast-math, may use UNEQ */
    
    /* UNGE (not less than) */
    if (vd1 >= vd2) checksum += 8;
    if (d1 >= d2) checksum += 16;
    
    /* UNGT (not less than or equal) */
    if (vd1 > vd2) checksum += 32;
    if (d1 > d2) checksum += 64;
    
    /* UNLE (unordered or less than or equal) */
    if (f1 <= f2) checksum += 128;
    
    /* UNLT (unordered or less than) */
    if (f1 < f2) checksum += 256;
    
    /* LTGT (less than or greater than, i.e., not equal and not unordered) */
    if (d1 != d2) checksum += 512;  /* With -ffast-math, may use LTGT */
    
    /* 3. Explicit built-in function calls */
    checksum += __builtin_isunordered(d1, d2) ? 1024 : 0;
    checksum += __builtin_islessgreater(d1, d2) ? 2048 : 0;
    checksum += __builtin_islessequal(f1, f2) ? 4096 : 0;
    checksum += __builtin_isless(f1, f2) ? 8192 : 0;
    checksum += __builtin_isgreaterequal(vd1, vd2) ? 16384 : 0;
    checksum += __builtin_isgreater(vd1, vd2) ? 32768 : 0;
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)cmov_result;
    
    float f_cmov = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)f_cmov;
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_ptr[i] != 0.0f) ? 1 : 0;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    
    /* Double vector */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    v2df dvec_cmp = (dvec_a > dvec_b);  /* May use UNGT */
    
    /* 6. Loop with FP condition to generate branch with condition code */
    for (int i = 0; i < 10 && (get_input_double(i) != 0.0); i++) {
        checksum += i;  /* Loop condition may use LTGT */
    }
    
    /* Switch based on comparison results */
    int cmp_val = 0;
    cmp_val += (d1 < d2) ? 1 : 0;   /* UNLT */
    cmp_val += (d1 > d2) ? 2 : 0;   /* UNGT */
    cmp_val += (d1 == d2) ? 4 : 0;  /* UNEQ */
    
    switch (cmp_val) {
        case 0: checksum += 1000; break;  /* All false - unordered? */
        case 1: checksum += 2000; break;  /* d1 < d2 */
        case 2: checksum += 3000; break;  /* d1 > d2 */
        case 4: checksum += 4000; break;  /* d1 == d2 */
        default: checksum += 5000; break; /* Multiple true */
    }
    
    /* Final output to prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 255;  /* Return non-constant */
}
