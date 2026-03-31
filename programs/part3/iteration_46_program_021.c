/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double get_double(void) __attribute__((noinline));
extern volatile float get_float(void) __attribute__((noinline));
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

/* Simulate runtime values */
volatile double get_double(void) {
    static volatile double counter = 0.0;
    return counter++ * 1.5;
}

volatile float get_float(void) {
    static volatile float counter = 0.0f;
    return counter++ * 2.0f;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_double();
    double d2 = get_double();
    float f1 = get_float();
    float f2 = get_float();
    
    /* UNORDERED/ORDERED patterns */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (vd1 <= vd2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (f1 >= f2) {  /* May generate UNGE */
        checksum += 4;
    }
    
    if (vf1 < vf2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    if (vf1 > vf2) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* 2. Explicit built-in functions for condition codes */
    checksum += __builtin_isunordered(d1, d2) ? 32 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(f1, f2) ? 64 : 0;    /* LTGT */
    checksum += !__builtin_islessequal(d1, d2) ? 128 : 0;    /* UNLE (inverted) */
    checksum += !__builtin_isgreaterequal(f1, f2) ? 256 : 0; /* UNGE (inverted) */
    
    /* Mixed integer/float comparisons */
    int i = 5;
    if (d1 != i) {  /* Mixed type comparison */
        checksum += 512;
    }
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float res[4];
    __builtin_memcpy(res, &cmp_eq, sizeof(res));
    checksum += (int)(res[0] + res[1] + res[2] + res[3]);
    
    /* Intrinsic-based unordered check */
    __m128 a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 unord = _mm_cmpunord_ps(a, b);  /* Explicit UNORDERED */
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result1 = (d1 < d2) ? d1 : d2;      /* May use UNLT */
    float cmov_result2 = (f1 >= f2) ? f1 : f2;      /* May use UNGE */
    double cmov_result3 = (d1 != d2) ? d1 : d2;     /* May use UNEQ/LTGT */
    
    checksum += (int)(cmov_result1 + cmov_result2 + cmov_result3);
    
    /* Loop with FP condition */
    volatile double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_double();
    }
    
    int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {  /* UNEQ in loop condition */
        count++;
    }
    checksum += count;
    
    /* Switch with FP-derived values */
    int fp_switch = (d1 > 0.0) ? 1 : (d1 < 0.0) ? 2 : (d1 == 0.0) ? 3 : 4;
    switch (fp_switch) {
        case 1: checksum += 1000; break;  /* UNGT */
        case 2: checksum += 2000; break;  /* UNLT */
        case 3: checksum += 3000; break;  /* UNEQ */
        default: checksum += 4000; break; /* UNORDERED */
    }
    
    /* 5. Prevent optimization through external use */
    use_result(checksum);
    
    /* Final observable output */
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
