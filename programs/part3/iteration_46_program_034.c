/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding with opaque functions */
extern float get_float(void) __attribute__((noinline));
extern double get_double(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile float volatile_float = 1.5f;
volatile double volatile_double = 2.5;
float global_float = 3.0f;
double global_double = 4.0;

/* Opaque function implementations */
float get_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.1f;
}

double get_double(void) {
    static double counter = 0.0;
    return counter++ * 1.1;
}

void use_result(int val) {
    /* Create side effect to prevent optimization */
    static int accumulator = 0;
    accumulator += val;
    __asm__ volatile("" : : "r"(accumulator) : "memory");
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    float f1 = get_float();
    float f2 = get_float() + 0.5f;
    double d1 = get_double();
    double d2 = get_double() + 0.5;
    
    /* Mix with volatile to prevent constant folding */
    float vf = volatile_float;
    double vd = volatile_double;
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - should generate "unord" */
    if (__builtin_isunordered(f1, f2)) {
        checksum += 1;
    }
    
    /* ORDERED - should generate "ord" */
    if (!__builtin_isunordered(vf, global_float)) {
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (__builtin_isunordered(f1, f2) || (f1 == f2)) {
        checksum += 4;
    }
    /* Alternative UNEQ generation */
    if (!(f1 < f2) && !(f1 > f2)) {  /* Not less and not greater */
        checksum += 8;
    }
    
    /* UNGE - unordered or greater-or-equal (nlt) */
    if (!(f1 < f2)) {  /* Not less than */
        checksum += 16;
    }
    
    /* UNGT - unordered or greater (nle) */
    if (!(f1 <= f2)) {  /* Not less-or-equal */
        checksum += 32;
    }
    
    /* UNLE - unordered or less-or-equal */
    if (__builtin_islessequal(f1, f2)) {
        checksum += 64;
    }
    
    /* UNLT - unordered or less-than */
    if (__builtin_isless(f1, f2)) {
        checksum += 128;
    }
    
    /* LTGT - less or greater (une) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 256;
    }
    
    /* 2. Mixed-type comparisons */
    if (vf != (float)((int)vd)) {  /* Float vs int conversion */
        checksum += 512;
    }
    
    /* 3. Conditional moves based on FP comparisons */
    double result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float result2 = (f1 <= f2) ? f1 : f2;   /* May use UNLE */
    
    checksum += (int)(result1 + result2);
    
    /* 4. Loop with FP condition */
    for (int i = 0; i < 10 && (get_float() != 0.0f); i++) {
        checksum += i;
    }
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector equality - may use UNEQ */
    v4sf cmp_eq = (vec_a == vec_b);
    int mask_eq = _mm_movemask_ps((__m128)cmp_eq);
    checksum += mask_eq;
    
    /* Vector less-than - may use UNLT */
    v4sf cmp_lt = (vec_a < vec_b);
    int mask_lt = _mm_movemask_ps((__m128)cmp_lt);
    checksum += mask_lt;
    
    /* Vector greater-than - may use UNGT */
    v4sf cmp_gt = (vec_a > vec_b);
    int mask_gt = _mm_movemask_ps((__m128)cmp_gt);
    checksum += mask_gt;
    
    /* Explicit unordered vector comparison */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    int mask_unord = _mm_movemask_ps(cmp_unord);
    checksum += mask_unord;
    
    /* 6. Double vector comparisons */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    v2df cmp_dneq = (vec_da != vec_db);  /* Not equal */
    long long mask_dneq;
    __asm__ volatile("" : : "r"(cmp_dneq) : "memory");
    
    /* 7. Switch statement with FP comparisons */
    int switch_val = checksum & 7;
    switch (switch_val) {
        case 0:
            if (f1 == f2) checksum += 1024;
            break;
        case 1:
            if (f1 != f2) checksum += 2048;
            break;
        case 2:
            if (f1 < f2) checksum += 4096;
            break;
        case 3:
            if (f1 > f2) checksum += 8192;
            break;
        case 4:
            if (f1 <= f2) checksum += 16384;
            break;
        case 5:
            if (f1 >= f2) checksum += 32768;
            break;
        default:
            if (__builtin_isunordered(d1, d2)) checksum += 65536;
            break;
    }
    
    /* 8. Complex expression mixing conditions */
    double complex_expr = (d1 < d2) ? 
                         ((f1 > f2) ? d1 : d2) : 
                         ((__builtin_isunordered(f1, f2)) ? 0.0 : d1 + d2);
    checksum += (int)complex_expr;
    
    /* Ensure all code paths are potentially reachable */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 255;  /* Return non-constant to prevent optimization */
}
