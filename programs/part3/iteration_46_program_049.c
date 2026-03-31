/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_input;
extern float get_float(void) __attribute__((noinline));
extern double get_double(void) __attribute__((noinline));

/* Opaque function to consume results */
extern void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile float vf1 = 1.0f, vf2 = 2.0f;
volatile double vd1 = 1.0, vd2 = 2.0;

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    float f1 = get_float();
    float f2 = get_float() + 0.5f;
    double d1 = get_double();
    double d2 = get_double() * 1.1;
    
    /* UNORDERED/ORDERED checks */
    if (f1 != f2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (vf1 <= vf2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (d1 >= d2) {  /* May generate UNGE */
        checksum += 4;
    }
    
    if (vd1 < vd2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    if (d1 > d2) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* Mixed float/double comparisons */
    if ((double)f1 == d1) {  /* May generate UNEQ */
        checksum += 32;
    }
    
    /* 2. Explicit built-in unordered checks */
    checksum += __builtin_isunordered(f1, f2) ? 64 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(d1, d2) ? 128 : 0;   /* LTGT */
    
    /* More builtins covering different conditions */
    if (__builtin_islessequal(f1, f2)) {    /* UNLE */
        checksum += 256;
    }
    
    if (__builtin_isgreaterequal(d1, d2)) { /* UNGE */
        checksum += 512;
    }
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {f1, f2, 3.0f, 4.0f};
    v4sf vec_b = {f2, f1, 3.0f, 5.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector equality - may use UNEQ */
    v4sf cmp_eq = (vec_a == vec_b);
    int eq_mask = __builtin_ia32_movmskps((__v4sf)cmp_eq);
    checksum += eq_mask * 1024;
    
    /* Vector inequality - may use LTGT */
    v4sf cmp_neq = (vec_a != vec_b);
    int neq_mask = __builtin_ia32_movmskps((__v4sf)cmp_neq);
    checksum += neq_mask * 2048;
    
    /* Vector less-than - may use UNLT */
    v4sf cmp_lt = (vec_a < vec_b);
    int lt_mask = __builtin_ia32_movmskps((__v4sf)cmp_lt);
    checksum += lt_mask * 4096;
    
    /* Vector greater-than - may use UNGT */
    v4sf cmp_gt = (vec_a > vec_b);
    int gt_mask = __builtin_ia32_movmskps((__v4sf)cmp_gt);
    checksum += gt_mask * 8192;
    
    /* Explicit unordered vector intrinsic */
    __m128 a = _mm_set_ps(f1, f2, 0.0f, 1.0f);
    __m128 b = _mm_set_ps(f2, f1, 0.0f, 0.0f);
    __m128 unord = _mm_cmpunord_ps(a, b);  /* Direct UNORDERED check */
    int unord_mask = _mm_movemask_ps(unord);
    checksum += unord_mask * 16384;
    
    /* 4. Conditional moves based on FP results */
    double cond_result = (f1 >= f2) ? (double)f1 : (double)f2;  /* May use UNGE */
    checksum += (int)(cond_result * 100);
    
    float select_result = (d1 != d2) ? f1 : f2;  /* May use UNEQ or LTGT */
    checksum += (int)(select_result * 10);
    
    /* 5. Loop with FP condition */
    float arr[4] = {f1, f2, 0.0f, 1.0f};
    for (int i = 0; i < 4 && (arr[i] != 0.0f); ++i) {  /* May use UNEQ */
        checksum += i * 1000;
    }
    
    /* Switch based on comparison results */
    int cmp_case = 0;
    cmp_case |= (f1 < f2) ? 1 : 0;   /* UNLT */
    cmp_case |= (d1 > d2) ? 2 : 0;   /* UNGT */
    cmp_case |= __builtin_isunordered(f1, f2) ? 4 : 0;  /* UNORDERED */
    
    switch (cmp_case) {
        case 0: checksum += 100000; break;
        case 1: checksum += 200000; break;  /* UNLT path */
        case 2: checksum += 300000; break;  /* UNGT path */
        case 4: checksum += 400000; break;  /* UNORDERED path */
        default: checksum += 500000; break;
    }
    
    /* 6. Mixed integer/float comparisons */
    int int_val = checksum % 100;
    if ((float)int_val <= f1) {  /* May generate UNLE */
        checksum += 600000;
    }
    
    if ((double)int_val >= d1) {  /* May generate UNGE */
        checksum += 700000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum > 0 ? 0 : 1;
}

/* Dummy implementations to satisfy linker */
volatile double external_input = 3.14159;
float get_float(void) { return (float)external_input; }
double get_double(void) { return external_input; }
void use_result(int x) { volatile int dummy = x; (void)dummy; }
