/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization */
static volatile int global_counter = 0;
static volatile double checksum = 0.0;

/* Different optimization levels to test various code paths */
__attribute__((optimize("O0"))) 
void test_scalar_conditions(double a, double b, float fa, float fb) {
    /* Test UNORDERED/ORDERED conditions */
    if (isunordered(a, b)) {
        checksum += 1.0;
    }
    
    /* Test UNEQ condition */
    if (!(a > b) && !(a < b) && !isunordered(a, b)) {
        checksum += 2.0;
    }
    
    /* Test UNGE condition (nlt) */
    if (!(a < b) || isunordered(a, b)) {
        checksum += 3.0;
    }
    
    /* Test UNGT condition (nle) */
    if (!(a <= b) || isunordered(a, b)) {
        checksum += 4.0;
    }
    
    /* Test UNLE condition (ule) */
    if ((a <= b) || isunordered(a, b)) {
        checksum += 5.0;
    }
    
    /* Test UNLT condition (ult) */
    if ((a < b) || isunordered(a, b)) {
        checksum += 6.0;
    }
    
    /* Test LTGT condition (une) */
    if ((a < b) || (a > b)) {
        checksum += 7.0;
    }
    
    /* Test ORDERED condition */
    if (!isunordered(a, b)) {
        checksum += 8.0;
    }
    
    /* Complex nested conditions to prevent optimization */
    volatile double result = 0.0;
    for (int i = 0; i < 3; i++) {
        if (isunordered(fa, fb)) {
            result += 1.5;
            if (!(fa > fb) && !(fa < fb)) {
                result *= 1.1;
            }
        } else {
            result -= 0.5;
            if ((fa <= fb) || isunordered(fa, fb)) {
                result *= 0.9;
            }
        }
    }
    checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    /* Test UNORDERED */
    __m128d cmp_unord = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    __m128d mask1 = _mm_and_pd(v1, cmp_unord);
    checksum += ((double*)&mask1)[0] + ((double*)&mask1)[1];
    
    /* Test ORDERED */
    __m128d cmp_ord = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    __m128d mask2 = _mm_and_pd(v1, cmp_ord);
    checksum += ((double*)&mask2)[0] + ((double*)&mask2)[1];
    
    /* Test UNEQ */
    __m128d cmp_ueq = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    __m128d mask3 = _mm_and_pd(v1, cmp_ueq);
    checksum += ((double*)&mask3)[0] + ((double*)&mask3)[1];
    
    /* Test UNGE (nlt) */
    __m128d cmp_nlt = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    __m128d mask4 = _mm_and_pd(v1, cmp_nlt);
    checksum += ((double*)&mask4)[0] + ((double*)&mask4)[1];
    
    /* Test UNGT (nle) */
    __m128d cmp_nle = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    __m128d mask5 = _mm_and_pd(v1, cmp_nle);
    checksum += ((double*)&mask5)[0] + ((double*)&mask5)[1];
    
    /* Test UNLE (ule) */
    __m128d cmp_ule = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    __m128d mask6 = _mm_and_pd(v1, cmp_ule);
    checksum += ((double*)&mask6)[0] + ((double*)&mask6)[1];
    
    /* Test UNLT (ult) */
    __m128d cmp_ult = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    __m128d mask7 = _mm_and_pd(v1, cmp_ult);
    checksum += ((double*)&mask7)[0] + ((double*)&mask7)[1];
    
    /* Test LTGT (une) */
    __m128d cmp_une = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    __m128d mask8 = _mm_and_pd(v1, cmp_une);
    checksum += ((double*)&mask8)[0] + ((double*)&mask8)[1];
    
    /* Test with float vectors */
    __m128 cmp_unord_f = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    __m128 mask_f = _mm_and_ps(f1, cmp_unord_f);
    checksum += ((float*)&mask_f)[0] + ((float*)&mask_f)[1] + 
                ((float*)&mask_f)[2] + ((float*)&mask_f)[3];
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    double result;
    
    /* Test UNORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test ORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test UNEQ */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result;
}

__attribute__((optimize("O3"), target("avx")))
void test_mixed_conditions(double a, double b, float fa, float fb) {
    volatile int cond1 = isunordered(a, b);
    volatile int cond2 = !(a > b) && !(a < b);
    volatile int cond3 = !(a < b);
    volatile int cond4 = !(a <= b);
    volatile int cond5 = (a <= b);
    volatile int cond6 = (a < b);
    volatile int cond7 = (a < b) || (a > b);
    volatile int cond8 = !isunordered(a, b);
    
    /* Force use of all conditions in complex expression */
    double result = 0.0;
    result += cond1 ? 1.1 : 0.1;
    result += cond2 ? 2.2 : 0.2;
    result += cond3 ? 3.3 : 0.3;
    result += cond4 ? 4.4 : 0.4;
    result += cond5 ? 5.5 : 0.5;
    result += cond6 ? 6.6 : 0.6;
    result += cond7 ? 7.7 : 0.7;
    result += cond8 ? 8.8 : 0.8;
    
    /* Mix with float operations */
    result += isunordered(fa, fb) ? 9.9 : 0.9;
    result += (!(fa > fb) && !(fa < fb)) ? 10.1 : 1.1;
    
    checksum += result;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with various special cases */
    double test_doubles[] = {
        1.0, 2.0, -1.0, 0.0, 
        INFINITY, -INFINITY, NAN,
        (double)rand() / RAND_MAX * 100.0
    };
    
    float test_floats[] = {
        1.5f, 2.5f, -1.5f, 0.0f,
        INFINITY, -INFINITY, NAN,
        (float)rand() / RAND_MAX * 100.0f
    };
    
    /* Test all combinations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double a = test_doubles[i];
            double b = test_doubles[j];
            float fa = test_floats[i];
            float fb = test_floats[j];
            
            /* Create vector values */
            __m128d v1 = _mm_set_pd(a, b);
            __m128d v2 = _mm_set_pd(b, a);
            __m128 f1 = _mm_set_ps(fa, fb, fa, fb);
            __m128 f2 = _mm_set_ps(fb, fa, fb, fa);
            
            /* Call test functions */
            test_scalar_conditions(a, b, fa, fb);
            test_vector_conditions(v1, v2, f1, f2);
            test_inline_asm_conditions(a, b);
            test_mixed_conditions(a, b, fa, fb);
            
            global_counter++;
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Tests executed: %d\n", global_counter);
    
    return 0;
}
