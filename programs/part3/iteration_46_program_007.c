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
    static double counter = 0.0;
    return counter++ * 1.234567;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 2.345678f;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink = 0;
    sink += val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize variables from mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    double d3 = 3.14159;
    double d4 = -2.71828;
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_input_float();
    float f2 = get_input_float();
    float f3 = 0.0f;
    float f4 = 1.0f / 0.0f;  /* Infinity */
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    
    /* UNORDERED - triggered by comparisons involving NaN/infinity */
    if (f4 != f4) {  /* NaN check */
        checksum += 1;
    }
    
    if (d1 == d1) {  /* Ordered check (inverse of unordered) */
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (f1 == f2) {
        checksum += 4;
    }
    
    /* UNGE - unordered or greater or equal (>= with fast-math) */
    if (vd1 >= d1) {
        checksum += 8;
    }
    
    /* UNGT - unordered or greater than (> with fast-math) */
    if (d2 > d3) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less or equal (<= with fast-math) */
    if (f1 <= vf2) {
        checksum += 32;
    }
    
    /* UNLT - unordered or less than (< with fast-math) */
    if (d4 < d1) {
        checksum += 64;
    }
    
    /* LTGT - less or greater (not equal, but ordered) */
    if (d1 != d2) {
        checksum += 128;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    
    /* Direct UNORDERED test */
    if (__builtin_isunordered(f1, f4)) {  /* f4 is infinity */
        checksum += 256;
    }
    
    /* ORDERED test */
    if (__builtin_isordered(d1, d2)) {
        checksum += 512;
    }
    
    /* LTGT via builtin */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 1024;
    }
    
    /* UNEQ via combination */
    if (!__builtin_islessgreater(vf1, vf2) && __builtin_isordered(vf1, vf2)) {
        checksum += 2048;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cmov_result2 = (f1 != f2) ? f1 : f2;   /* May use UNEQ or LTGT */
    checksum += (int)(cmov_result1 + cmov_result2);
    
    /* 5. Loop with FP condition */
    for (int i = 0; i < 10 && (d1 + i < d2); ++i) {
        checksum += i * 2;
    }
    
    /* 6. Vector (SIMD) comparisons */
    
    /* Initialize vector variables */
    v4sf vec_a = {f1, f2, f3, 4.0f};
    v4sf vec_b = {vf1, vf2, 0.0f, 4.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d3, d4};
    
    /* Vector comparisons - these may generate various condition codes */
    v4sf vec_cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf vec_cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf vec_cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf vec_cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf vec_cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf vec_cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results from vector comparisons */
    float* feq = (float*)&vec_cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (feq[i] != 0.0f) checksum += 1 << i;
    }
    
    /* Double vector comparisons */
    v2df vec_dcmp = (vec_da > vec_db);       /* May use UNGT */
    double* dgt = (double*)&vec_dcmp;
    if (dgt[0] != 0.0) checksum += 4096;
    if (dgt[1] != 0.0) checksum += 8192;
    
    /* 7. Use x86 intrinsics for explicit unordered comparison */
    __m128 mm_a = _mm_loadu_ps((float*)&vec_a);
    __m128 mm_b = _mm_loadu_ps((float*)&vec_b);
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);  /* Direct UNORDERED */
    
    /* Check unordered mask */
    int unord_mask = _mm_movemask_ps(mm_unord);
    checksum += unord_mask * 3;
    
    /* 8. Switch statement with FP comparisons */
    int switch_val = 0;
    if (d1 < 0.0) switch_val = 1;
    else if (d1 == 0.0) switch_val = 2;
    else if (d1 > 0.0) switch_val = 3;
    
    switch (switch_val) {
        case 1:
            if (f1 < 0.0f) checksum += 16384;  /* UNLT */
            break;
        case 2:
            if (f1 == 0.0f) checksum += 32768; /* UNEQ */
            break;
        case 3:
            if (f1 > 0.0f) checksum += 65536;  /* UNGT */
            break;
    }
    
    /* 9. Mixed integer/float comparisons */
    int int_val = checksum % 100;
    if ((float)int_val <= f1) {  /* Mixed types, may use UNLE */
        checksum += 131072;
    }
    
    if (d1 != (double)int_val) {  /* Mixed types, may use LTGT */
        checksum += 262144;
    }
    
    /* Final output to prevent dead code elimination */
    use_result(checksum);
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
