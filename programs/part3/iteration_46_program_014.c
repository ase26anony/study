/* test_condcodes.c - Target x86 condition code mnemonics for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return (double)((uintptr_t)__builtin_return_address(0) & 0xFF) / 100.0;
}

float __attribute__((noinline)) get_float_input(void) {
    return (float)((uintptr_t)__builtin_return_address(0) & 0xFF) / 100.0f;
}

/* Dummy function to create side effects */
void __attribute__((noinline)) use_result(int res) {
    /* Prevent dead code elimination */
    volatile static int sink;
    sink = res;
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
    float f2 = get_float_input() + 0.3f;
    
    /* 2. Perform scalar FP comparisons with relational operators */
    /* Using -ffast-math, these may generate UNORDERED/ORDERED variants */
    
    /* UNORDERED/UNEQ cases */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    /* UNLE case */
    if (vd1 <= d2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    /* UNLT case */
    if (f1 < f2) {  /* May generate UNLT */
        checksum += 4;
    }
    
    /* UNGE case */
    if (vd2 >= d1) {  /* May generate UNGE */
        checksum += 8;
    }
    
    /* UNGT case */
    if (d2 > d1) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* LTGT case */
    if (!(f1 == f2)) {  /* With -ffast-math, may generate LTGT */
        checksum += 32;
    }
    
    /* 3. Conditional moves based on FP comparisons */
    double cmov_result = (d1 >= vd1) ? d1 : vd1;  /* May use UNGE */
    checksum += (int)(cmov_result * 10);
    
    float f_cmov = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)(f_cmov * 10);
    
    /* 4. Explicit builtin calls for specific condition codes */
    
    /* Direct UNORDERED test */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* LTGT test */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* ORDERED test */
    if (!__builtin_isunordered(d1, vd1)) {
        checksum += 256;
    }
    
    /* UNEQ test via builtin */
    if (!__builtin_islessgreater(d1, d2) && !__builtin_isunordered(d1, d2)) {
        checksum += 512;
    }
    
    /* 5. Vector (SIMD) comparisons */
    
    /* Initialize vector variables */
    v4sf vec_a = {f1, f2, f1 + 1.0f, f2 + 1.0f};
    v4sf vec_b = {f2, f1, f2 + 1.0f, f1 + 1.0f};
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b); /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);   /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);  /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);   /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);  /* May use UNGE */
    
    /* Extract results to scalar checksum */
    int vec_mask = __builtin_ia32_movmskps(cmp_eq);
    checksum += vec_mask;
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_loadu_ps((float*)&vec_a);
    __m128 sse_b = _mm_loadu_ps((float*)&vec_b);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    int unord_mask = _mm_movemask_ps(unord_cmp);
    checksum += unord_mask * 2;
    
    /* Double precision vectors */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_d_eq = (vec_da == vec_db);
    
    /* Convert vector comparison to integer */
    int64_t d_mask;
    __builtin_memcpy(&d_mask, &cmp_d_eq, sizeof(d_mask));
    checksum += (int)(d_mask & 0xFF);
    
    /* 6. Loop with FP condition - prevents optimization */
    volatile float loop_f = 0.0f;
    for (int i = 0; i < 10 && (loop_f != 1.0f); ++i) {  /* May use UNEQ */
        loop_f += 0.1f;
        if (loop_f > 0.5f) {  /* May use UNGT */
            checksum += i;
        }
    }
    
    /* 7. Switch based on FP comparison results */
    int cmp_results[6] = {0};
    cmp_results[0] = (d1 == d2) ? 1 : 0;      /* UNEQ */
    cmp_results[1] = (d1 != d2) ? 1 : 0;      /* LTGT */
    cmp_results[2] = (d1 < d2) ? 1 : 0;       /* UNLT */
    cmp_results[3] = (d1 <= d2) ? 1 : 0;      /* UNLE */
    cmp_results[4] = (d1 > d2) ? 1 : 0;       /* UNGT */
    cmp_results[5] = (d1 >= d2) ? 1 : 0;      /* UNGE */
    
    for (int i = 0; i < 6; i++) {
        checksum += cmp_results[i] * (i + 1);
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = (int)d1;
    if (int_val != d1) {  /* Mixed type comparison */
        checksum += 1024;
    }
    
    /* Create observable output */
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent elimination */
    use_result(checksum);
    
    return checksum & 0xFF;
}
