/* Compile with:
   gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c
   gcc -O2 -march=x86-64 -fdump-rtl-final -o test_conds2 test_conds.c
   gcc -O1 -da -fno-trapping-math -o test_conds3 test_conds.c
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int sink;

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* LTGT (unordered not equal) */
    sum += islessgreater(c, d) ? 4 : 0;
    
    /* UNEQ (unordered or equal) - not a standard macro, simulate */
    sum += (!isless(a, b) && !isgreater(a, b)) ? 8 : 0;
    
    /* UNGE (not less than) */
    sum += !isless(a, b) ? 16 : 0;
    
    /* UNGT (not less or equal) */
    sum += !islessequal(a, b) ? 32 : 0;
    
    /* UNLE (unordered or less or equal) */
    sum += (isunordered(a, b) || islessequal(a, b)) ? 64 : 0;
    
    /* UNLT (unordered or less than) */
    sum += (isunordered(a, b) || isless(a, b)) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed comparisons in conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with different comparison types */
    for (int i = 0; i < 4; i++) {
        /* This should generate multiple condition codes */
        if ((a < b) ? (c != d) : (e >= f)) {
            sum += 1;
        }
        
        /* Nested ternary with unordered checks */
        int val = (a == a) ? ((b != b) ? 2 : 3) : ((c < d) ? 4 : 5);
        sum += val;
        
        /* Rotate values to prevent optimization */
        float tmp = a;
        a = b; b = c; c = d; d = e; e = f; f = tmp;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_result = va < vb;
    v4sf cmp_result2 = va == vb;
    v4sf cmp_result3 = va >= vb;
    
    /* Extract results to prevent elimination */
    float results[4];
    memcpy(results, &cmp_result, sizeof(results));
    memcpy(results + 2, &cmp_result2, sizeof(v4sf));
    memcpy(results + 6, &cmp_result3, sizeof(v4sf));
    
    for (int i = 0; i < 12; i++) {
        sum += (results[i] != 0.0f) ? (1 << (i % 8)) : 0;
    }
    
    /* Double vector comparisons */
    v2df dbl_cmp = vc != vd;
    v2df dbl_cmp2 = vc > vd;
    
    double dresults[4];
    memcpy(dresults, &dbl_cmp, sizeof(dbl_cmp));
    memcpy(dresults + 2, &dbl_cmp2, sizeof(dbl_cmp2));
    
    for (int i = 0; i < 4; i++) {
        sum += (dresults[i] != 0.0) ? (i * 7) : 0;
    }
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks - should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;           /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;          /* true if b is NaN */
    
    /* Under fast-math, these might use UNEQ/LTGT */
    sum += (a == c) ? 4 : 0;
    sum += (a != c) ? 8 : 0;
    sum += (b < d) ? 16 : 0;
    sum += (b > d) ? 32 : 0;
    sum += (b <= d) ? 64 : 0;
    sum += (b >= d) ? 128 : 0;
    
    /* Mixed type comparisons */
    sum += ((float)b == a) ? 256 : 0;
    sum += ((double)a != b) ? 512 : 0;
    
    return sum;
}

/* Test function 5: Complex chain of comparisons */
__attribute__((noinline))
static int test_comparison_chain(float a, float b, float c, float d, 
                                 double e, double f, double g, double h) {
    int sum = 0;
    
    /* Chain of comparisons that should generate various condition codes */
    if ((a < b) && (c >= d)) {
        sum += 1;
    }
    
    if ((e != f) || (g == h)) {
        sum += 2;
    }
    
    if (!(a == b) && !(c != d)) {
        sum += 4;
    }
    
    /* This complex expression should generate multiple condition checks */
    float temp = (a < b) ? ((c > d) ? a : b) : ((e < f) ? c : d);
    sum += (int)(temp * 100);
    
    /* Loop with varying comparisons */
    for (int i = 0; i < 3; i++) {
        switch (i % 3) {
            case 0:
                sum += (a <= b) ? 10 : 0;
                break;
            case 1:
                sum += (c >= d) ? 20 : 0;
                break;
            case 2:
                sum += (e != f) ? 30 : 0;
                break;
        }
        
        /* Rotate values */
        float ftmp = a;
        a = b; b = c; c = d; d = ftmp;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with pattern that includes normal numbers and potentially NaN */
    float float_data[16];
    double double_data[16];
    
    /* Fill with patterned data */
    for (int i = 0; i < 16; i++) {
        float_data[i] = (i % 3 == 0) ? (float)i : 
                       (i % 3 == 1) ? (float)(-i) : 
                       (float)(i * 0.5);
        
        double_data[i] = (i % 4 == 0) ? (double)i : 
                        (i % 4 == 1) ? (double)(-i * 2) : 
                        (i % 4 == 2) ? (double)(i * 0.25) : 
                        __builtin_nan("");  /* Some NaN values */
    }
    
    int total_sum = 0;
    
    /* Use argc to prevent loop unrolling from eliminating comparisons */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int iter = 0; iter < iterations; iter++) {
        sink = iter;  /* Volatile to prevent optimization */
        
        /* Test 1: Unordered comparisons */
        int idx = iter % 12;
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx + 1],
            double_data[idx], double_data[idx + 1]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[0], float_data[1], float_data[2],
            float_data[3], float_data[4], float_data[5]
        );
        
        /* Test 3: Vector comparisons */
        v4sf va = {float_data[0], float_data[1], float_data[2], float_data[3]};
        v4sf vb = {float_data[4], float_data[5], float_data[6], float_data[7]};
        v2df vc = {double_data[0], double_data[1]};
        v2df vd = {double_data[2], double_data[3]};
        
        total_sum += test_vector_comparisons(va, vb, vc, vd);
        
        /* Test 4: NaN checks */
        total_sum += test_nan_checks(
            float_data[iter % 8],
            double_data[iter % 8],
            float_data[(iter + 1) % 8],
            double_data[(iter + 1) % 8]
        );
        
        /* Test 5: Comparison chain */
        total_sum += test_comparison_chain(
            float_data[0], float_data[1], float_data[2], float_data[3],
            double_data[4], double_data[5], double_data[6], double_data[7]
        );
        
        /* Rotate data to create varying inputs */
        float ffirst = float_data[0];
        memmove(float_data, float_data + 1, 15 * sizeof(float));
        float_data[15] = ffirst;
    }
    
    printf("Final checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
