/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float __attribute__((noinline)) get_float_input(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    /* Create side effect */
    static int accumulator = 0;
    accumulator += val;
    external_double = accumulator;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_double_input();
    double d2 = get_double_input();
    float f1 = get_float_input();
    float f2 = get_float_input();
    
    /* Mix volatile and dynamic values */
    /* UNORDERED/ORDERED conditions */
    if (vd1 != vd2) checksum += 1;  /* May generate UNEQ or LTGT */
    if (d1 == d2) checksum += 2;    /* May generate UNEQ with -ffast-math */
    
    /* UNGE condition (nlt) */
    if (vd1 >= vd2) checksum += 4;  /* May generate UNGE -> "nlt" */
    if (d1 >= d2) checksum += 8;
    
    /* UNGT condition (nle) */
    if (vd1 > vd2) checksum += 16;  /* May generate UNGT -> "nle" */
    if (f1 > f2) checksum += 32;
    
    /* UNLE condition (ule) */
    if (vf1 <= vf2) checksum += 64; /* May generate UNLE -> "ule" */
    if (d1 <= d2) checksum += 128;
    
    /* UNLT condition (ult) */
    if (vf1 < vf2) checksum += 256; /* May generate UNLT -> "ult" */
    if (d1 < d2) checksum += 512;
    
    /* LTGT condition (une) */
    if (vd1 != vd2) checksum += 1024; /* May generate LTGT -> "une" */
    if (f1 != f2) checksum += 2048;
    
    /* 2. Explicit built-in unordered checks */
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d2)) checksum += 4096;
    if (__builtin_isunordered(f1, f2)) checksum += 8192;
    
    /* ORDERED condition */
    if (__builtin_islessequal(d1, d2)) checksum += 16384;  /* Uses ordered comparison */
    if (__builtin_islessgreater(f1, f2)) checksum += 32768; /* LTGT condition */
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp_vec_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_vec_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_vec_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_vec_ge = (vec_a >= vec_b);      /* May use UNGE */
    v4sf cmp_vec_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_vec_neq = (vec_a != vec_b);     /* May use LTGT */
    
    /* Extract results to scalar checksum */
    float* cmp_ptr = (float*)&cmp_vec_eq;
    for (int i = 0; i < 4; i++) {
        if (cmp_ptr[i] != 0.0f) checksum += 65536;
    }
    
    /* Intrinsic-based unordered check */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);  /* Explicit UNORDERED */
    
    /* 4. Conditional moves based on FP comparisons */
    double cond_mov_result;
    for (int i = 0; i < 4; i++) {
        /* Conditional move with UNGE condition */
        cond_mov_result = (d1 >= d2) ? d1 : d2;
        checksum += (int)cond_mov_result;
        
        /* Conditional move with UNLE condition */
        cond_mov_result = (f1 <= f2) ? f1 : f2;
        checksum += (int)cond_mov_result;
        
        /* Mix with integer to create varied patterns */
        d1 += 0.5;
        f1 += 0.25f;
    }
    
    /* 5. Loop with FP condition */
    double arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = get_double_input();
    }
    
    for (int i = 0; i < 8 && (arr[i] != 0.0); i++) {
        /* UNEQ or LTGT condition in loop */
        checksum += i * 1000;
    }
    
    /* Switch based on FP comparison results */
    int fp_case = 0;
    if (d1 < d2) fp_case = 1;      /* UNLT */
    else if (d1 > d2) fp_case = 2; /* UNGT */
    else if (d1 == d2) fp_case = 3; /* UNEQ */
    else if (d1 != d2) fp_case = 4; /* LTGT */
    
    switch (fp_case) {
        case 1: checksum += 1000000; break;
        case 2: checksum += 2000000; break;
        case 3: checksum += 3000000; break;
        case 4: checksum += 4000000; break;
    }
    
    /* 6. Prevent dead code elimination */
    use_result(checksum);
    
    /* Final observable output */
    printf("Condition code checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-constant */
}
