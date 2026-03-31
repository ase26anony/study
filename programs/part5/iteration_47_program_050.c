/* cc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c -lm */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent optimization of critical values */
static volatile float vnan = __builtin_nanf("");
static volatile double dnan = __builtin_nan("");

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with noinline to preserve RTL patterns */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate "unord" */
    if (isunordered(a, b)) sum |= 1;
    if (!isunordered(c, d)) sum |= 2;  /* "ord" */
    
    /* NaN checks that may generate unordered comparisons */
    if (a != a) sum |= 4;      /* Always true if a is NaN */
    if (!(c == c)) sum |= 8;   /* Always false unless c is NaN */
    
    /* Mixed ordered/unordered in conditional */
    sum += (isunordered(a, b) ? (c < d) : (c > d)) ? 16 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_uneq_unge(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These should generate UNEQ (ueq) under fast-math */
    if (a == b || isunordered(a, b)) sum |= 1;
    if (!(a != b) || isunordered(a, b)) sum |= 2;
    
    /* UNGE (nlt) - not less than */
    if (!(a < b) || isunordered(a, b)) sum |= 4;
    
    /* UNGT (nle) - not less than or equal */
    if (!(a <= b) || isunordered(a, b)) sum |= 8;
    
    /* Complex conditional mixing comparisons */
    sum += ((a < b) && !isunordered(a, b)) ? 16 : 0;
    sum += ((c >= d) || isunordered(c, d)) ? 32 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_unle_unlt_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* UNLE (ule) - unordered or less than or equal */
    if (a <= b || isunordered(a, b)) sum |= 1;
    
    /* UNLT (ult) - unordered or less than */
    if (a < b || isunordered(a, b)) sum |= 2;
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    if (islessgreater(a, b)) sum |= 4;
    
    /* Mixed in ternary operator */
    sum += (isunordered(a, b) ? (c != d) : (a > b)) ? 8 : 0;
    
    /* Chain of comparisons */
    if ((a < b) ? (c != d) : (b >= a)) sum |= 16;
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate various condition codes */
    v4sf vcmp = va < vb;
    v2df vcmpd = vc > vd;
    
    /* Extract results to prevent elimination */
    float fcmp[4];
    double dcmp[2];
    memcpy(fcmp, &vcmp, sizeof(vcmp));
    memcpy(dcmp, &vcmpd, sizeof(vcmpd));
    
    for (int i = 0; i < 4; i++) {
        sum += (fcmp[i] != 0.0f) ? (1 << i) : 0;
    }
    for (int i = 0; i < 2; i++) {
        sum += (dcmp[i] != 0.0) ? (1 << (i + 4)) : 0;
    }
    
    /* Unordered vector comparison */
    v4sf vunord = va != va | vb != vb;  /* Check for NaN elements */
    memcpy(fcmp, &vunord, sizeof(vunord));
    for (int i = 0; i < 4; i++) {
        sum += (fcmp[i] != 0.0f) ? (1 << (i + 8)) : 0;
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_precision(float f1, float f2, double d1, double d2) {
    int sum = 0;
    
    /* Cross-type comparisons */
    if ((double)f1 < d1 || isunordered(f1, f2)) sum |= 1;
    if (f2 > (float)d2 && !isunordered(d1, d2)) sum |= 2;
    
    /* Complex expression with multiple condition codes */
    sum += (isunordered(f1, f2) ? 
           (d1 != d2 || islessgreater(d1, d2)) : 
           (f1 == f2 && !isunordered(d1, d2))) ? 4 : 0;
    
    /* Nested conditionals */
    if (f1 <= f2) {
        if (isunordered(d1, d2)) sum |= 8;
        else if (d1 > d2) sum |= 16;
    } else {
        if (!isunordered(f1, f2)) sum |= 32;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[] = {1.0f, 2.0f, 0.0f, -1.0f, vnan, 3.0f, __builtin_inff(), -__builtin_inff()};
    double ddata[] = {1.0, 2.0, 0.0, -1.0, dnan, 3.0, __builtin_inf(), -__builtin_inf()};
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, vnan, 4.0f};
    v4sf v2 = {2.0f, 1.0f, 3.0f, vnan};
    v2df vd1 = {1.0, dnan};
    v2df vd2 = {dnan, 2.0};
    
    /* Use argc to prevent excessive unrolling */
    int iterations = (argc > 1) ? (argc & 7) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        int idx1 = i % 8;
        int idx2 = (i + 1) % 8;
        int idx3 = (i + 2) % 8;
        int idx4 = (i + 3) % 8;
        
        /* Call all test functions with different data patterns */
        total_sum += test_unordered_comparisons(
            fdata[idx1], fdata[idx2], 
            ddata[idx3], ddata[idx4]);
        
        total_sum += test_uneq_unge(
            fdata[idx1], fdata[idx2],
            fdata[idx3], fdata[idx4]);
        
        total_sum += test_unle_unlt_ltgt(
            ddata[idx1], ddata[idx2],
            ddata[idx3], ddata[idx4]);
        
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        total_sum += test_mixed_precision(
            fdata[idx1], fdata[idx2],
            ddata[idx3], ddata[idx4]);
        
        /* Modify vectors slightly each iteration */
        v1[0] += 0.1f;
        v2[1] -= 0.1f;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
