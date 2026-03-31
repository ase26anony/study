/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float __attribute__((noinline)) get_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.2f;
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
    
    /* 1. Initialize mixed floating-point variables */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_double();
    double d2 = get_double() + 1.0;
    float f1 = get_float();
    float f2 = get_float() + 1.0f;
    
    /* 2. Perform relational operator comparisons with -ffast-math */
    
    /* UNORDERED - should trigger with NaN-like behavior under fast-math */
    if (d1 != d1) {  /* NaN check that may generate UNORDERED */
        checksum |= 1;
    }
    
    /* UNEQ - unordered or equal */
    if (vd1 == vd2) {
        checksum |= 2;
    }
    
    /* UNGE - unordered or greater than or equal */
    if (d1 >= d2) {
        checksum |= 4;
    }
    
    /* UNGT - unordered or greater than */
    if (d1 > d2) {
        checksum |= 8;
    }
    
    /* UNLE - unordered or less than or equal */
    if (f1 <= f2) {
        checksum |= 16;
    }
    
    /* UNLT - unordered or less than */
    if (vf1 < vf2) {
        checksum |= 32;
    }
    
    /* LTGT - less than or greater than (unordered excluded) */
    if (d1 != d2) {
        checksum |= 64;
    }
    
    /* ORDERED - both operands are ordered (not NaN) */
    if (d1 == d1 && d2 == d2) {  /* Ordered check */
        checksum |= 128;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    checksum += __builtin_isunordered(d1, d2) ? 256 : 0;
    checksum += __builtin_islessgreater(f1, f2) ? 512 : 0;
    checksum += __builtin_islessequal(vf1, vf2) ? 1024 : 0;
    checksum += __builtin_isgreaterequal(vd1, vd2) ? 2048 : 0;
    
    /* 4. Conditional moves based on FP results */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cmov_result2 = (f1 != f2) ? f1 : f2;   /* May use UNEQ or LTGT */
    checksum += (int)cmov_result1;
    checksum += (int)cmov_result2;
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (get_float() != 0.0f); ++i) {
        checksum += i;
    }
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
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
    
    /* Use x86 intrinsics for explicit unordered comparison */
    __m128 mma = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 mmb = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 mm_unord = _mm_cmpunord_ps(mma, mmb);  /* Direct UNORDERED */
    
    /* Convert vector comparison to integer mask */
    int mask = _mm_movemask_ps(mm_unord);
    checksum += mask;
    
    /* Double precision vectors */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_dneq = (vec_da != vec_db);
    
    double* dneq_ptr = (double*)&cmp_dneq;
    checksum += (dneq_ptr[0] != 0.0) ? 4096 : 0;
    checksum += (dneq_ptr[1] != 0.0) ? 8192 : 0;
    
    /* 6. Switch statement with FP comparisons */
    int fp_case = 0;
    if (d1 < d2) fp_case = 1;
    else if (d1 > d2) fp_case = 2;
    else if (d1 == d2) fp_case = 3;
    else fp_case = 4;  /* unordered */
    
    switch (fp_case) {
        case 1: checksum += 16384; break;  /* UNLT */
        case 2: checksum += 32768; break;  /* UNGT */
        case 3: checksum += 65536; break;  /* UNEQ */
        case 4: checksum += 131072; break; /* UNORDERED */
    }
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
