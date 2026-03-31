#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Vector types for SSE/AVX operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink;

/* Function to generate UNORDERED/ORDERED condition codes */
__attribute__((noinline))
static int test_unordered_ordered(float a, float b, double c, double d) {
    int result = 0;
    
    /* Generate UNORDERED (unord) */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Generate ORDERED (ord) */
    if (!isunordered(a, b)) {
        result |= 2;
    }
    
    /* Mixed ordered/unordered checks */
    if (isunordered(c, d) || (a == b)) {
        result |= 4;
    }
    
    /* Complex expression that may generate UNEQ (ueq) */
    if ((a == a) && (b == b) && (a == b)) {
        result |= 8;
    }
    
    return result;
}

/* Function to generate UNGE/UNGT/UNLE/UNLT condition codes */
__attribute__((noinline))
static int test_unge_ungt_unle_unlt(float a, float b, float c, float d) {
    int result = 0;
    
    /* These may generate UNGE (nlt) under fast-math */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* These may generate UNGT (nle) under fast-math */
    if (!(a <= b)) {
        result |= 2;
    }
    
    /* These may generate UNLE (ule) under fast-math */
    if (a <= b || isunordered(a, b)) {
        result |= 4;
    }
    
    /* These may generate UNLT (ult) under fast-math */
    if (a < b || isunordered(a, b)) {
        result |= 8;
    }
    
    /* Mixed comparisons in conditional */
    result += ((a < b) ? (c >= d) : (d <= c)) ? 16 : 0;
    
    return result;
}

/* Function to generate UNEQ and LTGT condition codes */
__attribute__((noinline))
static int test_uneq_ltgt(double a, double b, double c, double d) {
    int result = 0;
    
    /* This should generate UNEQ (ueq) */
    if (!(a != b)) {  /* Equivalent to a == b including NaNs */
        result |= 1;
    }
    
    /* This should generate LTGT (une) */
    if (islessgreater(a, b)) {
        result |= 2;
    }
    
    /* Complex expression mixing comparisons */
    if ((a == b) ? (c != d) : (a > b)) {
        result |= 4;
    }
    
    /* NaN checks that may generate unordered comparisons */
    if (a != a || b != b) {
        result |= 8;
    }
    
    return result;
}

/* Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int result = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = va == vb;      /* May generate UNEQ */
    v4sf cmp_ne = va != vb;      /* May generate LTGT */
    v4sf cmp_lt = va < vb;       /* May generate UNLT */
    v4sf cmp_le = va <= vb;      /* May generate UNLE */
    v4sf cmp_gt = va > vb;       /* May generate UNGT */
    v4sf cmp_ge = va >= vb;      /* May generate UNGE */
    
    /* Check results */
    for (int i = 0; i < 4; i++) {
        if (cmp_eq[i]) result += 1 << i;
        if (cmp_ne[i]) result += 1 << (i + 4);
        if (cmp_lt[i]) result += 1 << (i + 8);
        if (cmp_le[i]) result += 1 << (i + 12);
    }
    
    /* Double vector comparisons */
    v2df cmp_d_eq = vc == vd;
    v2df cmp_d_ne = vc != vd;
    
    if (cmp_d_eq[0]) result |= 0x10000;
    if (cmp_d_eq[1]) result |= 0x20000;
    if (cmp_d_ne[0]) result |= 0x40000;
    if (cmp_d_ne[1]) result |= 0x80000;
    
    return result;
}

/* SSE intrinsics for precise control */
#ifdef __SSE__
__attribute__((noinline))
static int test_sse_intrinsics(__m128 a, __m128 b) {
    int result = 0;
    
    /* Generate UNORDERED comparison */
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    
    /* Generate ORDERED comparison */
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    
    /* Generate UNEQ comparison */
    __m128 cmp_ueq = _mm_cmpneq_ps(a, b);  /* Not equal */
    
    /* Check results */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_ueq = _mm_movemask_ps(cmp_ueq);
    
    result = mask_unord | (mask_ord << 4) | (mask_ueq << 8);
    
    return result;
}
#endif

/* Complex function with mixed comparison types */
__attribute__((noinline))
static int test_mixed_comparisons(float a, float b, float c, float d, 
                                  double e, double f, double g, double h) {
    int result = 0;
    
    /* Chain of comparisons that may generate various condition codes */
    if ((a < b) && (c >= d)) {
        result |= 1;
    }
    
    if ((e != f) || (g == h)) {
        result |= 2;
    }
    
    /* Ternary with different comparison types */
    result += ((a == b) ? (c != d) : (e < f)) ? 4 : 0;
    
    /* Nested conditionals */
    if (isunordered(a, b)) {
        if (c > d) result |= 8;
    } else {
        if (e <= f) result |= 16;
    }
    
    /* Complex boolean expression */
    if ((a != a) || (b != b) || ((c == d) && (e > f)) || !(g < h)) {
        result |= 32;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Patterned data including NaN values */
    float fdata[8];
    double ddata[8];
    
    /* Initialize with pattern: normal, zero, NaN, infinity */
    for (int i = 0; i < 8; i++) {
        fdata[i] = (i % 4 == 0) ? 1.0f / (i + 1) :
                   (i % 4 == 1) ? 0.0f :
                   (i % 4 == 2) ? __builtin_nanf("") :
                   (i % 4 == 3) ? 1.0f / 0.0f : 1.0f;
        
        ddata[i] = (i % 4 == 0) ? 1.0 / (i + 1) :
                   (i % 4 == 1) ? 0.0 :
                   (i % 4 == 2) ? __builtin_nan("") :
                   (i % 4 == 3) ? 1.0 / 0.0 : 1.0;
    }
    
    /* Use argc to prevent loop unrolling */
    int iterations = (argc > 1) ? argc : 4;
    
    for (int iter = 0; iter < iterations; iter++) {
        sink = iter;  /* Volatile to prevent optimization */
        
        /* Test various comparison patterns */
        total += test_unordered_ordered(
            fdata[iter % 8], fdata[(iter + 1) % 8],
            ddata[iter % 8], ddata[(iter + 2) % 8]
        );
        
        total += test_unge_ungt_unle_unlt(
            fdata[iter % 8], fdata[(iter + 3) % 8],
            fdata[(iter + 1) % 8], fdata[(iter + 2) % 8]
        );
        
        total += test_uneq_ltgt(
            ddata[iter % 8], ddata[(iter + 1) % 8],
            ddata[(iter + 2) % 8], ddata[(iter + 3) % 8]
        );
        
        /* Vector tests */
        v4sf va = {fdata[0], fdata[1], fdata[2], fdata[3]};
        v4sf vb = {fdata[4], fdata[5], fdata[6], fdata[7]};
        v2df vc = {ddata[0], ddata[1]};
        v2df vd = {ddata[2], ddata[3]};
        
        total += test_vector_comparisons(va, vb, vc, vd);
        
#ifdef __SSE__
        __m128 ma = _mm_set_ps(fdata[3], fdata[2], fdata[1], fdata[0]);
        __m128 mb = _mm_set_ps(fdata[7], fdata[6], fdata[5], fdata[4]);
        total += test_sse_intrinsics(ma, mb);
#endif
        
        /* Mixed comparisons test */
        total += test_mixed_comparisons(
            fdata[iter % 8], fdata[(iter + 1) % 8],
            fdata[(iter + 2) % 8], fdata[(iter + 3) % 8],
            ddata[iter % 8], ddata[(iter + 1) % 8],
            ddata[(iter + 2) % 8], ddata[(iter + 3) % 8]
        );
    }
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
