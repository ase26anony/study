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

/* Dummy function to prevent optimization */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Create side effect */
    volatile static int sink = 0;
    sink += val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile variables */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 3.0f;
    volatile float vf2 = 4.0f;
    
    /* Get dynamic values */
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* Mix integer and floating-point comparisons */
    int int_val = 5;
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED/ORDERED: Comparisons that might involve NaN with -ffast-math */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    /* UNGE: "nlt" (not less than) */
    if (vd1 >= vd2) {
        checksum += 2;
    }
    
    /* UNGT: "nle" (not less than or equal) */
    if (d1 > d2) {
        checksum += 3;
    }
    
    /* UNLE: "ule" (unordered or less than or equal) */
    if (f1 <= f2) {
        checksum += 4;
    }
    
    /* UNLT: "ult" (unordered or less than) */
    if (vf1 < vf2) {
        checksum += 5;
    }
    
    /* LTGT: "une" (unordered or not equal) */
    if (d1 != d2 && !(d1 < d2) && !(d1 > d2)) {
        /* Complex condition that might trigger LTGT */
        checksum += 6;
    }
    
    /* Mix with integer */
    if ((double)int_val > d1) {
        checksum += 7;
    }
    
    /* ===== BUILTIN FUNCTIONS FOR EXPLICIT CHECKS ===== */
    
    /* Direct unordered check - should generate UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 8;
    }
    
    /* Direct ordered check - should generate ORDERED */
    if (__builtin_isordered(f1, f2)) {
        checksum += 9;
    }
    
    /* LTGT builtin */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 10;
    }
    
    /* UNEQ builtin equivalent */
    if (!__builtin_islessgreater(d1, d2) && !__builtin_isunordered(d1, d2)) {
        checksum += 11;
    }
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons - these often use condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to scalar checksum */
    for (int i = 0; i < 4; i++) {
        if (cmp_eq[i]) checksum += 12 + i;
        if (cmp_neq[i]) checksum += 16 + i;
        if (cmp_lt[i]) checksum += 20 + i;
        if (cmp_le[i]) checksum += 24 + i;
        if (cmp_gt[i]) checksum += 28 + i;
        if (cmp_ge[i]) checksum += 32 + i;
    }
    
    /* Double vector comparisons */
    v2df cmp_d_eq = (vec_da == vec_db);
    v2df cmp_d_neq = (vec_da != vec_db);
    
    if (cmp_d_eq[0]) checksum += 36;
    if (cmp_d_eq[1]) checksum += 37;
    if (cmp_d_neq[0]) checksum += 38;
    if (cmp_d_neq[1]) checksum += 39;
    
    /* ===== INTRINSICS FOR EXPLICIT UNORDERED VECTOR COMPARISONS ===== */
    
    /* Use SSE/AVX intrinsics for direct unordered comparison */
    __m128 mm_a = _mm_loadu_ps((float*)&vec_a);
    __m128 mm_b = _mm_loadu_ps((float*)&vec_b);
    
    /* CMPUNORD_PS - unordered comparison */
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);
    
    /* CMPORD_PS - ordered comparison */
    __m128 mm_ord = _mm_cmpord_ps(mm_a, mm_b);
    
    /* Extract mask results */
    int mask_unord = _mm_movemask_ps(mm_unord);
    int mask_ord = _mm_movemask_ps(mm_ord);
    
    checksum += mask_unord + mask_ord;
    
    /* ===== CONDITIONAL MOVES BASED ON FP COMPARISONS ===== */
    
    /* Conditional move using ternary operator */
    double cond_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cond_result2 = (f1 <= f2) ? f1 : f2;   /* May use UNLE */
    double cond_result3 = (vd1 != vd2) ? vd1 : vd2; /* May use UNEQ or LTGT */
    
    /* Convert to integer for checksum */
    checksum += (int)cond_result1;
    checksum += (int)cond_result2;
    checksum += (int)cond_result3;
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    /* Create array with potential zeros */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_input_double() - 1.0;  /* Some might be zero */
    }
    
    /* Loop with FP comparison in condition */
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {
        checksum += i * 2;
    }
    
    /* ===== SWITCH BASED ON FP COMPARISON RESULTS ===== */
    
    /* Create multiple comparison sites */
    int cmp_results = 0;
    cmp_results |= (d1 < d2) ? 0x1 : 0;
    cmp_results |= (d1 > d2) ? 0x2 : 0;
    cmp_results |= (d1 == d2) ? 0x4 : 0;
    cmp_results |= (d1 != d2) ? 0x8 : 0;
    cmp_results |= (f1 <= f2) ? 0x10 : 0;
    cmp_results |= (f1 >= f2) ? 0x20 : 0;
    
    /* Switch on combined results */
    switch (cmp_results & 0x3F) {
        case 0x01: checksum += 100; break;  /* UNLT */
        case 0x02: checksum += 200; break;  /* UNGT */
        case 0x04: checksum += 300; break;  /* UNEQ */
        case 0x08: checksum += 400; break;  /* LTGT */
        case 0x10: checksum += 500; break;  /* UNLE */
        case 0x20: checksum += 600; break;  /* UNGE */
        default:   checksum += 700; break;
    }
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    /* Print result to ensure all code is live */
    printf("Condition code test checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
