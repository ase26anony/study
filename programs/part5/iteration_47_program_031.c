/* Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical values */
static volatile float vf = 0.0f;
static volatile double vd = 0.0;

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to trigger unordered comparisons */
__attribute__((noinline))
static int test_unordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* Ordered checks */
    sum += isordered(a + 1.0f, b - 1.0f) ? 4 : 0;
    
    /* UNEQ: unordered or equal */
    if (!(a < b) && !(a > b)) sum += 8;  /* Generates UNEQ with -ffast-math */
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) sum += 16;   /* Generates LTGT with -ffast-math */
    
    return sum;
}

/* Function to trigger UNGE/UNGT/UNLE/UNLT */
__attribute__((noinline))
static int test_unordered_inequalities(float a, float b, float c, float d) {
    int sum = 0;
    
    /* Mixed comparisons that can generate unordered variants */
    if (!(a < b)) sum += 1;   /* UNGE: not less than (greater or equal or unordered) */
    if (!(a <= b)) sum += 2;  /* UNGT: not less or equal (greater or unordered) */
    if (!(c > d)) sum += 4;   /* UNLE: not greater than (less or equal or unordered) */
    if (!(c >= d)) sum += 8;  /* UNLT: not greater or equal (less or unordered) */
    
    /* Complex conditional expression */
    sum += ((a < b) ? (c != d) : (b >= a)) ? 16 : 0;
    
    return sum;
}

/* Function using vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate multiple condition codes */
    v4sf cmp_result = va < vb;
    v2df cmp_result_d = vc > vd;
    
    /* Extract results to prevent elimination */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(temp));
    for (int i = 0; i < 4; i++) {
        sum += (temp[i] != 0.0f) ? (1 << i) : 0;
    }
    
    double temp_d[2];
    memcpy(temp_d, &cmp_result_d, sizeof(temp_d));
    for (int i = 0; i < 2; i++) {
        sum += (temp_d[i] != 0.0) ? (32 << i) : 0;
    }
    
    return sum;
}

/* Function using SSE intrinsics */
__attribute__((noinline))
static int test_sse_intrinsics(__m128 a, __m128 b) {
    int sum = 0;
    
    /* Generate various condition codes through intrinsics */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);   /* UNORD */
    __m128 cmp_ord = _mm_cmpord_ps(a, b);       /* ORD */
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);       /* NEQ (can become UNEQ/ORD) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);       /* NLT (UNGE) */
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);       /* NLE (UNGT) */
    
    /* Extract mask bits */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_neq = _mm_movemask_ps(cmp_neq);
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    
    sum = mask_unord + (mask_ord << 4) + (mask_neq << 8) + 
          (mask_nlt << 12) + (mask_nle << 16);
    
    return sum;
}

/* Complex function with mixed comparisons */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain of comparisons that forces multiple condition codes */
    sum += (a < b) ? ((c != d) ? 1 : 2) : ((e >= f) ? 4 : 8);
    
    /* Nested ternary with different comparison types */
    int val = (g != h) ? 
              ((a == b) ? 16 : 32) : 
              ((c <= d) ? 64 : 128);
    sum += val;
    
    /* Combined ordered/unordered checks */
    if ((a < b) && isunordered(e, f)) sum += 256;
    if ((c > d) || isordered(g, h)) sum += 512;
    
    /* LTGT pattern */
    if ((e < f) || (e > f)) sum += 1024;  /* Ordered and not equal */
    
    /* UNEQ pattern through negation */
    if (!((a < b) || (a > b))) sum += 2048;  /* Not less and not greater */
    
    return sum;
}

/* Function that can produce NaN values */
__attribute__((noinline))
static int test_nan_handling(float a, float b, double c, double d) {
    int sum = 0;
    
    /* NaN checks using direct comparisons */
    sum += (a != a) ? 1 : 0;      /* true if a is NaN */
    sum += !(c == c) ? 2 : 0;     /* true if c is NaN */
    
    /* Ordered comparisons that become unordered with NaN inputs */
    sum += (a < b) ? 4 : 0;
    sum += (c > d) ? 8 : 0;
    
    /* Check for unordered without using isunordered() */
    sum += ((a >= b) && (a <= b) && (a == b)) ? 0 : 16;
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[8];
    double ddata[8];
    
    /* Initialize with pattern: normal, zero, inf, nan */
    for (int i = 0; i < 8; i++) {
        fdata[i] = (i % 4 == 0) ? 1.0f * i : 
                   (i % 4 == 1) ? 0.0f :
                   (i % 4 == 2) ? 1.0f / 0.0f :  /* inf */
                   __builtin_nanf("");           /* nan */
        
        ddata[i] = (i % 4 == 0) ? 2.0 * i : 
                   (i % 4 == 1) ? -0.0 :
                   (i % 4 == 2) ? -1.0 / 0.0 :   /* -inf */
                   __builtin_nan("");
    }
    
    /* Use argc to prevent excessive unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        float f1 = fdata[iter % 8] + vf;
        float f2 = fdata[(iter + 1) % 8] - vf;
        float f3 = fdata[(iter + 2) % 8] * (1.0f + iter * 0.01f);
        float f4 = fdata[(iter + 3) % 8] / (1.0f + iter * 0.01f);
        
        double d1 = ddata[iter % 8] + vd;
        double d2 = ddata[(iter + 1) % 8] - vd;
        double d3 = ddata[(iter + 2) % 8] * (1.0 + iter * 0.01);
        double d4 = ddata[(iter + 3) % 8] / (1.0 + iter * 0.01);
        
        /* Call all test functions */
        total_sum += test_unordered(f1, f2, d1, d2);
        total_sum += test_unordered_inequalities(f1, f2, f3, f4);
        
        /* Vector tests */
        v4sf va = {f1, f2, f3, f4};
        v4sf vb = {f2, f3, f4, f1};
        v2df vc = {d1, d2};
        v2df vd = {d2, d1};
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* SSE intrinsics */
        __m128 sse_a = _mm_set_ps(f4, f3, f2, f1);
        __m128 sse_b = _mm_set_ps(f1, f2, f3, f4);
        total_sum += test_sse_intrinsics(sse_a, sse_b);
        
        /* Mixed comparisons */
        total_sum += test_mixed_comparisons(f1, f2, f3, f4, d1, d2, d3, d4);
        
        /* NaN handling */
        total_sum += test_nan_handling(f1, f2, d1, d2);
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
