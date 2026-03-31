/* Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c -lm */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent inlining to ensure RTL generation */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks - should generate UNORD/ORDERED */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* NaN checks using self-comparison */
    sum += (a != a) ? 4 : 0;      /* UNORD for a */
    sum += !(c == c) ? 8 : 0;     /* UNORD for c */
    
    /* Mixed ordered/unordered comparisons */
    if ((isunordered(a, b) && (c > d)) || (!isunordered(a, b) && (c <= d))) {
        sum += 16;
    }
    
    return sum;
}

__attribute__((noinline))
static int test_uneq_ltgt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These should generate UNEQ and LTGT under -ffast-math */
    sum += (a == b) ? 1 : 0;      /* May become UNEQ */
    sum += (a != b) ? 2 : 0;      /* May become LTGT */
    
    /* Complex expression mixing comparisons */
    sum += ((a < b) ? (c == d) : (a > b)) ? 4 : 0;
    sum += ((a <= b) != (c >= d)) ? 8 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_unge_ungt_unle_unlt(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate the UN* condition codes */
    sum += !(a < b) ? 1 : 0;      /* UNGE or UNLT inverse */
    sum += !(a <= b) ? 2 : 0;     /* UNGT */
    sum += !(c > d) ? 4 : 0;      /* UNLE */
    sum += !(c >= d) ? 8 : 0;     /* UNLT */
    
    /* Ternary with different comparison types */
    float res = (a != a) ? b : ((b > a) ? c : d);
    sum += (res > 0) ? 16 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons - may use UNORD/ORDERED codes */
    v4sf vcmp_unord = (v4sf)(va != va) | (v4sf)(vb != vb);
    v4sf vcmp_ord = (v4sf)(va == va) & (v4sf)(vb == vb);
    
    /* Extract results to prevent elimination */
    float fvals[4];
    memcpy(fvals, &vcmp_unord, sizeof(fvals));
    memcpy(fvals + 2, &vcmp_ord, sizeof(v4sf));
    
    for (int i = 0; i < 4; i++) {
        sum += (fvals[i] != 0.0f) ? (1 << i) : 0;
    }
    
    /* Vector ordered comparisons */
    v4sf vcmp_eq = va == vb;      /* May become UNEQ */
    v4sf vcmp_neq = va != vb;     /* May become LTGT */
    
    memcpy(fvals, &vcmp_eq, sizeof(v4sf));
    memcpy(fvals + 2, &vcmp_neq, sizeof(v4sf));
    
    for (int i = 0; i < 4; i++) {
        sum += (fvals[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_conditional_chain(float a, float b, float c, float d, 
                                        double e, double f, double g, double h) {
    int sum = 0;
    
    /* Complex chain of mixed comparisons */
    if ((a < b) ? (c != d) : (e >= f)) {
        sum += 1;
    }
    
    if (!(a == b) && (g != h)) {
        sum += 2;
    }
    
    /* Nested ternary with different float types */
    double result = (isunordered(a, c)) ? 
                   ((b > d) ? e : f) : 
                   ((islessgreater(e, f)) ? g : h);
    sum += (result > 0.0) ? 4 : 0;
    
    /* Multiple condition combination */
    sum += ((a <= b) != (c >= d)) ? 8 : 0;
    sum += ((e == f) || (g != h)) ? 16 : 0;
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    volatile int iterations = (argc > 1) ? argc : 10;
    int total_sum = 0;
    
    /* Patterned data including NaN for unordered cases */
    float fvals[] = {1.0f, 2.0f, __builtin_nanf(""), 0.0f, -1.0f, __builtin_nanf("")};
    double dvals[] = {3.0, __builtin_nan(""), 0.0, -3.0, 5.0, __builtin_nan("")};
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 0.0f};
    v4sf v2 = {__builtin_nanf(""), 2.0f, 3.0f, 0.0f};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 2.0};
    
    for (int i = 0; i < iterations && i < 6; i++) {
        /* Call each test function with different combinations */
        total_sum += test_unordered_comparisons(
            fvals[i % 6], 
            fvals[(i + 1) % 6],
            dvals[i % 6],
            dvals[(i + 2) % 6]
        );
        
        total_sum += test_uneq_ltgt(
            fvals[i % 6],
            fvals[(i + 2) % 6],
            fvals[(i + 3) % 6],
            fvals[(i + 4) % 6]
        );
        
        total_sum += test_unge_ungt_unle_unlt(
            fvals[i % 6],
            fvals[(i + 1) % 6],
            dvals[i % 6],
            dvals[(i + 1) % 6]
        );
        
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        total_sum += test_mixed_conditional_chain(
            fvals[i % 6],
            fvals[(i + 1) % 6],
            fvals[(i + 2) % 6],
            fvals[(i + 3) % 6],
            dvals[i % 6],
            dvals[(i + 1) % 6],
            dvals[(i + 2) % 6],
            dvals[(i + 3) % 6]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
