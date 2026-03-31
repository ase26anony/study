/* Test program to cover condition code output in i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile double checksum = 0.0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"))) 
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* UNORDERED: unord */
    res = isunordered(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* ORDERED: ord */
    res = !isunordered(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* UNEQ: ueq (unordered or equal) */
    res = !isgreater(a, b) && !isless(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* UNGE: nlt (not less than) */
    res = !isless(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* UNGT: nle (not less than or equal) */
    res = !islessequal(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* UNLE: ule (unordered or less than or equal) */
    res = isunordered(a, b) || islessequal(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* UNLT: ult (unordered or less than) */
    res = isunordered(a, b) || isless(a, b);
    checksum += res;
    if (res) global_counter++;
    
    /* LTGT: une (not equal and ordered) */
    res = !isunordered(a, b) && a != b;
    checksum += res;
    if (res) global_counter++;
    
    /* Float variants */
    res = isunordered(fa, fb);
    checksum += res;
    res = !isunordered(fa, fb);
    checksum += res;
    res = !isgreater(fa, fb) && !isless(fa, fb);
    checksum += res;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* UNORDERED: unord */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* ORDERED: ord */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* UNEQ: ueq */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* UNGE: nlt */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* UNGT: nle */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* UNLE: ule */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* UNLT: ult */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* LTGT: une */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += dres[0] + dres[1];
    
    /* Float vector versions */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    checksum += fres[0] + fres[1] + fres[2] + fres[3];
    
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    checksum += fres[0] + fres[1] + fres[2] + fres[3];
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int cc_result;
    
    /* Inline assembly with condition code mnemonics */
    /* UNORDERED: unord */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* ORDERED: ord */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* UNEQ: ueq - Using cmppd with template */
    __asm__ volatile (
        "cmppd %[ueq], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
    
    /* UNGE: nlt */
    __asm__ volatile (
        "cmppd %[nlt], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
    
    /* UNGT: nle */
    __asm__ volatile (
        "cmppd %[nle], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
    
    /* UNLE: ule */
    __asm__ volatile (
        "cmppd %[ule], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
    
    /* UNLT: ult */
    __asm__ volatile (
        "cmppd %[ult], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
    
    /* LTGT: une */
    __asm__ volatile (
        "cmppd %[une], %2, %1\n\t"
        "movq %1, %0"
        : "=m" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += result;
}

__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions_O3(__m256d v1, __m256d v2) {
    __m256d cmp_res;
    volatile double dres[4];
    
    /* Test all conditions with AVX */
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += dres[0] + dres[1] + dres[2] + dres[3];
}

int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    srand(seed);
    
    /* Initialize test data with varied values including NaN, Inf, normal */
    double dvals[8];
    float fvals[16];
    
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: dvals[i] = (double)rand() / RAND_MAX * 100.0; break;
            case 1: dvals[i] = -((double)rand() / RAND_MAX * 100.0); break;
            case 2: dvals[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: dvals[i] = 1.0 / 0.0; /* Inf */ break;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        switch (i % 4) {
            case 0: fvals[i] = (float)rand() / RAND_MAX * 100.0f; break;
            case 1: fvals[i] = -((float)rand() / RAND_MAX * 100.0f); break;
            case 2: fvals[i] = 0.0f / 0.0f; /* NaN */ break;
            case 3: fvals[i] = 1.0f / 0.0f; /* Inf */ break;
        }
    }
    
    /* Call test functions with different data combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions_O0(dvals[i*2], dvals[i*2+1], 
                                  fvals[i*4], fvals[i*4+1]);
        
        __m128d v1 = _mm_set_pd(dvals[i*2], dvals[i*2+1]);
        __m128d v2 = _mm_set_pd(dvals[(i*2+2)%8], dvals[(i*2+3)%8]);
        __m128 f1 = _mm_set_ps(fvals[i*4], fvals[i*4+1], 
                               fvals[i*4+2], fvals[i*4+3]);
        __m128 f2 = _mm_set_ps(fvals[(i*4+4)%16], fvals[(i*4+5)%16],
                               fvals[(i*4+6)%16], fvals[(i*4+7)%16]);
        
        test_vector_conditions_O2(v1, v2, f1, f2);
        test_inline_asm_conditions(dvals[i*2], dvals[i*2+1]);
        
        if (i < 2) {
            __m256d av1 = _mm256_set_pd(dvals[0], dvals[1], dvals[2], dvals[3]);
            __m256d av2 = _mm256_set_pd(dvals[4], dvals[5], dvals[6], dvals[7]);
            test_avx_conditions_O3(av1, av2);
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
