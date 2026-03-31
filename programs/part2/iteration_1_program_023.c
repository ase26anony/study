/* Compile with various options to cover different code generation paths:
   -O2 -march=x86-64 -mtune=generic -ffp-contract=off
   -O3 -msse4.2 -ftree-vectorize -fno-trapping-math
   -O1 -m32 -mfpmath=387 -fno-inline
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile variables to prevent constant folding */
volatile double g_nan = __builtin_nan("");
volatile double g_inf = __builtin_inf();
volatile double g_ninf = -__builtin_inf();
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg = -1.0;

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Matrix of comparisons to cover all condition codes */
    
    /* UNORDERED cases (comparisons involving NaN) */
    if (v1 != v1) { /* NaN != NaN is always true */
        result ^= 1;
    }
    
    if (v1 == g_nan) { /* Comparison with NaN */
        result ^= 2;
    }
    
    if (g_nan < v2) { /* NaN < anything */
        result ^= 4;
    }
    
    if (v2 > g_nan) { /* anything > NaN */
        result ^= 8;
    }
    
    /* ORDERED cases (normal comparisons) */
    if (v1 == v2) {
        result ^= 16;
    }
    
    if (v1 != v2) {
        result ^= 32;
    }
    
    if (v1 < v2) {
        result ^= 64;
    }
    
    if (v1 <= v2) {
        result ^= 128;
    }
    
    if (v1 > v2) {
        result ^= 256;
    }
    
    if (v1 >= v2) {
        result ^= 512;
    }
    
    /* Complex conditions to trigger UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
    
    /* UNEQ: unordered or equal */
    if (!(v3 < v4) && !(v3 > v4)) {
        result ^= 1024;
    }
    
    /* UNGE: unordered or greater or equal */
    if (!(v3 < v4)) {
        result ^= 2048;
    }
    
    /* UNGT: unordered or greater */
    if (!(v3 <= v4)) {
        result ^= 4096;
    }
    
    /* UNLE: unordered or less or equal */
    if (!(v3 > v4)) {
        result ^= 8192;
    }
    
    /* UNLT: unordered or less */
    if (!(v3 >= v4)) {
        result ^= 16384;
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((v3 < v4) || (v3 > v4)) {
        result ^= 32768;
    }
    
    /* Conditional moves using FP comparison results */
    double cmov_result = (v1 < v2) ? v3 : v4;
    result ^= (int)(cmov_result * 1000);
    
    cmov_result = (v1 != v1) ? v3 : v4; /* Check for NaN */
    result ^= (int)(cmov_result * 1000);
    
    /* Goto-based complex control flow */
    if (v1 < v2) goto label1;
    if (v1 == v2) goto label2;
    if (v1 > v2) goto label3;
    
    /* Unreachable, but creates control flow edges */
    result ^= 65536;
    
label1:
    result ^= 131072;
    goto end_labels;
    
label2:
    result ^= 262144;
    goto end_labels;
    
label3:
    result ^= 524288;
    
end_labels:
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    v2di mask;
    int result = 0;
    
    /* Various vector comparisons that generate cmppd/ucomisd with condition codes */
    
    /* Equality comparison */
    mask = (v2di)(vec1 == vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Inequality comparison */
    mask = (v2di)(vec1 != vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Less than comparison */
    mask = (v2di)(vec1 < vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Less or equal comparison */
    mask = (v2di)(vec1 <= vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Greater than comparison */
    mask = (v2di)(vec1 > vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Greater or equal comparison */
    mask = (v2di)(vec1 >= vec2);
    result ^= mask[0] ^ mask[1];
    
    /* Comparisons with NaN (unordered) */
    mask = (v2di)(vec1 == vec_nan);
    result ^= mask[0] ^ mask[1];
    
    mask = (v2di)(vec_nan < vec1);
    result ^= mask[0] ^ mask[1];
    
    /* Comparisons with infinity */
    mask = (v2di)(vec1 < vec_inf);
    result ^= mask[0] ^ mask[1];
    
    /* Float vector comparisons (different instruction generation) */
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf fvec_nan = {__builtin_nanf(""), __builtin_nanf(""), 
                     __builtin_nanf(""), __builtin_nanf("")};
    
    v4si fmask;
    
    fmask = (v4si)(fvec1 == fvec2);
    result ^= fmask[0] ^ fmask[1] ^ fmask[2] ^ fmask[3];
    
    fmask = (v4si)(fvec1 != fvec2);
    result ^= fmask[0] ^ fmask[1] ^ fmask[2] ^ fmask[3];
    
    fmask = (v4si)(fvec1 < fvec2);
    result ^= fmask[0] ^ fmask[1] ^ fmask[2] ^ fmask[3];
    
    fmask = (v4si)(fvec_nan == fvec1);
    result ^= fmask[0] ^ fmask[1] ^ fmask[2] ^ fmask[3];
    
    return result;
}

/* Function with inline assembly using FP condition codes */
static int asm_fp_comparisons(double x, double y) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* Check for unordered (NaN) - sets parity flag */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* Check for equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* Check for less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* Check for less or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* Check for greater than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* Check for greater or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* Conditional move based on FP comparison */
    double cmov_dest;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r"(cmov_dest)
        : "x"(x), "x"(y), "r"(x)
        : "cc"
    );
    result ^= (int)(cmov_dest * 100);
    
    return result;
}

/* Loop-based comparisons to prevent optimization */
static int loop_fp_comparisons(double *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        volatile double a = arr[i];
        volatile double b = arr[i + 1];
        
        /* Exhaustive comparison matrix in loop */
        if (a == b) result ^= 1;
        if (a != b) result ^= 2;
        if (a < b) result ^= 4;
        if (a <= b) result ^= 8;
        if (a > b) result ^= 16;
        if (a >= b) result ^= 32;
        
        /* Unordered checks */
        if (a != a) result ^= 64;  /* a is NaN */
        if (b != b) result ^= 128; /* b is NaN */
        
        /* Complex conditions */
        if (!(a < b) && !(a > b)) result ^= 256;  /* UNEQ */
        if (!(a < b)) result ^= 512;              /* UNGE */
        if (!(a <= b)) result ^= 1024;            /* UNGT */
        if (!(a > b)) result ^= 2048;             /* UNLE */
        if (!(a >= b)) result ^= 4096;            /* UNLT */
        if ((a < b) || (a > b)) result ^= 8192;   /* LTGT */
    }
    
    return result;
}

int main(void) {
    int final_result = 0;
    
    /* Initialize test values */
    double normal_vals[] = {0.0, 1.0, -1.0, 2.5, -2.5, 100.0, -100.0};
    double special_vals[] = {
        __builtin_inf(),
        -__builtin_inf(),
        __builtin_nan(""),
        0.0/0.0,  /* Another way to get NaN */
    };
    
    /* Test 1: Stress FP comparisons with various value combinations */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            final_result ^= stress_fp_comparisons(
                normal_vals[i], 
                normal_vals[j],
                special_vals[i % 3],
                special_vals[j % 3]
            );
        }
    }
    
    /* Test 2: Vectorized comparisons */
    final_result ^= vector_fp_comparisons();
    
    /* Test 3: Inline assembly comparisons */
    for (int i = 0; i < 4; i++) {
        final_result ^= asm_fp_comparisons(
            normal_vals[i], 
            special_vals[i % 3]
        );
    }
    
    /* Test 4: Loop-based comparisons */
    double test_array[20];
    for (int i = 0; i < 20; i++) {
        test_array[i] = (i % 2 == 0) ? normal_vals[i % 7] : special_vals[i % 4];
    }
    final_result ^= loop_fp_comparisons(test_array, 20);
    
    /* Additional explicit tests for edge cases */
    
    /* Direct NaN comparisons */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    
    if (nan1 == nan1) final_result ^= 1;  /* Always false */
    if (nan1 != nan1) final_result ^= 2;  /* Always true */
    if (nan1 < 0.0) final_result ^= 4;    /* Always false */
    if (nan1 > 0.0) final_result ^= 8;    /* Always false */
    if (0.0 < nan1) final_result ^= 16;   /* Always false */
    if (0.0 > nan1) final_result ^= 32;   /* Always false */
    
    /* Infinity comparisons */
    volatile double inf = __builtin_inf();
    volatile double ninf = -__builtin_inf();
    
    if (inf > 0.0) final_result ^= 64;
    if (ninf < 0.0) final_result ^= 128;
    if (inf == inf) final_result ^= 256;
    if (inf > ninf) final_result ^= 512;
    
    /* Mixed NaN and infinity */
    if (nan1 < inf) final_result ^= 1024;
    if (nan1 > ninf) final_result ^= 2048;
    if (inf == nan1) final_result ^= 4096;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
