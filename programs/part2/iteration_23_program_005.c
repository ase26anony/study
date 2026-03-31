#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

volatile int global_counter = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb, int* checksum) {
    volatile double da = a;
    volatile double db = b;
    volatile float ffa = fa;
    volatile float ffb = fb;
    
    /* Test UNORDERED (unord) */
    if (isunordered(da, db)) {
        *checksum += 1;
    }
    
    /* Test ORDERED (ord) */
    if (!isunordered(da, db)) {
        *checksum += 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(da, db) || da == db) {
        *checksum += 4;
    }
    
    /* Test UNGE (nlt) - not less than (unordered or greater or equal) */
    if (!(da < db) || isunordered(da, db)) {
        *checksum += 8;
    }
    
    /* Test UNGT (nle) - not less or equal (unordered or greater) */
    if (!(da <= db) || isunordered(da, db)) {
        *checksum += 16;
    }
    
    /* Test UNLE (ule) - unordered or less or equal */
    if (isunordered(da, db) || da <= db) {
        *checksum += 32;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(da, db) || da < db) {
        *checksum += 64;
    }
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    if ((da < db || da > db) && !isunordered(da, db)) {
        *checksum += 128;
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d a, __m128d b, __m128 fa, __m128 fb, int* checksum) {
    volatile __m128d va = a;
    volatile __m128d vb = b;
    volatile __m128 vfa = fa;
    volatile __m128 vfb = fb;
    
    /* Test UNORDERED (unord) with vector comparisons */
    __m128d cmp_unord = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    __m128i mask_unord = _mm_castpd_si128(cmp_unord);
    if (_mm_movemask_epi8(mask_unord) != 0) {
        *checksum += 256;
    }
    
    /* Test ORDERED (ord) */
    __m128d cmp_ord = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    __m128i mask_ord = _mm_castpd_si128(cmp_ord);
    if (_mm_movemask_epi8(mask_ord) != 0) {
        *checksum += 512;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    __m128d cmp_ueq = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    __m128i mask_ueq = _mm_castpd_si128(cmp_ueq);
    if (_mm_movemask_epi8(mask_ueq) != 0) {
        *checksum += 1024;
    }
    
    /* Test UNGE (nlt) - not less than (unordered or greater or equal) */
    __m128d cmp_nlt = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    __m128i mask_nlt = _mm_castpd_si128(cmp_nlt);
    if (_mm_movemask_epi8(mask_nlt) != 0) {
        *checksum += 2048;
    }
    
    /* Test UNGT (nle) - not less or equal (unordered or greater) */
    __m128d cmp_nle = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    __m128i mask_nle = _mm_castpd_si128(cmp_nle);
    if (_mm_movemask_epi8(mask_nle) != 0) {
        *checksum += 4096;
    }
    
    /* Test UNLE (ule) - unordered or less or equal */
    __m128d cmp_ule = _mm_cmp_pd(va, vb, _CMP_LE_UQ);
    __m128i mask_ule = _mm_castpd_si128(cmp_ule);
    if (_mm_movemask_epi8(mask_ule) != 0) {
        *checksum += 8192;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    __m128d cmp_ult = _mm_cmp_pd(va, vb, _CMP_LT_UQ);
    __m128i mask_ult = _mm_castpd_si128(cmp_ult);
    if (_mm_movemask_epi8(mask_ult) != 0) {
        *checksum += 16384;
    }
    
    /* Test LTGT (une) - not equal and ordered */
    __m128d cmp_une = _mm_cmp_pd(va, vb, _CMP_NEQ_OQ);
    __m128i mask_une = _mm_castpd_si128(cmp_une);
    if (_mm_movemask_epi8(mask_une) != 0) {
        *checksum += 32768;
    }
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, int* checksum) {
    volatile double da = a;
    volatile double db = b;
    double result;
    
    /* Inline assembly tests for each condition code */
    
    /* UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(da), "x"(db)
        : "al", "cc"
    );
    *checksum += (int)result * 65536;
    
    /* ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(da), "x"(db)
        : "al", "cc"
    );
    *checksum += (int)result * 131072;
    
    /* UNEQ (ueq) - Using cmppd with condition code */
    __m128d va = _mm_set_pd(da, da);
    __m128d vb = _mm_set_pd(db, db);
    __m128d cmp_result;
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 262144;
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 524288;
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 1048576;
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 2097152;
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 4194304;
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movapd %1, %0"
        : "=x"(cmp_result)
        : "x"(va), "x"(vb), "i"(0)
        : "cc"
    );
    *checksum += _mm_movemask_pd(cmp_result) * 8388608;
}

__attribute__((optimize("O3"), target("avx")))
void test_mixed_conditions_avx(double a, double b, float fa, float fb, int* checksum) {
    volatile double da = a;
    volatile double db = b;
    volatile float ffa = fa;
    volatile float ffb = fb;
    
    /* Complex nested conditions to prevent optimization */
    for (int i = 0; i < 3; i++) {
        if (isunordered(da, db)) {
            *checksum += 16777216;
            if (!isunordered(da + i, db - i)) {
                *checksum += 33554432;
            }
        } else {
            if (da > db && !isunordered(da, db)) {
                *checksum += 67108864;
            }
            if (da < db || isunordered(da * i, db / (i + 1))) {
                *checksum += 134217728;
            }
        }
        
        /* Ternary operator with uncommon conditions */
        double result = (isunordered(da, db) || da == db) ? da * 2.0 : db / 2.0;
        *checksum += (int)result;
        
        result = (!(da < db) || isunordered(da, db)) ? da + 1.0 : db - 1.0;
        *checksum += (int)result;
        
        result = (isunordered(da, db) || da <= db) ? da * 3.0 : db / 3.0;
        *checksum += (int)result;
        
        result = ((da < db || da > db) && !isunordered(da, db)) ? da + 2.0 : db - 2.0;
        *checksum += (int)result;
    }
}

int main(int argc, char* argv[]) {
    int checksum = 0;
    
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? (unsigned int)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darray[4];
    float farray[4];
    
    for (int i = 0; i < 4; i++) {
        farray[i] = (rand() % 100) / 10.0f;
        darray[i] = (rand() % 100) / 10.0;
        
        /* Introduce some special values */
        if (i == 1) {
            farray[i] = 0.0f / 0.0f; /* NaN */
            darray[i] = 0.0 / 0.0;   /* NaN */
        } else if (i == 2) {
            farray[i] = 1.0f / 0.0f; /* +Inf */
            darray[i] = 1.0 / 0.0;   /* +Inf */
        } else if (i == 3) {
            farray[i] = -1.0f / 0.0f; /* -Inf */
            darray[i] = -1.0 / 0.0;   /* -Inf */
        }
    }
    
    /* Test with different pairs of values */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i != j) {
                /* Scalar tests with O0 optimization */
                test_scalar_conditions_O0(darray[i], darray[j], 
                                         farray[i], farray[j], &checksum);
                
                /* Vector tests with O2 optimization and SSE2 */
                __m128d vda = _mm_set_pd(darray[i], darray[(i+1)%4]);
                __m128d vdb = _mm_set_pd(darray[j], darray[(j+1)%4]);
                __m128 vfa = _mm_set_ps(farray[i], farray[(i+1)%4], 
                                       farray[(i+2)%4], farray[(i+3)%4]);
                __m128 vfb = _mm_set_ps(farray[j], farray[(j+1)%4], 
                                       farray[(j+2)%4], farray[(j+3)%4]);
                test_vector_conditions_O2(vda, vdb, vfa, vfb, &checksum);
                
                /* Inline assembly tests */
                test_inline_asm_conditions(darray[i], darray[j], &checksum);
                
                /* Mixed tests with AVX and O3 */
                test_mixed_conditions_avx(darray[i], darray[j], 
                                         farray[i], farray[j], &checksum);
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
