#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

volatile int global_checksum = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int result = 0;
    
    /* Test UNORDERED (unord) */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Test ORDERED (ord) */
    if (!isunordered(a, b)) {
        result |= 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    if (isunordered(a, b) || a >= b) {
        result |= 8;
    }
    
    /* Test UNGT (nle) - unordered or greater than */
    if (isunordered(a, b) || a > b) {
        result |= 16;
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (isunordered(a, b) || a <= b) {
        result |= 32;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(a, b) || a < b) {
        result |= 64;
    }
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    if (!isunordered(a, b) && a != b) {
        result |= 128;
    }
    
    global_checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    volatile int result = 0;
    
    /* Test UNORDERED (unord) with vector comparisons */
    __m128d cmp_unord = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    __m128i mask_unord = _mm_castpd_si128(cmp_unord);
    if (_mm_movemask_epi8(mask_unord) != 0) {
        result |= 1;
    }
    
    /* Test ORDERED (ord) */
    __m128d cmp_ord = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    __m128i mask_ord = _mm_castpd_si128(cmp_ord);
    if (_mm_movemask_epi8(mask_ord) != 0) {
        result |= 2;
    }
    
    /* Test UNEQ (ueq) */
    __m128d cmp_ueq = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    __m128i mask_ueq = _mm_castpd_si128(cmp_ueq);
    if (_mm_movemask_epi8(mask_ueq) != 0) {
        result |= 4;
    }
    
    /* Test UNGE (nlt) */
    __m128d cmp_nlt = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    __m128i mask_nlt = _mm_castpd_si128(cmp_nlt);
    if (_mm_movemask_epi8(mask_nlt) != 0) {
        result |= 8;
    }
    
    /* Test UNGT (nle) */
    __m128d cmp_nle = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    __m128i mask_nle = _mm_castpd_si128(cmp_nle);
    if (_mm_movemask_epi8(mask_nle) != 0) {
        result |= 16;
    }
    
    /* Test UNLE (ule) */
    __m128d cmp_ule = _mm_cmp_pd(va, vb, _CMP_LE_UQ);
    __m128i mask_ule = _mm_castpd_si128(cmp_ule);
    if (_mm_movemask_epi8(mask_ule) != 0) {
        result |= 32;
    }
    
    /* Test UNLT (ult) */
    __m128d cmp_ult = _mm_cmp_pd(va, vb, _CMP_LT_UQ);
    __m128i mask_ult = _mm_castpd_si128(cmp_ult);
    if (_mm_movemask_epi8(mask_ult) != 0) {
        result |= 64;
    }
    
    /* Test LTGT (une) */
    __m128d cmp_une = _mm_cmp_pd(va, vb, _CMP_NEQ_OQ);
    __m128i mask_une = _mm_castpd_si128(cmp_une);
    if (_mm_movemask_epi8(mask_une) != 0) {
        result |= 128;
    }
    
    /* Test with float vectors as well */
    __m128 cmp_unord_f = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    __m128i mask_unord_f = _mm_castps_si128(cmp_unord_f);
    if (_mm_movemask_epi8(mask_unord_f) != 0) {
        result |= 256;
    }
    
    global_checksum += result;
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d va, __m128d vb) {
    volatile int result = 0;
    volatile double dresult = 0.0;
    volatile __m128d vresult;
    
    /* Inline assembly tests for each condition code */
    
    /* UNORDERED (unord) */
    __asm__ volatile (
        "cmppd %[unord_pred], %[b], %[a]\n\t"
        "movmskpd %%xmm0, %[res]\n\t"
        : [res] "=r" (result)
        : [a] "x" (va), [b] "x" (vb), [unord_pred] "i" (3)  /* _CMP_UNORD_Q */
        : "xmm0", "cc"
    );
    
    /* ORDERED (ord) - using template substitution */
    __asm__ volatile (
        "cmppd %{%[pred]|ord}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (7)  /* _CMP_ORD_Q */
        : "xmm0", "cc"
    );
    
    /* UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %{%[pred]|ueq}, %[b], %[a]\n\t"
        "movapd %%xmm0, %[vres]\n\t"
        : [vres] "=x" (vresult)
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (8)  /* _CMP_EQ_UQ */
        : "xmm0", "cc"
    );
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmppd %{%[pred]|nlt}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (13) /* _CMP_NLT_UQ */
        : "xmm0", "cc"
    );
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmppd %{%[pred]|nle}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (14) /* _CMP_NLE_UQ */
        : "xmm0", "cc"
    );
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmppd %{%[pred]|ule}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (18) /* _CMP_LE_UQ */
        : "xmm0", "cc"
    );
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmppd %{%[pred]|ult}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (17) /* _CMP_LT_UQ */
        : "xmm0", "cc"
    );
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmppd %{%[pred]|une}, %[b], %[a]\n\t"
        : 
        : [a] "x" (va), [b] "x" (vb), [pred] "i" (12) /* _CMP_NEQ_OQ */
        : "xmm0", "cc"
    );
    
    /* Scalar version with different syntax */
    __asm__ volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %%al\n\t"
        : "=a" (result)
        : [a] "x" (va), [b] "x" (vb)
        : "cc"
    );
    
    global_checksum += result;
}

__attribute__((optimize("O3"), target("sse2")))
void test_ternary_conditions(double a, double b, float fa, float fb) {
    volatile double dresult = 0.0;
    volatile float fresult = 0.0f;
    
    /* Use ternary operators with uncommon conditions to force condition code generation */
    
    /* UNORDERED */
    dresult = isunordered(a, b) ? a + 1.0 : b - 1.0;
    
    /* ORDERED */
    dresult = !isunordered(a, b) ? a * 2.0 : b / 2.0;
    
    /* UNEQ */
    dresult = (isunordered(a, b) || a == b) ? a + b : a - b;
    
    /* UNGE */
    dresult = (isunordered(a, b) || a >= b) ? a * a : b * b;
    
    /* UNGT */
    dresult = (isunordered(a, b) || a > b) ? sqrt(a) : sqrt(b);
    
    /* UNLE */
    dresult = (isunordered(a, b) || a <= b) ? sin(a) : cos(b);
    
    /* UNLT */
    dresult = (isunordered(a, b) || a < b) ? exp(a) : exp(b);
    
    /* LTGT */
    dresult = (!isunordered(a, b) && a != b) ? log(a) : log(b);
    
    /* Repeat with float */
    fresult = isunordered(fa, fb) ? fa + 1.0f : fb - 1.0f;
    fresult = !isunordered(fa, fb) ? fa * 2.0f : fb / 2.0f;
    
    global_checksum += (int)dresult + (int)fresult;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values including NaN, infinity, and normal numbers */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with various special values */
    double a = (rand() % 100) / 10.0;
    double b = (rand() % 100) / 10.0;
    double nan_val = 0.0 / 0.0;  /* Generate NaN */
    double inf_val = 1.0 / 0.0;  /* Generate infinity */
    
    float fa = (rand() % 100) / 10.0f;
    float fb = (rand() % 100) / 10.0f;
    float nan_f = 0.0f / 0.0f;
    float inf_f = 1.0f / 0.0f;
    
    /* Vector test data */
    __m128d va = _mm_set_pd(a, nan_val);  /* Mix NaN and normal */
    __m128d vb = _mm_set_pd(b, inf_val);  /* Mix infinity and normal */
    __m128 vfa = _mm_set_ps(fa, nan_f, fb, inf_f);
    __m128 vfb = _mm_set_ps(fb, inf_f, fa, nan_f);
    
    /* Call test functions multiple times with different data combinations */
    for (int i = 0; i < 10; i++) {
        /* Vary the inputs slightly each iteration */
        double a_mod = a + (i * 0.1);
        double b_mod = b - (i * 0.1);
        float fa_mod = fa + (i * 0.1f);
        float fb_mod = fb - (i * 0.1f);
        
        /* Alternate between normal and special values */
        if (i % 3 == 0) {
            a_mod = nan_val;
        } else if (i % 3 == 1) {
            b_mod = inf_val;
        }
        
        test_scalar_conditions(a_mod, b_mod, fa_mod, fb_mod);
        test_vector_conditions(va, vb, vfa, vfb);
        test_inline_asm_conditions(a_mod, b_mod, va, vb);
        test_ternary_conditions(a_mod, b_mod, fa_mod, fb_mod);
        
        /* Modify vectors slightly */
        va = _mm_add_pd(va, _mm_set1_pd(0.1));
        vb = _mm_sub_pd(vb, _mm_set1_pd(0.1));
    }
    
    printf("Final checksum: %d\n", global_checksum);
    return global_checksum != 0 ? 0 : 1;
}
