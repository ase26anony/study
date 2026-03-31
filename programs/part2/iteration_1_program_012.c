/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -fno-trapping-math -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -fno-trapping-math */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    int temp;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* UNORDERED cases - comparisons with NaN */
    if (v_nan == v1) result ^= 1;          /* Should be false, but tests unordered */
    if (v1 == v_nan) result ^= 2;          /* Same */
    if (v_nan != v_nan) result ^= 4;       /* Always true: NaN != NaN */
    
    /* ORDERED cases - normal comparisons */
    if (v1 == v2) result ^= 8;
    if (v1 != v2) result ^= 16;
    
    /* UNLT, UNLE, UNGT, UNGE cases */
    if (v1 < v_nan) result ^= 32;          /* Unordered comparison */
    if (v_nan <= v2) result ^= 64;         /* Unordered comparison */
    if (v1 > v_nan) result ^= 128;         /* Unordered comparison */
    if (v_nan >= v2) result ^= 256;        /* Unordered comparison */
    
    /* LTGT case */
    if (v1 < v2 || v1 > v2) result ^= 512; /* LTGT: less or greater, but not equal */
    
    /* UNEQ case - equal or unordered */
    if (!(v1 < v2) && !(v1 > v2)) result ^= 1024; /* UNEQ: equal or unordered */
    
    /* Complex conditional expressions using ?: operator */
    double d1 = (v1 < v2) ? 1.0 : 2.0;
    double d2 = (v1 <= v2) ? 3.0 : 4.0;
    double d3 = (v1 > v2) ? 5.0 : 6.0;
    double d4 = (v1 >= v2) ? 7.0 : 8.0;
    double d5 = (v1 == v2) ? 9.0 : 10.0;
    double d6 = (v1 != v2) ? 11.0 : 12.0;
    
    result ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5 ^ (int)d6;
    
    /* Comparisons with infinity */
    if (v1 < v_inf) result ^= 2048;
    if (v_neg_inf < v2) result ^= 4096;
    if (v_inf > v_neg_inf) result ^= 8192;
    
    /* Use goto to create complex control flow */
    if (v1 < v2) goto label_lt;
    if (v1 > v2) goto label_gt;
    goto label_eq;
    
label_lt:
    result += 1000;
    goto label_cont;
    
label_gt:
    result += 2000;
    goto label_cont;
    
label_eq:
    result += 3000;
    
label_cont:
    /* More comparisons in the goto flow */
    if (v_nan == v_nan) { /* Always false */
        result += 4000;
    }
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {3.0, 2.0};
    v2df vec_nan = {__builtin_nan(""), 4.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons that should generate cmppd/ucomisd with condition codes */
    v2di mask1 = (v2di)(vec1 < vec2);      /* LT */
    v2di mask2 = (v2di)(vec1 <= vec2);     /* LE */
    v2di mask3 = (v2di)(vec1 > vec2);      /* GT */
    v2di mask4 = (v2di)(vec1 >= vec2);     /* GE */
    v2di mask5 = (v2di)(vec1 == vec2);     /* EQ */
    v2di mask6 = (v2di)(vec1 != vec2);     /* NEQ */
    
    /* Unordered comparisons with NaN */
    v2di mask7 = (v2di)(vec1 < vec_nan);   /* UNLT */
    v2di mask8 = (v2di)(vec_nan <= vec2);  /* UNLE */
    v2di mask9 = (v2di)(vec1 > vec_nan);   /* UNGT */
    v2di mask10 = (v2di)(vec_nan >= vec2); /* UNGE */
    v2di mask11 = (v2di)(vec_nan == vec_nan); /* UNORDERED/ORDERED */
    
    /* Extract results to prevent optimization */
    long long *m1 = (long long*)&mask1;
    long long *m2 = (long long*)&mask2;
    
    return (int)(m1[0] ^ m1[1] ^ m2[0] ^ m2[1]);
}

/* Function with inline assembly using condition codes */
static int asm_fp_comparisons(double a, double b) {
    int result = 0;
    int setp_result, setnp_result, seta_result, setae_result;
    int setb_result, setbe_result, sete_result, setne_result;
    
    /* Various inline assembly blocks that use FP condition codes */
    
    /* UNORDERED/ORDERED tests */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setp_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setnp_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* Above/below tests (for unsigned comparisons) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (seta_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setae_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setb_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setbe_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* Equal/not equal tests */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (sete_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (setne_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    result = setp_result ^ setnp_result ^ seta_result ^ setae_result ^
             setb_result ^ setbe_result ^ sete_result ^ setne_result;
    
    return result;
}

/* Main test function */
int main(void) {
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double normal_vals[] = {1.0, 2.0, 3.0, 0.0, -1.0, -2.0};
    
    int total_result = 0;
    
    /* Test various combinations of values */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            total_result ^= stress_fp_comparisons(
                normal_vals[i], 
                normal_vals[j],
                nan_val,
                inf_val,
                neg_inf_val
            );
            
            /* Also test with NaN as one operand */
            total_result ^= stress_fp_comparisons(
                nan_val,
                normal_vals[j],
                nan_val,
                inf_val,
                neg_inf_val
            );
            
            /* Test with infinity */
            total_result ^= stress_fp_comparisons(
                inf_val,
                normal_vals[j],
                nan_val,
                inf_val,
                neg_inf_val
            );
        }
    }
    
    /* Test vectorized comparisons */
    total_result ^= vector_fp_comparisons();
    
    /* Test inline assembly comparisons */
    total_result ^= asm_fp_comparisons(1.0, 2.0);
    total_result ^= asm_fp_comparisons(2.0, 1.0);
    total_result ^= asm_fp_comparisons(1.0, 1.0);
    total_result ^= asm_fp_comparisons(nan_val, 1.0);
    total_result ^= asm_fp_comparisons(1.0, nan_val);
    total_result ^= asm_fp_comparisons(nan_val, nan_val);
    
    /* Additional exhaustive NaN comparisons */
    volatile double v_nan = nan_val;
    volatile double v_num = 5.0;
    
    /* These should trigger UNORDERED condition codes */
    if (v_nan < v_num) total_result += 1;
    if (v_nan <= v_num) total_result += 2;
    if (v_nan > v_num) total_result += 4;
    if (v_nan >= v_num) total_result += 8;
    if (v_nan == v_num) total_result += 16;
    if (v_nan != v_num) total_result += 32;
    
    /* And the reverse */
    if (v_num < v_nan) total_result += 64;
    if (v_num <= v_nan) total_result += 128;
    if (v_num > v_nan) total_result += 256;
    if (v_num >= v_nan) total_result += 512;
    if (v_num == v_nan) total_result += 1024;
    if (v_num != v_nan) total_result += 2048;
    
    /* LTGT specific test: less or greater but not equal and not unordered */
    double x = 1.0, y = 2.0;
    if ((x < y) || (x > y)) {  /* LTGT when x and y are ordered and not equal */
        total_result += 4096;
    }
    
    /* UNEQ specific test: equal or unordered */
    if (!(x < y) && !(x > y)) {  /* UNEQ */
        total_result += 8192;
    }
    
    printf("Final result checksum: %d\n", total_result);
    return total_result != 0 ? 0 : 1;  /* Return 0 if we computed something */
}
