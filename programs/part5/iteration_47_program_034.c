/* i386_condition_codes.c
 * Program to trigger x86 condition code mnemonics for floating-point comparisons
 * Compile with: gcc -O3 -ffast-math -march=x86-64 -mavx2 -fdump-rtl-final -o test i386_condition_codes.c -lm
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Prevent optimization */
static volatile int global_counter = 0;

/* Function to generate UNORDERED/ORDERED condition codes */
__attribute__((noinline))
static int test_unordered_ordered(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Generate UNORDERED (unord) */
    if (isunordered(a, b)) {
        sum += 1;
    }
    
    /* Generate ORDERED (ord) */
    if (!isunordered(a, b)) {  /* Equivalent to isordered() */
        sum += 2;
    }
    
    /* Mixed ordered/unordered comparisons */
    if (isunordered(c, d) || (a == b)) {
        sum += 4;
    }
    
    /* Check for NaN using a != a */
    if (a != a) {  /* Always true if a is NaN */
        sum += 8;
    }
    
    if (!(c == c)) {  /* Another NaN check */
        sum += 16;
    }
    
    return sum;
}

/* Function to generate UNEQ, UNGE, UNGT, UNLE, UNLT condition codes */
__attribute__((noinline))
static int test_uneq_unge_ungt(float a, float b, float c, float d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ, UNGE, etc. */
    
    /* UNEQ (ueq) - unordered or equal */
    if (!(a != b)) {  /* Equivalent to a == b with fast-math */
        sum += 1;
    }
    
    /* UNGE (nlt) - unordered or greater than or equal */
    if (!(a < b)) {  /* With fast-math, becomes !(a < b) -> a >= b or unordered */
        sum += 2;
    }
    
    /* UNGT (nle) - unordered or greater than */
    if (!(a <= b)) {  /* !(a <= b) -> a > b or unordered */
        sum += 4;
    }
    
    /* UNLE (ule) - unordered or less than or equal */
    if (!(a > b)) {  /* !(a > b) -> a <= b or unordered */
        sum += 8;
    }
    
    /* UNLT (ult) - unordered or less than */
    if (!(a >= b)) {  /* !(a >= b) -> a < b or unordered */
        sum += 16;
    }
    
    /* Complex conditional to force multiple condition codes */
    if ((a < b) ? (c != d) : (c >= d)) {
        sum += 32;
    }
    
    return sum;
}

/* Function to generate LTGT (une) condition code */
__attribute__((noinline))
static int test_ltgt(double a, double b, double c, double d) {
    int sum = 0;
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    /* islessgreater() macro should generate LTGT */
    if (islessgreater(a, b)) {
        sum += 1;
    }
    
    /* Alternative using direct comparisons */
    if ((a < b) || (a > b)) {  /* With fast-math, may generate LTGT */
        sum += 2;
    }
    
    /* Mixed comparisons */
    if ((a != b) && !isunordered(a, b)) {  /* Ordered and not equal */
        sum += 4;
    }
    
    return sum;
}

/* Function using vector extensions to generate vector comparisons */
__attribute__((noinline))
static int test_vector_comparisons(v4sf a, v4sf b) {
    int sum = 0;
    
    /* Vector comparisons generate condition codes for each element */
    v4sf cmp_result;
    
    /* Unordered comparison */
    for (int i = 0; i < 4; i++) {
        if (isunordered(a[i], b[i])) {
            sum += (1 << i);
        }
    }
    
    /* Generate mask from vector comparison */
    cmp_result = a < b;  /* May generate UNLT or similar in vector form */
    
    /* Check elements */
    for (int i = 0; i < 4; i++) {
        if (cmp_result[i] != 0.0f) {
            sum += (1 << (i + 4));
        }
    }
    
    /* Ordered greater than or equal */
    cmp_result = a >= b;  /* May generate UNGE */
    for (int i = 0; i < 4; i++) {
        if (cmp_result[i] != 0.0f) {
            sum += (1 << (i + 8));
        }
    }
    
    return sum;
}

/* Function with mixed float/double comparisons */
__attribute__((noinline))
static int test_mixed_comparisons(float f1, float f2, double d1, double d2) {
    int sum = 0;
    
    /* Chain of comparisons */
    if (f1 < f2) {
        if (d1 != d2) {
            sum += 1;
        } else if (isunordered(d1, d2)) {
            sum += 2;
        }
    } else if (f1 > f2) {
        if (d1 <= d2) {
            sum += 4;
        }
    } else { /* f1 == f2 */
        if (!islessgreater(d1, d2)) {
            sum += 8;
        }
    }
    
    /* Ternary with different comparison types */
    sum += ((f1 == f2) ? (d1 < d2) : (f1 > f2)) ? 16 : 0;
    
    /* Nested ternary */
    sum += (f1 != f2) ? ((d1 >= d2) ? 32 : 64) : ((f1 < f2) ? 128 : 256);
    
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), 5.0f,
        -1.0f, -2.0f, __builtin_nanf("123"), 6.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, 3.0, __builtin_nan("456"),
        -1.0, -2.0, 5.0, 6.0
    };
    
    /* Vector data */
    v4sf vec1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec2 = {4.0f, 2.0f, 3.0f, __builtin_nanf("")};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 8) + 1 : 4;
    
    for (int i = 0; i < iterations; i++) {
        global_counter = i;  /* Volatile to prevent optimization */
        
        /* Test different comparison patterns */
        int idx1 = i % 12;
        int idx2 = (i + 1) % 12;
        int idx3 = (i + 2) % 12;
        int idx4 = (i + 3) % 12;
        
        /* Test 1: Unordered/Ordered condition codes */
        total_sum += test_unordered_ordered(
            float_data[idx1], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 2: UNEQ, UNGE, UNGT, UNLE, UNLT */
        total_sum += test_uneq_unge_ungt(
            float_data[idx1], float_data[idx2],
            float_data[idx3], float_data[idx4]
        );
        
        /* Test 3: LTGT condition code */
        total_sum += test_ltgt(
            double_data[idx1], double_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 4: Vector comparisons */
        total_sum += test_vector_comparisons(vec1, vec2);
        
        /* Test 5: Mixed comparisons */
        total_sum += test_mixed_comparisons(
            float_data[idx1], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Modify data slightly each iteration */
        float_data[idx1] += 0.1f;
        double_data[idx2] -= 0.1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    printf("Global counter: %d\n", global_counter);
    
    return total_sum != 0 ? 0 : 1;
}
