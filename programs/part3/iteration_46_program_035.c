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
void __attribute__((noinline)) use_result(int val) {
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(val) : "memory");
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
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED - Triggered by explicit unordered checks */
    if (__builtin_isunordered(d1, d2)) checksum += 1;
    
    /* ORDERED - Inverse of unordered */
    if (!__builtin_isunordered(f1, f2)) checksum += 2;
    
    /* UNEQ - unordered or equal (with -ffast-math) */
    if (d1 == d2) checksum += 4;  /* May generate UNEQ with fast-math */
    
    /* UNGE - unordered or greater-or-equal */
    if (vd1 >= vd2) checksum += 8;
    
    /* UNGT - unordered or greater-than */
    if (vd1 > vd2) checksum += 16;
    
    /* UNLE - unordered or less-or-equal */
    if (f1 <= f2) checksum += 32;
    
    /* UNLT - unordered or less-than */
    if (f1 < f2) checksum += 64;
    
    /* LTGT - less or greater (ordered and not equal) */
    if (__builtin_islessgreater(d1, d2)) checksum += 128;
    
    /* ===== CONDITIONAL MOVES BASED ON FP COMPARISONS ===== */
    
    /* Use conditional operator to force conditional move generation */
    double cond_result = (d1 != d2) ? d1 : d2;  /* May use UNEQ or LTGT */
    checksum += (int)(cond_result * 10);
    
    float cond_float = (f1 >= f2) ? f1 : f2;  /* May use UNGE */
    checksum += (int)(cond_float * 20);
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons - these generate mask results */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_ne = (vec_a != vec_b);      /* May use LTGT */
    
    /* Extract results from vector comparisons */
    int mask_eq = __builtin_ia32_movmskps(cmp_eq);
    int mask_gt = __builtin_ia32_movmskps(cmp_gt);
    checksum += mask_eq + mask_gt;
    
    /* Double precision vector comparisons */
    v2df cmp_d_eq = (vec_da == vec_db);
    v2df cmp_d_lt = (vec_da < vec_db);
    int mask_d_eq = __builtin_ia32_movmskpd(cmp_d_eq);
    int mask_d_lt = __builtin_ia32_movmskpd(cmp_d_lt);
    checksum += mask_d_eq + mask_d_lt;
    
    /* Explicit unordered vector comparison using intrinsic */
    __m128 va = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 vb = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 cmp_unord = _mm_cmpunord_ps(va, vb);  /* Direct UNORDERED */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    checksum += mask_unord;
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    /* Create array with potential NaN values */
    volatile double arr[5] = {1.0, 2.0, 0.0/0.0, 4.0, 5.0};  /* 0/0.0 creates NaN */
    
    /* Loop with FP comparison in condition */
    int count = 0;
    for (int i = 0; i < 5 && (arr[i] != 0.0); ++i) {  /* May use UNEQ */
        count++;
    }
    checksum += count;
    
    /* ===== SWITCH BASED ON FP COMPARISON RESULTS ===== */
    
    /* Create multiple comparison results */
    int cmp_results = 0;
    cmp_results |= (d1 < d2) ? 0x1 : 0;    /* UNLT */
    cmp_results |= (d1 > d2) ? 0x2 : 0;    /* UNGT */
    cmp_results |= (d1 == d2) ? 0x4 : 0;   /* UNEQ */
    cmp_results |= (d1 != d2) ? 0x8 : 0;   /* LTGT or UNEQ */
    cmp_results |= (d1 <= d2) ? 0x10 : 0;  /* UNLE */
    cmp_results |= (d1 >= d2) ? 0x20 : 0;  /* UNGE */
    
    /* Use switch to create multiple branches */
    switch (cmp_results & 0x3F) {
        case 0x01: checksum += 1000; break;  /* Only less-than true */
        case 0x02: checksum += 2000; break;  /* Only greater-than true */
        case 0x04: checksum += 3000; break;  /* Only equal true */
        case 0x08: checksum += 4000; break;  /* Only not-equal true */
        default:   checksum += 5000; break;
    }
    
    /* ===== MIXED INTEGER/FLOAT COMPARISONS ===== */
    
    /* Compare floats with integers */
    int int_val = 2;
    if (f1 < int_val) checksum += 10000;    /* May use UNLT */
    if (vf1 > int_val) checksum += 20000;   /* May use UNGT */
    if (d1 == int_val) checksum += 30000;   /* May use UNEQ */
    
    /* ===== FINAL OUTPUT ===== */
    
    /* Create observable output */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-constant result */
}

/* Define external volatile variables */
volatile double external_double = 3.14159;
volatile float external_float = 2.71828f;
