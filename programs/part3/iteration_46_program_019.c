/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double __attribute__((weak));
extern volatile float external_float __attribute__((weak));

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.23456789;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.23456789f;
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
    float f4 = 1.0f / 0.0f;  /* Infinity */
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    /* Using -ffast-math, these may generate various condition codes */
    
    /* UNORDERED / UNEQ patterns */
    if (d1 != d2) checksum += 1;          /* May generate UNEQ or LTGT */
    if (!(f1 == f2)) checksum += 2;       /* Negated equality */
    
    /* UNLE / UNLT patterns */
    if (vd1 <= vd2) checksum += 4;        /* May generate UNLE */
    if (vf1 < vf2) checksum += 8;         /* May generate UNLT */
    
    /* UNGE / UNGT patterns */
    if (d3 >= d4) checksum += 16;         /* May generate UNGE */
    if (d3 > d4) checksum += 32;          /* May generate UNGT */
    
    /* Mixed integer/float comparisons */
    int i = 5;
    if (d1 < i) checksum += 64;
    if (i >= f1) checksum += 128;
    
    /* 3. Explicit built-in function calls for specific condition codes */
    checksum += __builtin_isunordered(d1, d2) ? 256 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(f1, f2) ? 512 : 0;    /* LTGT */
    checksum += __builtin_islessequal(vd1, vd2) ? 1024 : 0;   /* UNLE */
    checksum += __builtin_isless(vf1, vf2) ? 2048 : 0;        /* UNLT */
    checksum += __builtin_isgreaterequal(d3, d4) ? 4096 : 0;  /* UNGE */
    checksum += __builtin_isgreater(d3, d4) ? 8192 : 0;       /* UNGT */
    
    /* ORDERED check */
    checksum += __builtin_isordered(d1, d2) ? 16384 : 0;
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cmov_result2 = (f1 <= f2) ? f1 : f2;   /* May use UNLE */
    checksum += (int)cmov_result1;
    checksum += (int)cmov_result2;
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 1.0f, 1.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_c);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results from vector comparisons */
    int vec_checksum = 0;
    for (int i = 0; i < 4; i++) {
        vec_checksum += cmp_eq[i] ? 1 : 0;
        vec_checksum += cmp_neq[i] ? 2 : 0;
        vec_checksum += cmp_lt[i] ? 4 : 0;
        vec_checksum += cmp_le[i] ? 8 : 0;
        vec_checksum += cmp_gt[i] ? 16 : 0;
        vec_checksum += cmp_ge[i] ? 32 : 0;
    }
    checksum += vec_checksum;
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* Convert vector mask to integer */
    int unord_mask = _mm_movemask_ps(sse_cmp_unord);
    checksum += unord_mask;
    
    /* Double precision vector */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d3, d4};
    v2df vec_dcmp = (vec_da > vec_db);  /* May use UNGT */
    
    checksum += (int)vec_dcmp[0];
    checksum += (int)vec_dcmp[1];
    
    /* 6. Loop with FP condition */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_input_double();
    }
    
    /* Loop condition with FP comparison */
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {  /* May use UNEQ */
        checksum += i;
    }
    
    /* Switch based on FP comparison results */
    int case_selector = 0;
    case_selector += (d1 < d2) ? 1 : 0;
    case_selector += (d1 > d2) ? 2 : 0;
    case_selector += (d1 == d2) ? 4 : 0;
    case_selector += __builtin_isunordered(d1, d2) ? 8 : 0;
    
    switch (case_selector & 0x7) {
        case 0: checksum += 1000; break;
        case 1: checksum += 2000; break;  /* UNLT */
        case 2: checksum += 3000; break;  /* UNGT */
        case 3: checksum += 4000; break;
        case 4: checksum += 5000; break;  /* UNEQ */
        default: checksum += 6000; break;
    }
    
    /* 7. Create side effects to prevent optimization */
    use_result(checksum);
    
    /* Final output to prevent dead code elimination */
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
