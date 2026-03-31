/* Compile with:
   gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test_conds test_conds.c
   gcc -O2 -ffast-math -march=x86-64 -fdump-rtl-expand -o test_conds2 test_conds.c
   gcc -O1 -fno-trapping-math -da -o test_conds3 test_conds.c
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/SSE comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int volatile_counter = 0;

/* Function to generate UNORDERED/ORDERED condition codes */
__attribute__((noinline))
static int test_unordered_ordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Generate UNORDERED condition code */
    if (isunordered(a, b)) {
        sum += 1;
    }
    
    /* Generate ORDERED condition code */
    if (isordered(c, d)) {
        sum += 2;
    }
    
    /* Mixed ordered/unordered checks */
    if (!isunordered(a, b) && isunordered(c, d)) {
        sum += 4;
    }
    
    return sum;
}

/* Function to generate UNEQ condition code */
__attribute__((noinline))
static int test_uneq(float a, float b, double c, double d) {
    int sum = 0;
    
    /* With -ffast-math, == can generate UNEQ */
    if (a == b) {
        sum += 1;
    }
    
    /* Direct unordered equal check */
    if (!(c != d)) {  /* This can generate UNEQ under fast-math */
        sum += 2;
    }
    
    /* Complex expression that might generate UNEQ */
    if ((a < b) ? (c == d) : (a != a)) {
        sum += 4;
    }
    
    return sum;
}

/* Function to generate UNGE/UNGT/UNLE/UNLT condition codes */
__attribute__((noinline))
static int test_unge_ungt_unle_unlt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These comparisons can generate unordered versions with fast-math */
    if (a >= b) {  /* May generate UNGE (nlt) */
        sum += 1;
    }
    
    if (b > c) {   /* May generate UNGT (nle) */
        sum += 2;
    }
    
    if (c <= d) {  /* May generate UNLE (ule) */
        sum += 4;
    }
    
    if (d < a) {   /* May generate UNLT (ult) */
        sum += 8;
    }
    
    /* Mixed comparisons in conditional */
    if ((a >= b) ? (c <= d) : (b > c)) {
        sum += 16;
    }
    
    return sum;
}

/* Function to generate LTGT (une) condition code */
__attribute__((noinline))
static int test_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* islessgreater generates LTGT directly */
    if (islessgreater(a, b)) {
        sum += 1;
    }
    
    /* != can generate LTGT with fast-math */
    if (c != d) {
        sum += 2;
    }
    
    /* Complex expression */
    if ((a != b) && !(c == d)) {
        sum += 4;
    }
    
    return sum;
}

/* Function using vector extensions to generate condition codes */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate multiple condition codes */
    v4sf cmp1 = va < vb;   /* May generate UNLT in vector context */
    v4sf cmp2 = vc == vd;  /* May generate UNEQ in vector context */
    
    /* Extract results to prevent elimination */
    float f1 = cmp1[0] + cmp1[1] + cmp1[2] + cmp1[3];
    double d1 = cmp2[0] + cmp2[1];
    
    if (f1 != 0.0f) sum += 1;
    if (d1 != 0.0) sum += 2;
    
    /* More vector ops */
    v4sf cmp3 = va >= vb;  /* May generate UNGE */
    v4sf cmp4 = va != vb;  /* May generate LTGT */
    
    float f2 = cmp3[0] + cmp3[1] + cmp3[2] + cmp3[3];
    float f3 = cmp4[0] + cmp4[1] + cmp4[2] + cmp4[3];
    
    if (f2 != 0.0f) sum += 4;
    if (f3 != 0.0f) sum += 8;
    
    return sum;
}

/* Complex function mixing all comparison types */
__attribute__((noinline))
static int test_mixed_comparisons(float fa1, float fa2, double db1, double db2,
                                  float fb1, float fb2, double da1, double da2) {
    int sum = 0;
    
    /* Chain of different comparisons */
    if (isunordered(fa1, fa2)) {
        sum += 1;
        if (db1 != db2) {  /* LTGT */
            sum += 2;
        }
    } else if (isordered(db1, db2)) {
        sum += 4;
        if (fb1 >= fb2) {  /* UNGE */
            sum += 8;
        }
    }
    
    /* Ternary with mixed comparisons */
    sum += (fa1 < fa2) ? 
           ((db1 == db2) ? 16 : 32) :  /* UNEQ vs LTGT */
           ((fb1 > fb2) ? 64 : 128);   /* UNGT vs something else */
    
    /* Nested conditionals */
    if (da1 != da2) {  /* LTGT */
        if (!(da1 == da2)) {  /* UNEQ negated */
            sum += 256;
        }
        if (islessgreater(da1, da2)) {  /* Explicit LTGT */
            sum += 512;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float fdata[16];
    double ddata[16];
    
    /* Initialize with pattern: normals, zeros, NaN */
    for (int i = 0; i < 16; i++) {
        fdata[i] = (i % 4 == 0) ? __builtin_nanf("") : 
                   (i % 4 == 1) ? 0.0f : 
                   (i % 4 == 2) ? (float)i : 
                   -(float)i;
        
        ddata[i] = (i % 5 == 0) ? __builtin_nan("") : 
                   (i % 5 == 1) ? 0.0 : 
                   (i % 5 == 2) ? (double)i : 
                   (i % 5 == 3) ? 1.0/(double)(i+1) : 
                   -(double)i;
    }
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 4 : 8;
    
    for (int iter = 0; iter < iterations; iter++) {
        volatile_counter = iter;
        
        /* Cycle through different indices to use different values */
        int idx = iter % 12;
        
        /* Test unordered/ordered */
        total_sum += test_unordered_ordered(
            fdata[idx], fdata[idx+1], 
            ddata[idx], ddata[idx+2]
        );
        
        /* Test UNEQ */
        total_sum += test_uneq(
            fdata[idx+1], fdata[idx+2],
            ddata[idx+1], ddata[idx+3]
        );
        
        /* Test UNGE/UNGT/UNLE/UNLT */
        total_sum += test_unge_ungt_unle_unlt(
            fdata[idx], fdata[idx+2],
            fdata[idx+1], fdata[idx+3]
        );
        
        /* Test LTGT */
        total_sum += test_ltgt(
            ddata[idx], ddata[idx+1],
            ddata[idx+2], ddata[idx+3]
        );
        
        /* Test vector comparisons */
        v4sf va = {fdata[idx], fdata[idx+1], fdata[idx+2], fdata[idx+3]};
        v4sf vb = {fdata[idx+4], fdata[idx+5], fdata[idx+6], fdata[idx+7]};
        v2df vc = {ddata[idx], ddata[idx+1]};
        v2df vd = {ddata[idx+2], ddata[idx+3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test mixed comparisons */
        total_sum += test_mixed_comparisons(
            fdata[idx], fdata[idx+1], ddata[idx], ddata[idx+1],
            fdata[idx+2], fdata[idx+3], ddata[idx+2], ddata[idx+3]
        );
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
