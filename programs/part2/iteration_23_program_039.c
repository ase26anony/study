#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization of test functions */
#define NOINLINE __attribute__((noinline))
#define OPTIMIZE(level) __attribute__((optimize(level)))

/* Checksum to prevent dead code elimination */
static volatile unsigned long checksum = 0;

/* Test function for scalar floating-point conditions */
NOINLINE OPTIMIZE("O0")
static void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) */
    result = isunordered(a, b);
    checksum += result;
    
    /* Test ORDERED (ord) */
    result = !isunordered(a, b);
    checksum += result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    result = (isunordered(a, b) || (a == b));
    checksum += result;
    
    /* Test UNGE (nlt) - unordered or greater or equal */
    result = (isunordered(a, b) || (a >= b));
    checksum += result;
    
    /* Test UNGT (nle) - unordered or greater */
    result = (isunordered(a, b) || (a > b));
    checksum += result;
    
    /* Test UNLE (ule) - unordered or less or equal */
    result = (isunordered(a, b) || (a <= b));
    checksum += result;
    
    /* Test UNLT (ult) - unordered or less */
    result = (isunordered(a, b) || (a < b));
    checksum += result;
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    result = (!isunordered(a, b) && (a != b));
    checksum += result;
    
    /* Complex branching to force condition code generation */
    if (isunordered(fa, fb)) {
        checksum += 1;
    } else if (!isunordered(fa, fb) && fa != fb) {
        checksum += 2;
    } else if (fa >= fb || isunordered(fa, fb)) {
        checksum += 3;
    }
}

/* Test function with SSE2 vector conditions */
NOINLINE OPTIMIZE("O2") __attribute__((target("sse2")))
static void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_result;
    __m128 cmp_result_f;
    volatile double dresult[2];
    volatile float fresult[4];
    
    /* Test UNORDERED (unord) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test ORDERED (ord) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNEQ (ueq) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNGE (nlt) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNGT (nle) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNLE (ule) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNLT (ult) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test LTGT (une) for vectors */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test with float vectors */
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fresult, cmp_result_f);
    checksum += (int)fresult[0] + (int)fresult[1] + (int)fresult[2] + (int)fresult[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fresult, cmp_result_f);
    checksum += (int)fresult[0] + (int)fresult[1] + (int)fresult[2] + (int)fresult[3];
}

/* Test function with inline assembly using condition codes */
NOINLINE OPTIMIZE("O1") __attribute__((target("sse2")))
static void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile double result;
    volatile __m128d vresult;
    
    /* Inline assembly with UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with UNEQ (ueq) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Inline assembly with LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += (unsigned long)result;
    
    /* Vector inline assembly with UNORDERED */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    
    /* Vector inline assembly with ORDERED */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
}

/* Test with mixed optimization levels */
NOINLINE __attribute__((optimize("O3")))
static void test_optimized_conditions(double a, double b, float fa, float fb) {
    volatile double temp = a;
    volatile float ftemp = fa;
    
    /* Loop to create more complex control flow */
    for (int i = 0; i < 3; i++) {
        /* Use ternary operator with uncommon conditions */
        double d = (isunordered(temp, b) || temp < b) ? temp : b;
        checksum += (unsigned long)d;
        
        float f = (!isunordered(ftemp, fb) && ftemp != fb) ? ftemp : fb;
        checksum += (unsigned long)f;
        
        temp += 1.0;
        ftemp += 1.0f;
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values including NaN and normal numbers */
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with various floating-point values */
    double test_doubles[] = {
        1.0, 2.0, 0.0, -1.0, 
        __builtin_nan(""), __builtin_inf(), -__builtin_inf()
    };
    
    float test_floats[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        __builtin_nanf(""), __builtin_inff(), -__builtin_inff()
    };
    
    /* Initialize vector data */
    __m128d v1 = _mm_set_pd(test_doubles[0], test_doubles[1]);
    __m128d v2 = _mm_set_pd(test_doubles[2], test_doubles[3]);
    __m128 f1 = _mm_set_ps(test_floats[0], test_floats[1], test_floats[2], test_floats[3]);
    __m128 f2 = _mm_set_ps(test_floats[4], test_floats[5], test_floats[6], test_floats[0]);
    
    /* Run all test functions with different combinations */
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (i == j) continue; /* Skip identical values for more interesting comparisons */
            
            test_scalar_conditions(test_doubles[i], test_doubles[j], 
                                  test_floats[i % 7], test_floats[j % 7]);
            
            test_vector_conditions(v1, v2, f1, f2);
            
            test_inline_asm_conditions(test_doubles[i], test_doubles[j], v1, v2);
            
            test_optimized_conditions(test_doubles[i], test_doubles[j],
                                     test_floats[i % 7], test_floats[j % 7]);
        }
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
