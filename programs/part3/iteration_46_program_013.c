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

/* Functions to get dynamic values */
volatile double get_input_double(void) {
    static volatile double counter = 0.0;
    return counter++ * 1.234567;
}

volatile float get_input_float(void) {
    static volatile float counter = 0.0f;
    return counter++ * 2.345678f;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
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
    float f4 = 1.0f;
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    /* Using -ffast-math, these may generate UNORDERED/ORDERED condition codes */
    
    /* UNORDERED case - potentially with NaN inputs */
    if (d1 != d1) {  /* NaN check */
        checksum += 1;
    }
    
    /* UNEQ case - unordered or equal */
    if (d1 == d2) {
        checksum += 2;
    }
    
    /* UNGE case - unordered or greater than or equal */
    if (d1 >= d3) {
        checksum += 4;
    }
    
    /* UNGT case - unordered or greater than */
    if (d1 > d4) {
        checksum += 8;
    }
    
    /* UNLE case - unordered or less than or equal */
    if (vd1 <= vd2) {
        checksum += 16;
    }
    
    /* UNLT case - unordered or less than */
    if (vd1 < vd2) {
        checksum += 32;
    }
    
    /* LTGT case - less than or greater than (ordered and not equal) */
    if (d1 != d3) {
        checksum += 64;
    }
    
    /* Mix float and double comparisons */
    if ((double)f1 > d2) {
        checksum += 128;
    }
    
    if (f1 <= (float)d3) {
        checksum += 256;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    /* These directly map to specific condition codes */
    
    /* __builtin_isunordered - UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 512;
    }
    
    /* __builtin_islessgreater - LTGT */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 1024;
    }
    
    /* Other builtins that may generate condition codes */
    if (__builtin_islessequal(f1, f3)) {  /* May use UNLE */
        checksum += 2048;
    }
    
    if (__builtin_isgreaterequal(d1, d4)) {  /* May use UNGE */
        checksum += 4096;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    /* Force materialization of condition codes */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    double cmov_result2 = (f1 != f2) ? f1 : f2;  /* May use UNEQ or LTGT */
    checksum += (int)(cmov_result1 + cmov_result2);
    
    /* 5. Vector (SIMD) comparisons */
    /* Using GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_unord;                    /* For UNORDERED */
    
    /* Extract results to scalar checksum */
    for (int i = 0; i < 4; i++) {
        if (cmp_eq[i]) checksum += 8192;
        if (cmp_gt[i]) checksum += 16384;
        if (cmp_le[i]) checksum += 32768;
    }
    
    /* Using x86 intrinsics for explicit unordered comparison */
    __m128 va = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vb = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q - unordered comparison */
    __m128 cmp_unord_intrin = _mm_cmpunord_ps(va, vb);
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(cmp_unord_intrin);
    if (mask != 0) {
        checksum += 65536;
    }
    
    /* Double precision vector */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d3, d4};
    v2df cmp_dneq = (vec_da != vec_db);  /* May use UNEQ or LTGT */
    
    if (cmp_dneq[0] || cmp_dneq[1]) {
        checksum += 131072;
    }
    
    /* 6. Loop with FP condition to generate multiple condition code uses */
    float arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_input_float();
    }
    
    for (int i = 0; i < 10; i++) {
        /* Mix comparisons in loop */
        if (i < 5 && arr[i] != 0.0f) {  /* May use UNEQ */
            checksum += i * 2;
        }
        if (i >= 5 && arr[i] >= 1.0f) {  /* May use UNGE */
            checksum += i * 3;
        }
    }
    
    /* 7. Switch based on FP comparison results */
    /* Create multiple condition code emission sites */
    int fp_switch = 0;
    if (d1 < d2) fp_switch = 1;      /* May use UNLT */
    if (d1 == d3) fp_switch = 2;     /* May use UNEQ */
    if (__builtin_isunordered(d1, d4)) fp_switch = 3;  /* UNORDERED */
    
    switch (fp_switch) {
        case 1: checksum += 262144; break;
        case 2: checksum += 524288; break;
        case 3: checksum += 1048576; break;
        default: checksum += 2097152; break;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Condition code checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;  /* Return non-constant result */
}
