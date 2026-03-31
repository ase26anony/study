/* 
 * FP comparison stress test for GCC x86 backend coverage
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_compare fp_compare.c
 * Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_compare_vec fp_compare.c
 * And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_compare_32 fp_compare.c
 */

#include <stdint.h>
#include <stdio.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Volatile variables to prevent constant folding */
static volatile double g_nan = __builtin_nan("");
static volatile double g_inf = __builtin_inf();
static volatile double g_ninf = -__builtin_inf();
static volatile double g_zero = 0.0;
static volatile double g_one = 1.0;
static volatile double g_neg_one = -1.0;

/* Function to stress FP comparisons with complex control flow */
static int stress_fp_comparisons(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Label for goto-based control flow */
    start_comparisons:
    
    /* Exhaustive comparison matrix */
    
    /* UNORDERED cases (involving NaN) */
    if (v1 != v1) {  /* v1 is NaN */
        result |= 1;
        goto unordered_path;
    }
    
    if (v2 != v2) {  /* v2 is NaN */
        result |= 2;
        goto unordered_path;
    }
    
    /* ORDERED cases (no NaN) */
    if (v1 == v3) {
        result |= 4;
    }
    
    /* UNEQ: unordered or equal */
    if (!(v1 < v3) && !(v1 > v3)) {
        result |= 8;
    }
    
    /* UNGE: unordered or greater-or-equal */
    if (!(v1 < v3)) {
        result |= 16;
    }
    
    /* UNGT: unordered or greater */
    if (!(v1 <= v3)) {
        result |= 32;
    }
    
    /* UNLE: unordered or less-or-equal */
    if (!(v1 > v3)) {
        result |= 64;
    }
    
    /* UNLT: unordered or less */
    if (!(v1 >= v3)) {
        result |= 128;
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((v1 < v3) || (v1 > v3)) {
        result |= 256;
    }
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v2) ? (v3 + v4) : 
                     (v1 > v2) ? (v3 - v4) :
                     (v1 == v2) ? (v3 * v4) :
                     (v1 != v1 || v2 != v2) ? g_nan :  /* unordered */
                     (v3 / v4);
    
    result += (int)(cond_val * 1000);
    
    unordered_path:
    
    /* More comparisons with explicit NaN */
    if (g_nan == v1) {  /* Always false, but tests UNORDERED path */
        result |= 512;
    }
    
    if (v1 < g_nan) {   /* Unordered comparison */
        result |= 1024;
    }
    
    if (g_nan != g_nan) {  /* NaN != NaN is true */
        result |= 2048;
    }
    
    /* Conditional move via inline assembly */
    int asm_result;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(asm_result)
        : "x"(v1), "x"(v2)
        : "al", "cc"
    );
    result += asm_result * 4096;
    
    /* Another inline assembly with different condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(asm_result)
        : "x"(v3), "x"(v4)
        : "al", "cc"
    );
    result += asm_result * 8192;
    
    return result;
}

/* Vectorized FP comparison function */
static v2di vector_fp_comparisons(v2df a, v2df b) {
    /* Various vector comparisons that generate different condition codes */
    v2df cmp1 = a < b;    /* LT */
    v2df cmp2 = a <= b;   /* LE */
    v2df cmp3 = a > b;    /* GT */
    v2df cmp4 = a >= b;   /* GE */
    v2df cmp5 = a == b;   /* EQ */
    v2df cmp6 = a != b;   /* NEQ/UNORD */
    
    /* Combine results into integer mask */
    v2di mask1 = (v2di)cmp1;
    v2di mask2 = (v2di)cmp2;
    v2di mask3 = (v2di)cmp3;
    v2di mask4 = (v2di)cmp4;
    v2di mask5 = (v2di)cmp5;
    v2di mask6 = (v2di)cmp6;
    
    return mask1 + mask2 + mask3 + mask4 + mask5 + mask6;
}

/* Main test function with loops and arrays */
static int test_fp_comprehensive(void) {
    double test_values[] = {
        0.0, 1.0, -1.0, 2.5, -2.5,
        __builtin_inf(), -__builtin_inf(),
        __builtin_nan(""), __builtin_nan("0xdead")
    };
    
    int total_result = 0;
    
    /* Test all pairs of values */
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        for (int j = 0; j < sizeof(test_values)/sizeof(test_values[0]); j++) {
            for (int k = 0; k < sizeof(test_values)/sizeof(test_values[0]); k++) {
                for (int l = 0; l < sizeof(test_values)/sizeof(test_values[0]); l++) {
                    total_result ^= stress_fp_comparisons(
                        test_values[i], 
                        test_values[j], 
                        test_values[k], 
                        test_values[l]
                    );
                }
            }
        }
    }
    
    /* Vector tests */
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_inf()};
    
    v2di vec_result = vector_fp_comparisons(vec_a, vec_b);
    total_result += vec_result[0] + vec_result[1];
    
    vec_result = vector_fp_comparisons(vec_a, vec_nan);
    total_result += vec_result[0] + vec_result[1];
    
    vec_result = vector_fp_comparisons(vec_nan, vec_b);
    total_result += vec_result[0] + vec_result[1];
    
    /* Array-based vector operations */
    double arr1[4] = {1.0, 2.0, 3.0, 4.0};
    double arr2[4] = {4.0, 3.0, 2.0, 1.0};
    double arr3[4] = {__builtin_nan(""), 1.0, __builtin_inf(), -__builtin_inf()};
    
    int mask_result = 0;
    for (int i = 0; i < 4; i++) {
        volatile double x = arr1[i];
        volatile double y = arr2[i];
        volatile double z = arr3[i];
        
        /* Complex conditional chain */
        if (x < y) mask_result |= (1 << (i * 3));
        if (x > z) mask_result |= (1 << (i * 3 + 1));
        if (x == y) mask_result |= (1 << (i * 3 + 2));
        if (x != z) mask_result |= (1 << (i * 4));
        if (!(x < z)) mask_result |= (1 << (i * 4 + 1));  /* UNGE */
        if (!(x <= z)) mask_result |= (1 << (i * 4 + 2)); /* UNGT */
        if (!(x > z)) mask_result |= (1 << (i * 4 + 3));  /* UNLE */
        if (!(x >= z)) mask_result |= (1 << (i * 5));     /* UNLT */
    }
    
    total_result ^= mask_result;
    
    /* Final inline assembly with FP condition codes */
    double final_a = test_values[0];
    double final_b = test_values[1];
    double final_c = test_values[2];
    
    int final_asm_result;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "jp 1f\n\t"           /* Jump if unordered (parity) */
        "ja 2f\n\t"           /* Jump if above */
        "jb 3f\n\t"           /* Jump if below */
        "je 4f\n\t"           /* Jump if equal */
        "1:\n\t"
        "movl $1, %0\n\t"     /* UNORDERED */
        "jmp 9f\n\t"
        "2:\n\t"
        "movl $2, %0\n\t"     /* GT (ordered) */
        "jmp 9f\n\t"
        "3:\n\t"
        "movl $3, %0\n\t"     /* LT (ordered) */
        "jmp 9f\n\t"
        "4:\n\t"
        "movl $4, %0\n\t"     /* EQ (ordered) */
        "9:\n\t"
        "ucomisd %3, %4\n\t"  /* Another comparison */
        "setnp %%al\n\t"      /* ORDERED */
        "movzbl %%al, %%eax\n\t"
        "addl %%eax, %0"
        : "=r"(final_asm_result)
        : "x"(final_a), "x"(final_b), "x"(final_c), "x"(g_nan)
        : "eax", "cc"
    );
    
    total_result += final_asm_result;
    
    return total_result;
}

int main(void) {
    printf("Starting FP comparison stress test...\n");
    
    int result = test_fp_comprehensive();
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
