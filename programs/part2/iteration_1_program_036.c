/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double vzero = 0.0;
static volatile double vone = 1.0;
static volatile double vneg = -1.0;
static volatile double vinf = __builtin_inf();
static volatile double vninf = -__builtin_inf();
static volatile double vnan = __builtin_nan("");
static volatile double vsnan = __builtin_nans("");

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
__attribute__((noinline))
static int stress_fp_comparisons(double a, double b, double c, double d) {
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    volatile double w = d;
    
    int result = 0;
    
    /* Complex control flow with goto to prevent simplification */
    if (x == y) goto label_eq;
    if (x != y) goto label_neq;
    
label_eq:
    /* Ordered equality comparisons */
    if (x == y) result ^= 1;
    if (z == w) result ^= 2;
    
    /* Unordered equality (UNEQ) - should trigger when either is NaN */
    if (!(x != x) && !(y != y) && x == y) result ^= 4;
    
    /* Jump to avoid dead code */
    goto label_ordered;
    
label_neq:
    /* Inequality comparisons */
    if (x != y) result ^= 8;
    if (z != w) result ^= 16;
    
    /* LTGT condition (ordered and not equal) */
    if (x < y || x > y) result ^= 32;
    
    goto label_lt;

label_ordered:
    /* ORDERED condition (neither is NaN) */
    if (x == x && y == y) result ^= 64;
    
    /* UNORDERED condition (either is NaN) */
    if (x != x || y != y) result ^= 128;
    
    /* More comparisons to generate different condition codes */
    if (x < y) result ^= 256;
    if (x <= y) result ^= 512;
    if (x > y) result ^= 1024;
    if (x >= y) result ^= 2048;

label_lt:
    /* UNLT (unordered or less than) */
    if (x != x || y != y || x < y) result ^= 4096;
    
    /* UNLE (unordered or less than or equal) */
    if (x != x || y != y || x <= y) result ^= 8192;
    
    /* UNGT (unordered or greater than) */
    if (x != x || y != y || x > y) result ^= 16384;
    
    /* UNGE (unordered or greater than or equal) */
    if (x != x || y != y || x >= y) result ^= 32768;
    
    return result;
}

/* Function with vectorized FP comparisons */
__attribute__((noinline))
static v2di vector_fp_comparisons(v2df va, v2df vb) {
    v2di mask = {0, 0};
    
    /* Generate all vector comparison conditions */
    mask += (v2di)(va == vb);  /* EQ */
    mask += (v2di)(va != vb);  /* NEQ/UNORD */
    mask += (v2di)(va < vb);   /* LT */
    mask += (v2di)(va <= vb);  /* LE */
    mask += (v2di)(va > vb);   /* GT */
    mask += (v2di)(va >= vb);  /* GE */
    
    /* Ordered/unordered comparisons */
    v2df nan_vec = {__builtin_nan(""), __builtin_nan("")};
    mask += (v2di)(va == nan_vec);  /* Should be false for all non-NaN */
    mask += (v2di)(va != nan_vec);  /* Should be true for all non-NaN */
    
    return mask;
}

/* Function with inline assembly using condition codes */
__attribute__((noinline))
static int asm_fp_conditions(double a, double b) {
    int res_eq = 0, res_lt = 0, res_unord = 0;
    
    /* Use ucomisd and check condition codes via set instructions */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(res_eq)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(res_lt)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(res_unord)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0"
        : "=t"(cmov_result)
        : "u"(a), "u"(b), "t"(3.14159)
        : "cc"
    );
    
    return res_eq + (res_lt << 1) + (res_unord << 2) + (int)cmov_result;
}

/* Main test function */
__attribute__((noinline))
static int exhaustive_fp_test(void) {
    int checksum = 0;
    
    /* Test matrix of all value combinations */
    double test_values[] = {
        0.0, 1.0, -1.0, 2.5, -2.5,
        __builtin_inf(), -__builtin_inf(),
        __builtin_nan(""), __builtin_nans("")
    };
    
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive pairwise comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            checksum ^= stress_fp_comparisons(
                test_values[i], test_values[j],
                test_values[(i + 1) % num_values],
                test_values[(j + 1) % num_values]
            );
            
            /* Also test with inline assembly */
            checksum += asm_fp_conditions(test_values[i], test_values[j]);
        }
    }
    
    /* Vectorized comparisons */
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_inf()};
    
    v2di mask1 = vector_fp_comparisons(vec_a, vec_b);
    v2di mask2 = vector_fp_comparisons(vec_a, vec_nan);
    v2di mask3 = vector_fp_comparisons(vec_nan, vec_nan);
    
    checksum += mask1[0] + mask1[1];
    checksum += mask2[0] + mask2[1];
    checksum += mask3[0] + mask3[1];
    
    /* Additional unordered-specific tests */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nans("");
    volatile double inf = __builtin_inf();
    
    /* These should trigger UNORDERED paths */
    if (nan1 == 1.0) checksum ^= 0x1000;  /* Always false */
    if (1.0 == nan1) checksum ^= 0x2000;  /* Always false */
    if (nan1 != nan2) checksum ^= 0x4000; /* Always true */
    if (nan1 < inf) checksum ^= 0x8000;   /* Unordered, false */
    if (inf > nan1) checksum ^= 0x10000;  /* Unordered, false */
    
    /* Complex conditional expressions */
    double cond_result = 
        (nan1 == nan2) ? 1.0 :
        (nan1 < 0.0) ? 2.0 :
        (0.0 > nan1) ? 3.0 :
        (inf <= nan1) ? 4.0 :
        (nan1 >= inf) ? 5.0 : 6.0;
    
    checksum += (int)cond_result;
    
    /* Loop with FP comparisons to generate conditional branches */
    volatile double accum = 0.0;
    for (int i = 0; i < 10; i++) {
        volatile double val = i * 0.5;
        if (val < nan1) accum += 1.0;  /* Never true */
        if (val > nan2) accum += 2.0;  /* Never true */
        if (val != val) accum += 3.0;  /* Never true (val is not NaN) */
        if (val == val) accum += 4.0;  /* Always true */
        if (val < inf) accum += 5.0;   /* Always true for finite val */
        if (val > -inf) accum += 6.0;  /* Always true for finite val */
    }
    
    checksum += (int)accum;
    
    return checksum;
}

int main(void) {
    printf("Starting FP comparison condition code stress test...\n");
    
    int result = exhaustive_fp_test();
    
    printf("Result checksum: %d (0x%08x)\n", result, result);
    printf("Test completed.\n");
    
    return 0;
}
