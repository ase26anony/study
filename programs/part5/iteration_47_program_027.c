/* 
 * Program to trigger x86 condition code mnemonics in i86.cc
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Prevent optimization of critical values */
static volatile float vf1, vf2;
static volatile double vd1, vd2;

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !islessgreater(c, d) ? 4 : 0;  /* a == b or unordered */
    
    /* UNGE: not less than (greater or equal or unordered) */
    sum += !isless(c, d) ? 8 : 0;
    
    /* UNGT: not less or equal (greater or unordered) */
    sum += !islessequal(c, d) ? 16 : 0;
    
    /* UNLE: less or equal or unordered */
    sum += islessequal(a, b) ? 32 : 0;
    
    /* UNLT: less than or unordered */
    sum += isless(a, b) ? 64 : 0;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 4; i++) {
        /* Ternary with different FP comparisons in branches */
        sum += ((a < b) ? (c != d) : (e >= f)) ? (1 << i) : 0;
        
        /* Nested conditionals */
        if ((a == b) || (c != c)) {  /* c != c checks for NaN */
            sum += (g > h) ? 10 : 20;
        } else if (!(e < f)) {  /* UNGE: not less than */
            sum += (g <= h) ? 30 : 40;
        }
        
        /* Rotate values to create different comparison scenarios */
        float temp = a;
        a = b; b = c; c = d; d = temp;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_result;
    
    /* Unordered comparison (any element is NaN) */
    cmp_result = va != vb;  /* Generates UNORD-like behavior */
    sum += ((int*)&cmp_result)[0] & 1;
    
    /* Ordered less-than */
    cmp_result = va < vb;
    sum += ((int*)&cmp_result)[1] & 2;
    
    /* Ordered greater-than or equal */
    cmp_result = va >= vb;
    sum += ((int*)&cmp_result)[2] & 4;
    
    /* Not equal (ordered or unordered) */
    cmp_result = va != vb;
    sum += ((int*)&cmp_result)[3] & 8;
    
    /* Double vector comparisons */
    v2df dbl_cmp = vc == vd;
    sum += ((long long*)&dbl_cmp)[0] ? 16 : 0;
    
    dbl_cmp = vc < vd;
    sum += ((long long*)&dbl_cmp)[1] ? 32 : 0;
    
    return sum;
}

/* Test function 4: AVX intrinsics for explicit unordered comparisons */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m128 a, __m128 b, __m128d c, __m128d d) {
    int sum = 0;
    
    /* _CMP_UNORD_Q: unordered (quiet) - true if either operand is NaN */
    __m128 cmp = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    sum += _mm_movemask_ps(cmp) & 0xF;
    
    /* _CMP_NEQ_UQ: not equal (unordered, quiet) */
    cmp = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    sum += (_mm_movemask_ps(cmp) & 0xF) << 4;
    
    /* _CMP_NLT_UQ: not less than (unordered, quiet) - UNGE */
    cmp = _mm_cmp_ps(a, b, _CMP_NLT_UQ);
    sum += (_mm_movemask_ps(cmp) & 0xF) << 8;
    
    /* _CMP_NLE_UQ: not less or equal (unordered, quiet) - UNGT */
    cmp = _mm_cmp_ps(a, b, _CMP_NLE_UQ);
    sum += (_mm_movemask_ps(cmp) & 0xF) << 12;
    
    /* Double precision comparisons */
    __m128d cmp_d = _mm_cmp_pd(c, d, _CMP_UNORD_Q);
    sum += _mm_movemask_pd(cmp_d) << 16;
    
    cmp_d = _mm_cmp_pd(c, d, _CMP_EQ_UQ);  /* equal (unordered, quiet) - UNEQ */
    sum += _mm_movemask_pd(cmp_d) << 18;
    
    return sum;
}
#endif

/* Test function 5: Chain of comparisons with multiple parameters */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d,
                                 float e, float f, float g, float h) {
    int result = 0;
    
    /* Chain of comparisons that should generate various condition codes */
    if (a < b) result |= 1;      /* LT */
    if (c >= d) result |= 2;     /* GE */
    if (e != f) result |= 4;     /* NEQ (could be UNEQ with fast-math) */
    if (!(g > h)) result |= 8;   /* UNLE: not greater than */
    if (b <= a) result |= 16;    /* LE */
    if (!(d < c)) result |= 32;  /* UNGE: not less than */
    if (a == a && b == b) result |= 64;  /* ORDERED check */
    if (c != c || d != d) result |= 128; /* UNORDERED check */
    
    /* Mixed in ternary */
    result += (a == b) ? 
              ((c < d) ? 256 : 512) : 
              ((e > f) ? 1024 : 2048);
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normal numbers, zeros, and some NaNs */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 4 == 0) ? 0.0f : 
                   (i % 4 == 1) ? (float)i * 1.5f : 
                   (i % 4 == 2) ? -(float)i * 0.5f : 
                   __builtin_nanf("");  /* NaN for every 4th element */
        
        ddata[i] = (i % 3 == 0) ? 0.0 : 
                   (i % 3 == 1) ? (double)i * 2.5 : 
                   __builtin_nan("");
    }
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? argc : 4;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            fdata[iter % 16], fdata[(iter + 1) % 16],
            ddata[iter % 16], ddata[(iter + 2) % 16]
        );
        
        /* Test 2: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            fdata[iter % 16], fdata[(iter + 1) % 16],
            fdata[(iter + 2) % 16], fdata[(iter + 3) % 16],
            ddata[iter % 16], ddata[(iter + 1) % 16],
            ddata[(iter + 2) % 16], ddata[(iter + 3) % 16]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {fdata[iter % 16], fdata[(iter + 1) % 16], 
                   fdata[(iter + 2) % 16], fdata[(iter + 3) % 16]};
        v4sf vb = {fdata[(iter + 4) % 16], fdata[(iter + 5) % 16], 
                   fdata[(iter + 6) % 16], fdata[(iter + 7) % 16]};
        v2df vc = {ddata[iter % 16], ddata[(iter + 1) % 16]};
        v2df vd = {ddata[(iter + 2) % 16], ddata[(iter + 3) % 16]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: AVX intrinsics (if available) */
        #ifdef __AVX__
        __m128 avx_a = _mm_loadu_ps(&fdata[iter % 16]);
        __m128 avx_b = _mm_loadu_ps(&fdata[(iter + 4) % 16]);
        __m128d avx_c = _mm_loadu_pd(&ddata[iter % 16]);
        __m128d avx_d = _mm_loadu_pd(&ddata[(iter + 2) % 16]);
        
        total_sum += test_avx_intrinsics(avx_a, avx_b, avx_c, avx_d);
        #endif
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            fdata[iter % 16], fdata[(iter + 1) % 16],
            fdata[(iter + 2) % 16], fdata[(iter + 3) % 16],
            fdata[(iter + 4) % 16], fdata[(iter + 5) % 16],
            fdata[(iter + 6) % 16], fdata[(iter + 7) % 16]
        );
        
        /* Modify data slightly each iteration to prevent constant folding */
        fdata[iter % 16] += 0.1f * iter;
        ddata[iter % 16] += 0.05 * iter;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    /* Use volatile values to force generation of comparison code */
    vf1 = fdata[0];
    vf2 = fdata[1];
    vd1 = ddata[0];
    vd2 = ddata[1];
    
    /* One more unordered check with volatiles */
    total_sum += isunordered(vf1, vf2) ? 1000 : 0;
    total_sum += islessgreater(vd1, vd2) ? 2000 : 0;
    
    printf("Final result: %d\n", total_sum);
    
    return total_sum > 0 ? 0 : 1;
}
