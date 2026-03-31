/* Compile with:
   gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_fp_conds test_fp_conds.c
   Additional flags for debugging: -da -fno-trapping-math
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Vector types for AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile int g_volatile = 0;

/* Test 1: Direct unordered comparisons using math.h macros */
NOINLINE static int test_unordered_comparisons(float a, float b, double c, double d)
{
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* LTGT comparison */
    sum += islessgreater(c, d) ? 4 : 0;
    
    /* UNEQ comparison */
    sum += !islessgreater(c, d) && !isunordered(c, d) ? 8 : 0;
    
    /* UNGE/UNGT/UNLE/UNLT through combinations */
    sum += !isless(a, b) || isunordered(a, b) ? 16 : 0;  /* UNGE: nlt */
    sum += !islessequal(a, b) || isunordered(a, b) ? 32 : 0; /* UNGT: nle */
    sum += islessequal(a, b) || isunordered(a, b) ? 64 : 0;  /* UNLE: ule */
    sum += isless(a, b) || isunordered(a, b) ? 128 : 0;      /* UNLT: ult */
    
    return sum;
}

/* Test 2: Mixed comparison types in complex conditional expressions */
NOINLINE static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f)
{
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        /* This should generate multiple condition codes */
        sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
        
        /* Another mixed expression */
        sum += ((a == b) ? (c > d) : (e <= f)) ? 2 : 0;
        
        /* Chain of comparisons */
        sum += (a < b && c > d) || (e == f) ? 4 : 0;
        
        /* XOR of comparisons */
        sum += ((a < b) != (c > d)) ? 8 : 0;
    }
    
    return sum;
}

/* Test 3: Vectorized floating-point comparisons */
NOINLINE static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd)
{
    int sum = 0;
    
    /* Element-wise vector comparisons - these often use UNORDERED variants */
    v4sf mask1 = va < vb;      /* May generate UNLT with fast-math */
    v4sf mask2 = va == vb;     /* May generate UNEQ with fast-math */
    v4sf mask3 = va != vb;     /* May generate LTGT with fast-math */
    v4sf mask4 = va >= vb;     /* May generate UNGE with fast-math */
    
    /* Extract results to prevent elimination */
    float m1[4], m2[4], m3[4], m4[4];
    memcpy(m1, &mask1, sizeof(m1));
    memcpy(m2, &mask2, sizeof(m2));
    memcpy(m3, &mask3, sizeof(m3));
    memcpy(m4, &mask4, sizeof(m4));
    
    for (int i = 0; i < 4; i++) {
        sum += (m1[i] != 0.0f) ? (1 << i) : 0;
        sum += (m2[i] != 0.0f) ? (1 << (i + 4)) : 0;
        sum += (m3[i] != 0.0f) ? (1 << (i + 8)) : 0;
        sum += (m4[i] != 0.0f) ? (1 << (i + 12)) : 0;
    }
    
    /* Double vector comparisons */
    v2df dmask1 = vc < vd;
    v2df dmask2 = vc != vd;
    
    double dm1[2], dm2[2];
    memcpy(dm1, &dmask1, sizeof(dm1));
    memcpy(dm2, &dmask2, sizeof(dm2));
    
    sum += (dm1[0] != 0.0) ? 65536 : 0;
    sum += (dm1[1] != 0.0) ? 131072 : 0;
    sum += (dm2[0] != 0.0) ? 262144 : 0;
    sum += (dm2[1] != 0.0) ? 524288 : 0;
    
    return sum;
}

/* Test 4: NaN checks and unordered semantics */
NOINLINE static int test_nan_handling(float a, double b, float c, double d)
{
    int sum = 0;
    
    /* Explicit NaN checks using a != a idiom */
    sum += (a != a) ? 1 : 0;           /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;          /* true if b is NaN */
    
    /* Mixed ordered/unordered comparisons */
    sum += (a < c) || (a != a) || (c != c) ? 4 : 0;  /* UNLT: ult */
    sum += (b > d) || (b != b) || (d != d) ? 8 : 0;  /* UNGT through transformation */
    
    /* Complex condition with potential NaN */
    sum += ((a == c) && (b == d)) || (a != a) || (b != b) ? 16 : 0;
    
    return sum;
}

/* Test 5: Function with multiple FP parameters and comparison chain */
NOINLINE static int test_comparison_chain(float a, float b, float c, float d, 
                                          float e, float f, float g, float h)
{
    int result = 0;
    
    /* Chain of comparisons that might generate various condition codes */
    if (a < b) result |= 1;      /* May become UNLT with fast-math */
    if (c > d) result |= 2;      /* May become UNGT with fast-math */
    if (e == f) result |= 4;     /* May become UNEQ with fast-math */
    if (g != h) result |= 8;     /* May become LTGT with fast-math */
    
    /* Additional unordered checks */
    if (a >= b) result |= 16;    /* May become UNGE with fast-math */
    if (c <= d) result |= 32;    /* May become UNLE with fast-math */
    
    /* Combined condition */
    if ((a < b) == (c > d)) result |= 64;
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv)
{
    int total_sum = 0;
    
    /* Patterned data including normal numbers, zeros, and NaN */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 3 == 0) ? i * 1.5f : 
                   (i % 3 == 1) ? -i * 0.5f : 0.0f;
        ddata[i] = (i % 4 == 0) ? i * 2.5 : 
                   (i % 4 == 1) ? -i * 1.5 : 
                   (i % 4 == 2) ? __builtin_nan("") : 0.0;
    }
    
    /* Introduce some NaN values in float array (if not using -ffast-math) */
    fdata[5] = __builtin_nanf("");
    fdata[11] = __builtin_nanf("");
    
    /* Use argc to prevent loop unrolling from collapsing comparisons */
    int iterations = (argc > 1) ? argc : 4;
    iterations = (iterations > 100) ? 100 : iterations; /* Limit iterations */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs slightly each iteration */
        int idx = iter % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            fdata[idx], fdata[idx + 1], 
            ddata[idx], ddata[idx + 1]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            fdata[idx], fdata[idx + 2],
            fdata[idx + 1], fdata[idx + 3],
            fdata[idx + 4], fdata[idx + 5]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {fdata[idx], fdata[idx + 1], fdata[idx + 2], fdata[idx + 3]};
        v4sf vb = {fdata[idx + 4], fdata[idx + 5], fdata[idx + 6], fdata[idx + 7]};
        v2df vc = {ddata[idx], ddata[idx + 1]};
        v2df vd = {ddata[idx + 2], ddata[idx + 3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: NaN handling */
        total_sum += test_nan_handling(
            fdata[idx], ddata[idx],
            fdata[idx + 1], ddata[idx + 1]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            fdata[idx], fdata[idx + 1],
            fdata[idx + 2], fdata[idx + 3],
            fdata[idx + 4], fdata[idx + 5],
            fdata[idx + 6], fdata[idx + 7]
        );
        
        /* Mix in volatile to prevent optimization */
        total_sum += g_volatile;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
