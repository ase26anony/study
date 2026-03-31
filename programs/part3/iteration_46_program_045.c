/* test_condcodes.c - Target x86 condition code mnemonics for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double get_input_double(void) __attribute__((noinline));
extern volatile float get_input_float(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Opaque function to prevent optimization */
void use_result(int val) {
    /* Create side effect */
    static volatile int sink;
    sink = val;
}

/* Functions to get dynamic inputs */
volatile double get_input_double(void) {
    static volatile double counter = 0.0;
    return counter += 1.0;
}

volatile float get_input_float(void) {
    static volatile float counter = 0.0f;
    return counter += 1.0f;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize mixed floating-point variables */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* Mix with integer operands */
    int i1 = 3;
    int i2 = 4;
    
    /* 2. Perform relational operator comparisons with -ffast-math assumptions */
    
    /* UNORDERED - should generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* ORDERED - should generate "ord" */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal, from != with fast-math */
    if (d1 != d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* UNGE - unordered or greater-or-equal, from >= */
    if (vd1 >= vd2) {
        checksum += 8;
    }
    
    /* UNGT - unordered or greater-than, from > */
    if (f1 > f2) {
        checksum += 16;
    }
    
    /* UNLE - unordered or less-or-equal, from <= */
    if (d1 <= (double)i1) {  /* Mixed type */
        checksum += 32;
    }
    
    /* UNLT - unordered or less-than, from < */
    if (vf1 < vf2) {
        checksum += 64;
    }
    
    /* LTGT - less or greater (ordered and not equal), from != without fast-math? 
       or explicitly with __builtin_islessgreater */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    checksum += __builtin_isunordered(f1, f2) ? 256 : 0;
    checksum += __builtin_islessgreater(vd1, vd2) ? 512 : 0;
    
    /* 4. Conditional moves based on FP results */
    double cmov_result = (d1 == d2) ? d1 : d2;  /* May use UNEQ */
    checksum += (int)cmov_result;
    
    float f_cmov = (f1 >= f2) ? f1 : f2;  /* May use UNGE */
    checksum += (int)f_cmov;
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    
    /* Extract results to affect checksum */
    for (int i = 0; i < 4; i++) {
        checksum += cmp_eq[i] ? 1024 : 0;
        checksum += cmp_gt[i] ? 2048 : 0;
        checksum += cmp_le[i] ? 4096 : 0;
        checksum += cmp_lt[i] ? 8192 : 0;
    }
    
    /* Use x86 intrinsics for explicit unordered vector comparison */
    __m128 mm_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 mm_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);  /* Direct UNORDERED */
    
    /* Convert vector result to affect checksum */
    float unord_result[4];
    _mm_store_ps(unord_result, mm_unord);
    for (int i = 0; i < 4; i++) {
        checksum += (unord_result[i] != 0.0f) ? 16384 : 0;
    }
    
    /* 6. Loop with FP condition to create multiple emission sites */
    volatile double arr[4] = {1.0, 2.0, 3.0, 4.0};
    for (int i = 0; i < 4 && (arr[i] != 0.0); ++i) {  /* May use UNEQ */
        checksum += i * 32768;
    }
    
    /* Switch based on FP comparisons */
    int fp_switch = 0;
    if (d1 < d2) fp_switch = 1;      /* May use UNLT */
    else if (d1 > d2) fp_switch = 2; /* May use UNGT */
    else if (d1 == d2) fp_switch = 3; /* May use UNEQ */
    
    switch (fp_switch) {
        case 1: checksum += 65536; break;
        case 2: checksum += 131072; break;
        case 3: checksum += 262144; break;
    }
    
    /* 7. Double vector comparisons */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    v2df dvec_cmp = (dvec_a != dvec_b);  /* May use UNEQ or LTGT */
    
    checksum += (int)(dvec_cmp[0] + dvec_cmp[1]) * 524288;
    
    /* Final output to prevent dead code elimination */
    printf("Condition codes checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;
}
