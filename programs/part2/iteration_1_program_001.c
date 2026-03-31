/* fp_condition_stress.c - Exhaustive test of FP comparison condition codes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Function to stress FP comparisons with various conditions */
static void stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    volatile double inf = inf_val;
    volatile double ninf = neg_inf;
    
    int result = 0;
    
    /* Complex control flow with goto to prevent optimization */
    if (x == y) goto label_eq;
    if (x != y) goto label_neq;
    
label_eq:
    /* UNEQ and ORDERED cases */
    if (x == y) {
        result |= 1;
        /* Force unordered comparison with NaN */
        if (!(nan == nan)) {  /* UNORDERED: nan != nan */
            result |= 2;
        }
    }
    
    /* Jump around to confuse optimizer */
    if (result > 1000) goto impossible_label;
    
label_neq:
    /* LTGT case (ordered and not equal) */
    if (x < y) {
        result |= 4;
    }
    if (x > y) {
        result |= 8;
    }
    
    /* UNLT, UNLE, UNGE, UNGT cases with NaN */
    if (x < nan) {  /* UNORDERED or UNLT */
        result |= 16;
    }
    
    if (nan <= x) {  /* UNGE (not less than) */
        result |= 32;
    }
    
    if (x > nan) {  /* UNORDERED or UNGT */
        result |= 64;
    }
    
    if (nan >= x) {  /* UNLE (not greater than) */
        result |= 128;
    }
    
    /* Compare with infinity */
    if (x < inf) {
        result |= 256;
    }
    
    if (x > ninf) {
        result |= 512;
    }
    
    /* Compare NaN with NaN - always unordered */
    if (nan == nan) {
        /* This should never be taken */
        result |= 1024;
    }
    
    if (nan != nan) {  /* UNORDERED - should be true */
        result |= 2048;
    }
    
    /* Ordered comparison between normal values */
    if (x <= y) {
        result |= 4096;
    }
    
    if (x >= y) {
        result |= 8192;
    }
    
    sink = result;
    
impossible_label:
    return;
}

/* Vectorized FP comparisons using GCC vector extensions */
static void vector_fp_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    volatile v2df vx = vec_a;
    volatile v2df vy = vec_b;
    
    /* Perform various vector comparisons */
    v2di mask_eq = (v2di)(vx == vy);      /* EQ */
    v2di mask_neq = (v2di)(vx != vy);     /* NEQ/UNEQ */
    v2di mask_lt = (v2di)(vx < vy);       /* LT/UNLT */
    v2di mask_le = (v2di)(vx <= vy);      /* LE/UNLE */
    v2di mask_gt = (v2di)(vx > vy);       /* GT/UNGT */
    v2di mask_ge = (v2di)(vx >= vy);      /* GE/UNGE */
    
    /* Comparisons with NaN */
    v2di mask_nan_eq = (v2di)(vx == vec_nan);  /* UNORDERED */
    v2di mask_nan_neq = (v2di)(vx != vec_nan); /* ORDERED */
    
    /* Comparisons with infinity */
    v2di mask_inf_lt = (v2di)(vx < vec_inf);   /* LT */
    v2di mask_inf_gt = (v2di)(vx > vec_inf);   /* GT */
    
    /* Store results to prevent elimination */
    volatile v2di sink_vec[8];
    sink_vec[0] = mask_eq;
    sink_vec[1] = mask_neq;
    sink_vec[2] = mask_lt;
    sink_vec[3] = mask_le;
    sink_vec[4] = mask_gt;
    sink_vec[5] = mask_ge;
    sink_vec[6] = mask_nan_eq;
    sink_vec[7] = mask_nan_neq;
    
    /* Use __builtin_ia32_cmpsd for explicit condition codes */
    v2df cmp_result;
    
    /* Compare with different predicates */
    cmp_result = __builtin_ia32_cmpeqsd(vx, vy);    /* EQ */
    cmp_result = __builtin_ia32_cmpltsd(vx, vy);    /* LT */
    cmp_result = __builtin_ia32_cmplesd(vx, vy);    /* LE */
    cmp_result = __builtin_ia32_cmpunordsd(vx, vec_nan); /* UNORDERED */
    cmp_result = __builtin_ia32_cmpneqsd(vx, vy);   /* NEQ */
    cmp_result = __builtin_ia32_cmpnltsd(vx, vy);   /* NLT (UNGE) */
    cmp_result = __builtin_ia32_cmpnlesd(vx, vy);   /* NLE (UNGT) */
    
    sink = cmp_result[0];
}

/* Inline assembly that uses FP condition codes directly */
static void inline_asm_fp_conditions(double a, double b, double nan_val) {
    int8_t result_unordered, result_ordered, result_eq, result_lt, result_le;
    int8_t result_gt, result_ge, result_neq;
    
    /* Unordered comparison (UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result_unordered)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    
    /* Ordered comparison (ORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result_ordered)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Equal (UNEQ when used with unordered check) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(result_eq)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Less than (UNLT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(result_lt)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Less or equal (UNLE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(result_le)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Greater than (UNGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(result_gt)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Greater or equal (UNGE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(result_ge)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Not equal (LTGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result_neq)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r"(cmov_result)
        : "x"(a), "x"(b), "r"(100.0)
        : "cc"
    );
    
    sink = result_unordered + result_ordered + result_eq + result_lt + 
           result_le + result_gt + result_ge + result_neq + cmov_result;
}

/* Main test function with exhaustive comparison matrix */
static int test_all_fp_conditions(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double zero = 0.0;
    double neg = -3.14;
    double nan = __builtin_nan("");
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test all pairs of values */
    double test_values[] = {normal1, normal2, zero, neg, nan, inf, neg_inf};
    const char* value_names[] = {"normal1", "normal2", "zero", "neg", "nan", "inf", "neg_inf"};
    
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            /* Force use of all comparison operators */
            if (a == b) checksum += 1;
            if (a != b) checksum += 2;
            if (a < b) checksum += 4;
            if (a <= b) checksum += 8;
            if (a > b) checksum += 16;
            if (a >= b) checksum += 32;
            
            /* Use conditional operator (may generate conditional moves) */
            double cond_result = (a < b) ? a : b;
            checksum += (int)cond_result;
            
            /* Complex conditional expression */
            if ((a == b) || (a != a) || (b != b)) {
                checksum += 64;
            }
            
            /* Nested comparisons */
            if ((a < b) && (b > a)) {
                checksum += 128;
            }
        }
    }
    
    /* Call stress functions */
    stress_fp_comparisons(normal1, normal2, nan, inf, neg_inf);
    vector_fp_comparisons();
    inline_asm_fp_conditions(normal1, normal2, nan);
    
    return checksum;
}

int main(void) {
    printf("Testing all FP comparison condition codes...\n");
    
    int result = test_all_fp_conditions();
    
    printf("Result checksum: %d\n", result);
    printf("If non-zero, all code paths were likely exercised.\n");
    
    /* Use sink to prevent dead code elimination */
    printf("Sink value: %f\n", (double)sink);
    
    return 0;
}
