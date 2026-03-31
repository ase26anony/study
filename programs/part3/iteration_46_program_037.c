/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return external_double;
}

float __attribute__((noinline)) get_float_input(void) {
    return external_float;
}

/* Dummy function to create side effects */
void __attribute__((noinline)) use_result(int res) {
    /* Prevent optimization */
    asm volatile("" : : "r"(res));
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    /* Get dynamic values */
    double d1 = get_double_input();
    double d2 = d1 + 1.0;
    float f1 = get_float_input();
    float f2 = f1 + 1.0f;
    
    /* ===== SCALAR COMPARISONS WITH FAST-MATH ===== */
    
    /* 1. UNORDERED - May generate "unord" */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* 2. ORDERED - May generate "ord" */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;
    }
    
    /* 3. UNEQ (Unordered or Equal) - May generate "ueq" */
    /* With -ffast-math, != comparison can use UNEQ */
    if (vd1 != vd2) {
        checksum += 4;
    }
    
    /* 4. UNGE (Unordered or Greater or Equal) - May generate "nlt" */
    /* >= with -ffast-math */
    if (d1 >= d2) {
        checksum += 8;
    }
    
    /* 5. UNGT (Unordered or Greater) - May generate "nle" */
    /* > with -ffast-math */
    if (f1 > f2) {
        checksum += 16;
    }
    
    /* 6. UNLE (Unordered or Less or Equal) - May generate "ule" */
    /* <= with -ffast-math */
    if (vd1 <= vd2) {
        checksum += 32;
    }
    
    /* 7. UNLT (Unordered or Less) - May generate "ult" */
    /* < with -ffast-math */
    if (d1 < d2) {
        checksum += 64;
    }
    
    /* 8. LTGT (Less or Greater) - May generate "une" */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* Mixed integer/float comparisons */
    int i = 5;
    if (vd1 < i) {
        checksum += 256;
    }
    
    if (i >= f1) {
        checksum += 512;
    }
    
    /* ===== CONDITIONAL MOVES ===== */
    
    /* Conditional move based on FP comparison */
    double cmov_result = (d1 == d2) ? d1 : d2;  /* May use UNEQ */
    checksum += (int)cmov_result;
    
    float f_cmov = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)f_cmov;
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons - these often generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    v4sf cmp_ne = (vec_a != vec_b);    /* May use UNEQ or LTGT */
    
    /* Extract results to scalar checksum */
    for (int i = 0; i < 4; i++) {
        checksum += cmp_eq[i] ? 1 : 0;
        checksum += cmp_lt[i] ? 2 : 0;
        checksum += cmp_gt[i] ? 4 : 0;
        checksum += cmp_le[i] ? 8 : 0;
        checksum += cmp_ge[i] ? 16 : 0;
        checksum += cmp_ne[i] ? 32 : 0;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Convert vector mask to integer */
    int unord_mask = _mm_movemask_ps(unord_cmp);
    checksum += unord_mask;
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    /* Loop with floating-point condition */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 0.5;
    }
    
    volatile int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {  /* May use UNEQ */
        count++;
    }
    checksum += count;
    
    /* ===== SWITCH BASED ON FP COMPARISONS ===== */
    
    /* Create multiple comparison sites */
    int case_selector = 0;
    
    /* Series of comparisons in switch-like pattern */
    if (d1 < d2) case_selector |= 1;   /* UNLT */
    if (d1 > d2) case_selector |= 2;   /* UNGT */
    if (d1 == d2) case_selector |= 4;  /* UNEQ */
    if (d1 != d2) case_selector |= 8;  /* UNEQ or LTGT */
    if (d1 <= d2) case_selector |= 16; /* UNLE */
    if (d1 >= d2) case_selector |= 32; /* UNGE */
    
    switch (case_selector & 0x3F) {
        case 0:
            checksum += 1000;
            break;
        case 1:
            checksum += 2000;  /* UNLT path */
            break;
        case 2:
            checksum += 3000;  /* UNGT path */
            break;
        case 4:
            checksum += 4000;  /* UNEQ path */
            break;
        default:
            checksum += 5000;
            break;
    }
    
    /* ===== FINAL OUTPUT ===== */
    
    /* Use result to prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
