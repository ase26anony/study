/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return (double)((uintptr_t)__builtin_return_address(0) & 0xFF) / 256.0;
}

float __attribute__((noinline)) get_float_input(void) {
    return (float)((uintptr_t)__builtin_return_address(0) & 0xFF) / 256.0;
}

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int cond) {
    /* Create side effect */
    static volatile int sink;
    sink = cond;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize FP variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = get_double_input() + 0.5;
    float f1 = get_float_input();
    float f2 = get_float_input() + 0.25f;
    
    /* 2. Standard relational operators with -ffast-math assumptions */
    
    /* UNORDERED/ORDERED: Comparisons that might involve NaN with fast-math */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum ^= 1;
    }
    
    if (!(d1 < d2)) {  /* May generate UNGE (nlt) */
        checksum ^= 2;
    }
    
    if (!(d1 <= d2)) {  /* May generate UNGT (nle) */
        checksum ^= 4;
    }
    
    if (f1 <= f2) {  /* May generate UNLE */
        checksum ^= 8;
    }
    
    if (f1 < f2) {  /* May generate UNLT */
        checksum ^= 16;
    }
    
    /* LTGT: Not equal and ordered */
    if (vd1 != vd2) {  /* With volatile, may generate LTGT (une) */
        checksum ^= 32;
    }
    
    /* 3. Explicit built-in functions for unordered checks */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(d1, d2)) {
        checksum ^= 64;
    }
    
    /* ORDERED: !__builtin_isunordered */
    if (!__builtin_isunordered(f1, f2)) {
        checksum ^= 128;
    }
    
    /* UNEQ: unordered or equal */
    /* Simulate with builtins */
    int is_unord = __builtin_isunordered(d1, d2);
    int is_eq = (d1 == d2);
    if (is_unord || is_eq) {
        checksum ^= 256;
    }
    
    /* LTGT: less or greater (ordered, not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum ^= 512;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result;
    
    /* UNGE: Generate with !(a < b) in conditional move */
    cmov_result = (!(d1 < d2)) ? d1 : d2;
    use_result((int)cmov_result);
    
    /* UNLE: Generate with (a <= b) in conditional move */
    float f_cmov = (f1 <= f2) ? f1 : f2;
    use_result((int)f_cmov);
    
    /* 5. Vector (SIMD) comparisons */
    
    /* Initialize vector variables */
    v4sf vec_a = {f1, f2, f1 + 0.1f, f2 + 0.1f};
    v4sf vec_b = {f2, f1, f2 + 0.2f, f1 + 0.2f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons - these often generate condition codes */
    v4sf cmp_vec_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_vec_lt = (vec_a < vec_b);   /* May use UNLT */
    v4sf cmp_vec_le = (vec_a <= vec_b);  /* May use UNLE */
    v4sf cmp_vec_gt = (vec_a > vec_b);   /* May use UNGT */
    v4sf cmp_vec_ge = (vec_a >= vec_b);  /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float cmp_results[4];
    cmp_results[0] = cmp_vec_eq[0] + cmp_vec_lt[0];
    cmp_results[1] = cmp_vec_le[1] + cmp_vec_gt[1];
    cmp_results[2] = cmp_vec_ge[2];
    use_result((int)cmp_results[0]);
    
    /* Double precision vector comparisons */
    v2df cmp_dbl_ne = (vec_da != vec_db);  /* May generate LTGT */
    use_result((int)cmp_dbl_ne[0]);
    
    /* 6. SSE intrinsics for explicit unordered comparisons */
    __m128 sse_a = _mm_loadu_ps((float*)&vec_a);
    __m128 sse_b = _mm_loadu_ps((float*)&vec_b);
    
    /* _CMP_UNORD_Q - unordered comparison */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* _CMP_NEQ_UQ - not equal or unordered */
    __m128 cmp_neq_uq = _mm_cmpneq_ps(sse_a, sse_b);
    
    /* Extract mask results */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_neq = _mm_movemask_ps(cmp_neq_uq);
    checksum ^= (mask_unord | mask_neq);
    
    /* 7. Loop with FP condition to generate multiple instances */
    double loop_var = d1;
    for (int i = 0; i < 10 && (loop_var != 0.0); ++i) {
        /* UNEQ/LTGT in loop condition */
        loop_var *= 0.5;
        checksum += i;
    }
    
    /* 8. Switch based on comparison results */
    int cmp_case = 0;
    if (d1 < d2) cmp_case = 1;
    else if (d1 > d2) cmp_case = 2;
    else if (d1 == d2) cmp_case = 3;
    else cmp_case = 4;  /* unordered case */
    
    switch (cmp_case) {
        case 1: checksum |= 0x1000; break;  /* UNLT */
        case 2: checksum |= 0x2000; break;  /* UNGT */
        case 3: checksum |= 0x3000; break;  /* UNEQ */
        case 4: checksum |= 0x4000; break;  /* UNORDERED */
    }
    
    /* 9. Mixed integer/float comparisons */
    int int_val = (int)d1;
    if ((float)int_val <= f1) {  /* Mixed types may generate UNLE */
        checksum ^= 0x8000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Condition code test checksum: %d\n", checksum);
    return checksum & 0xFF;
}
