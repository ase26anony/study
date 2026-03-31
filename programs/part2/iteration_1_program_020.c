/* 
 * FP comparison stress test for GCC x86 backend coverage
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -o fp_compare fp_compare.c
 * Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math
 * And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile double global_nan = __builtin_nan("");
volatile double global_inf = __builtin_inf();
volatile double global_neg_inf = -__builtin_inf();
volatile double global_zero = 0.0;
volatile double global_one = 1.0;
volatile double global_two = 2.0;
volatile double global_neg_one = -1.0;

/* Function with complex control flow using goto */
static int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    int temp;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* UNORDERED cases (involving NaN) */
    if (v1 != v1) {  /* NaN != NaN is true */
        result |= 1;
        goto unordered_block;
    }
    
    if (v1 == global_nan) {  /* x == NaN is false, but generates unordered check */
        /* This block should not be taken */
        result |= 2;
    }
    
unordered_block:
    if (global_nan < v2) {  /* NaN < x is false (unordered) */
        /* Not taken */
        result |= 4;
    }
    
    /* ORDERED cases (normal numbers) */
    if (v2 < v3) {  /* Regular less-than */
        result |= 8;
        goto ordered_block;
    }
    
ordered_block:
    if (v3 >= v4) {  /* Regular greater-or-equal */
        result |= 16;
    }
    
    /* UNEQ (unordered or equal) */
    double uniq_test = global_nan;
    if (!(uniq_test > v1) && !(uniq_test < v1)) {  /* NaN == x? Actually unordered */
        result |= 32;
    }
    
    /* UNGE (unordered or greater-or-equal) */
    if (!(v1 < v2)) {  /* Not less-than includes unordered and >= */
        result |= 64;
    }
    
    /* UNGT (unordered or greater-than) */
    if (!(v2 <= v3)) {  /* Not less-or-equal */
        result |= 128;
    }
    
    /* UNLE (unordered or less-or-equal) */
    if (!(v3 > v4)) {  /* Not greater-than */
        result |= 256;
    }
    
    /* UNLT (unordered or less-than) */
    if (!(v1 >= v2)) {  /* Not greater-or-equal */
        result |= 512;
    }
    
    /* LTGT (less-than or greater-than, but not equal and not unordered) */
    if (v1 != v2 && v1 == v1 && v2 == v2) {  /* Both ordered and not equal */
        result |= 1024;
    }
    
    /* Conditional moves using ternary operator */
    double cmov_result = (v1 < v2) ? v3 : v4;
    result += (int)(cmov_result * 100);
    
    cmov_result = (global_nan == v1) ? v2 : v3;  /* Unordered case */
    result += (int)(cmov_result * 100);
    
    /* Complex nested conditionals */
    if (v1 < v2) {
        if (v3 > v4) {
            result += 2048;
        } else if (v3 == v4) {
            result += 4096;
        }
    } else if (v1 == v2) {
        result += 8192;
    } else {
        if (v3 != v4) {
            result += 16384;
        }
    }
    
    return result;
}

/* Function with vectorized comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b) {
    /* Perform various vector comparisons */
    v2df vec_nan = {global_nan, global_nan};
    v2df vec_inf = {global_inf, global_inf};
    
    /* These generate cmppd/ucomisd with various condition codes */
    v2di cmp1 = (v2di)(a < b);      /* LT */
    v2di cmp2 = (v2di)(a <= b);     /* LE */
    v2di cmp3 = (v2di)(a > b);      /* GT */
    v2di cmp4 = (v2di)(a >= b);     /* GE */
    v2di cmp5 = (v2di)(a == b);     /* EQ */
    v2di cmp6 = (v2di)(a != b);     /* NEQ */
    
    /* Unordered comparisons */
    v2di cmp7 = (v2di)(a == vec_nan);  /* Unordered check */
    v2di cmp8 = (v2di)(vec_nan < b);   /* Another unordered */
    
    /* Combine results */
    return cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6 + cmp7 + cmp8;
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b) {
    int result = 0;
    uint8_t byte_result;
    
    /* Test UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 0);
    
    /* Test ORDERED (not parity) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 1);
    
    /* Test LESS-THAN (CF=1) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 2);
    
    /* Test GREATER-THAN (ZF=0 && CF=0) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 3);
    
    /* Test EQUAL (ZF=1) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 4);
    
    /* Test UNORDERED OR LESS-THAN (PF=1 || CF=1) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 5);
    
    return result;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Test values */
    double normal_values[] = {0.0, 1.0, -1.0, 2.5, -3.75};
    double special_values[] = {global_nan, global_inf, global_neg_inf};
    
    /* Test scalar comparisons */
    printf("Testing scalar FP comparisons...\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            checksum += fp_comparison_stress(
                normal_values[i],
                normal_values[j],
                normal_values[(i+1)%5],
                normal_values[(j+2)%5]
            );
        }
    }
    
    /* Test with NaN and Inf */
    printf("Testing with NaN/Inf...\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            checksum += fp_comparison_stress(
                normal_values[i],
                special_values[j],
                normal_values[(i+2)%5],
                special_values[(j+1)%3]
            );
        }
    }
    
    /* Test vectorized comparisons */
    printf("Testing vectorized comparisons...\n");
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {global_nan, global_nan};
    v2df vec_inf = {global_inf, global_neg_inf};
    
    v2di vec_result;
    
    vec_result = vector_fp_comparisons(vec1, vec2);
    checksum += vec_result[0] + vec_result[1];
    
    vec_result = vector_fp_comparisons(vec1, vec_nan);
    checksum += vec_result[0] + vec_result[1];
    
    vec_result = vector_fp_comparisons(vec_nan, vec_inf);
    checksum += vec_result[0] + vec_result[1];
    
    /* Test inline assembly condition codes */
    printf("Testing inline assembly condition codes...\n");
    checksum += asm_fp_conditions(1.0, 2.0);      /* 1.0 < 2.0 */
    checksum += asm_fp_conditions(2.0, 1.0);      /* 2.0 > 1.0 */
    checksum += asm_fp_conditions(1.0, 1.0);      /* 1.0 == 1.0 */
    checksum += asm_fp_conditions(global_nan, 1.0); /* NaN unordered */
    checksum += asm_fp_conditions(1.0, global_nan); /* unordered */
    checksum += asm_fp_conditions(global_nan, global_nan); /* NaN == NaN unordered */
    
    /* Additional unordered pattern with volatile */
    {
        volatile double vnan = __builtin_nan("");
        volatile double vinf = __builtin_inf();
        volatile double vnum = 42.0;
        
        /* Complex conditional expression forcing unordered checks */
        double complex_result = 
            (vnan < vnum) ? 1.0 :
            (vnum > vnan) ? 2.0 :
            (vinf == vnan) ? 3.0 :
            (vnan != vnan) ? 4.0 : 5.0;
        
        checksum += (int)complex_result;
        
        /* Loop with FP comparisons */
        for (volatile int k = 0; k < 10; k++) {
            volatile double loop_var = k * 0.5;
            if (loop_var < vnan) {
                checksum += k * 10;
            } else if (loop_var > vinf) {
                checksum += k * 20;
            } else if (loop_var != loop_var) {
                checksum += k * 30;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
