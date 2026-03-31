/* fp_condition_stress.c - Exhaustive test of FP comparison condition codes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function with complex control flow to prevent optimization */
__attribute__((noinline))
double fp_compare_stress(double a, double b, double nan_val, double inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double nan = nan_val;
    volatile double inf = inf_val;
    volatile double neg_inf = -inf_val;
    
    int result = 0;
    
    /* Label maze for goto-based control flow */
    start_normal_comparisons:
    
    /* 1. Normal number comparisons - will generate various condition codes */
    if (v1 == v2) result |= 1;
    if (v1 != v2) result |= 2;
    if (v1 < v2)  result |= 4;
    if (v1 <= v2) result |= 8;
    if (v1 > v2)  result |= 16;
    if (v1 >= v2) result |= 32;
    
    /* 2. Comparisons with NaN - triggers UNORDERED paths */
    if (nan == v1) result |= 64;      /* Always false, but generates UNORDERED */
    if (v1 < nan)  result |= 128;     /* UNORDERED */
    if (nan <= v2) result |= 256;     /* UNORDERED */
    if (nan != nan) result |= 512;    /* Always true (NaN != NaN) */
    
    /* 3. Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : 
                        (v1 == v2) ? 2.0 :
                        (v1 > v2) ? 3.0 :
                        (nan == nan) ? 4.0 : 5.0;
    
    sink = cond_result;
    
    /* 4. More NaN comparisons for different condition codes */
    if (!(nan < v1)) result |= 1024;   /* ORDERED or UNGE */
    if (!(v1 > nan)) result |= 2048;   /* ORDERED or UNLE */
    
    /* Goto to create non-linear control flow */
    if (result & 1) goto nan_comparisons;
    
    /* 5. Infinity comparisons */
    if (v1 == inf) result |= 4096;
    if (v1 < inf)  result |= 8192;
    if (neg_inf < v1) result |= 16384;
    if (inf == inf) result |= 32768;   /* Always true */
    
    nan_comparisons:
    /* 6. Direct unordered comparisons */
    int unordered = (v1 != v1) || (v2 != v2);  /* Check for NaN */
    if (unordered) result |= 65536;
    
    /* 7. Ordered comparison */
    int ordered = (v1 == v1) && (v2 == v2);
    if (ordered) result |= 131072;
    
    return (double)result;
}

/* Vectorized comparison function */
__attribute__((noinline))
v2di vector_compare_stress(v2df a, v2df b, v2df nan_vec) {
    /* Various vector comparisons generating different condition codes */
    v2di mask1 = (v2di)(a == b);    /* EQ */
    v2di mask2 = (v2di)(a != b);    /* NEQ/UNEQ */
    v2di mask3 = (v2di)(a < b);     /* LT */
    v2di mask4 = (v2di)(a <= b);    /* LE */
    v2di mask5 = (v2di)(a > b);     /* GT */
    v2di mask6 = (v2di)(a >= b);    /* GE */
    
    /* NaN comparisons - trigger unordered conditions */
    v2di mask7 = (v2di)(a == nan_vec);  /* UNORDERED */
    v2di mask8 = (v2di)(nan_vec < b);   /* UNORDERED */
    v2di mask9 = (v2di)(a != a);        /* UNORDERED check */
    
    /* Combine masks */
    return mask1 ^ mask2 ^ mask3 ^ mask4 ^ mask5 ^ mask6 ^ mask7 ^ mask8 ^ mask9;
}

/* Function with inline assembly using condition codes */
__attribute__((noinline))
int asm_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    uint8_t byte_result;
    
    /* 1. ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 0);  /* UNORDERED */
    
    /* 2. ucomisd with seta (above, for GT & !UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 1);  /* GT & ORDERED */
    
    /* 3. ucomisd with setb (below, for LT & !UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 2);  /* LT & ORDERED */
    
    /* 4. ucomisd with sete (equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(byte_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (byte_result << 3);  /* EQ & ORDERED */
    
    /* 5. Comparison with NaN */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(byte_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result |= (byte_result << 4);  /* UNORDERED with NaN */
    
    return result;
}

/* Main test driver */
int main() {
    /* Initialize test values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -inf_val;
    double zero = 0.0;
    
    printf("Starting FP comparison condition code stress test...\n");
    
    /* Accumulator to prevent dead code elimination */
    volatile double checksum = 0.0;
    
    /* Test matrix of value pairs */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf, zero};
    const char* value_names[] = {"normal1", "normal2", "nan", "inf", "-inf", "zero"};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            printf("Testing %s vs %s: ", value_names[i], value_names[j]);
            
            /* Force all comparison types */
            double cmp_result = fp_compare_stress(
                test_values[i], 
                test_values[j],
                nan_val,
                inf_val
            );
            
            /* Use inline assembly condition codes */
            int asm_result = asm_condition_codes(
                test_values[i],
                test_values[j],
                nan_val
            );
            
            checksum += cmp_result + asm_result;
            printf("checksum += %f + %d\n", cmp_result, asm_result);
        }
    }
    
    /* Vectorized tests */
    printf("\nVectorized comparisons:\n");
    v2df vec1 = {normal1, normal2};
    v2df vec2 = {normal2, normal1};
    v2df nan_vec = {nan_val, nan_val};
    
    v2di vec_result = vector_compare_stress(vec1, vec2, nan_vec);
    
    /* Extract and use vector results */
    long long* vec_arr = (long long*)&vec_result;
    checksum += vec_arr[0] + vec_arr[1];
    printf("Vector checksum addition: %lld + %lld\n", vec_arr[0], vec_arr[1]);
    
    /* Additional unordered-specific tests */
    printf("\nUnordered-specific tests:\n");
    
    /* UNORDERED case: NaN comparison */
    if (nan_val != nan_val) {  /* Always true */
        checksum += 1000;
        printf("NaN != NaN triggered\n");
    }
    
    /* ORDERED case: normal comparison */
    if (normal1 == normal1 && normal2 == normal2) {  /* Always true */
        checksum += 2000;
        printf("Ordered comparison triggered\n");
    }
    
    /* UNEQ: unordered or equal */
    volatile double un_eq_test = (nan_val == normal1) ? 1.0 : 0.0;
    checksum += un_eq_test;
    
    /* UNGE: not less than (unordered or greater or equal) */
    volatile double un_ge_test = !(normal1 < nan_val) ? 1.0 : 0.0;
    checksum += un_ge_test;
    
    /* UNGT: not less than or equal */
    volatile double un_gt_test = !(normal1 <= nan_val) ? 1.0 : 0.0;
    checksum += un_gt_test;
    
    /* UNLE: unordered or less or equal */
    volatile double un_le_test = (nan_val <= normal1) ? 1.0 : 0.0;
    checksum += un_le_test;
    
    /* UNLT: unordered or less than */
    volatile double un_lt_test = (nan_val < normal1) ? 1.0 : 0.0;
    checksum += un_lt_test;
    
    /* LTGT: less than or greater than (but not equal, not unordered) */
    volatile double ltgt_test = (normal1 < normal2 || normal1 > normal2) ? 1.0 : 0.0;
    checksum += ltgt_test;
    
    /* Final output to prevent optimization */
    printf("\nFinal checksum: %f\n", (double)checksum);
    printf("FP comparison stress test completed.\n");
    
    return (int)(checksum) & 0xFF;  /* Return non-constant value */
}
