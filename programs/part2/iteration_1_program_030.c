/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true, unordered */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v1 <= v_nan) result ^= 512;     /* Unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Unordered */
    
    /* NaN vs NaN */
    if (v_nan == v_nan) result ^= 4096; /* False, unordered */
    if (v_nan != v_nan) result ^= 8192; /* True, UNORDERED case */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_inf > v_neg_inf) result ^= 32768;
    if (v_neg_inf < v_inf) result ^= 65536;
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    result += (int)cmov_result;
    
    cmov_result = (v1 != v_nan) ? 3.0 : 4.0;
    result += (int)cmov_result;
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v_nan) goto unordered_block;
    if (v1 < v_nan) goto unordered_block;
    
    normal_block:
    result += 1000;
    goto continue_block;
    
    unordered_block:
    result += 2000;  /* UNORDERED path */
    goto continue_block;
    
    continue_block:
    
    /* More comparisons with different NaN types */
    double nan2 = __builtin_nan("0x1234");
    volatile double v_nan2 = nan2;
    
    if (v_nan == v_nan2) result += 3000;
    if (v_nan != v_nan2) result += 4000;  /* UNORDERED */
    
    /* Compare with infinity */
    if (v1 == v_inf) result += 5000;
    if (v1 < v_inf) result += 6000;
    if (v_inf > v1) result += 7000;
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df sum_mask = {0.0, 0.0};
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df v1 = {arr1[i], arr1[i+1]};
        v2df v2 = {arr2[i], arr2[i+1]};
        
        /* Vector comparisons generating various condition codes */
        v2di cmp_eq = (v2di)(v1 == v2);
        v2di cmp_ne = (v2di)(v1 != v2);      /* LTGT -> "une" */
        v2di cmp_lt = (v2di)(v1 < v2);       /* UNLT -> "ult" */
        v2di cmp_le = (v2di)(v1 <= v2);      /* UNLE -> "ule" */
        v2di cmp_gt = (v2di)(v1 > v2);
        v2di cmp_ge = (v2di)(v1 >= v2);      /* UNGE -> "nlt" */
        
        /* Accumulate results */
        sum_mask += (v2df)cmp_eq + (v2df)cmp_ne + (v2df)cmp_lt + 
                   (v2df)cmp_le + (v2df)cmp_gt + (v2df)cmp_ge;
    }
    
    return (int)(sum_mask[0] + sum_mask[1]);
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that uses FP condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"           /* Compare a and b */
        "setp %%al\n\t"                /* Set if unordered (parity) */
        "sete %%bl\n\t"                /* Set if equal */
        "setb %%cl\n\t"                /* Set if below (less than) */
        "setbe %%dl\n\t"               /* Set if below or equal */
        "seta %%dil\n\t"               /* Set if above (greater than) */
        "setae %%sil\n\t"              /* Set if above or equal */
        "movzb %%al, %0\n\t"           /* Move result */
        : "=r"(result)
        : "x"(a), "x"(b)
        : "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "cc"
    );
    
    /* More assembly with NaN */
    asm volatile (
        "ucomisd %2, %1\n\t"           /* Compare a and NaN */
        "setnp %%al\n\t"               /* Set if ordered */
        "setne %%bl\n\t"               /* Set if not equal */
        "movzb %%al, %0\n\t"
        : "+r"(result)
        : "x"(a), "x"(nan_val)
        : "rax", "rbx", "cc"
    );
    
    /* Conditional move based on FP comparison */
    double cmov_val;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0\n\t"           /* Conditional move if below or equal */
        : "=t"(cmov_val)
        : "u"(a), "x"(b), "t"(1.0), "0"(2.0)
        : "cc"
    );
    
    result += (int)cmov_val;
    
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Exhaustive comparison matrix */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val};
    const char* value_names[] = {"normal1", "normal2", "nan", "inf", "-inf"};
    
    printf("Testing FP comparisons...\n");
    
    /* Test all pairs */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            printf("Comparing %s vs %s:\n", value_names[i], value_names[j]);
            checksum ^= stress_fp_comparisons(test_values[i], test_values[j], 
                                            nan_val, inf_val, neg_inf_val);
        }
    }
    
    /* Test vectorized comparisons */
    double arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = i * 0.3;
        if (i == 3) arr1[i] = nan_val;  /* Insert NaN */
        if (i == 7) arr2[i] = nan_val;  /* Insert NaN */
    }
    
    checksum += vector_fp_comparisons(arr1, arr2, 10);
    
    /* Test inline assembly conditions */
    checksum += asm_fp_conditions(normal1, normal2, nan_val);
    checksum += asm_fp_conditions(nan_val, normal1, nan_val);
    checksum += asm_fp_conditions(inf_val, normal1, nan_val);
    checksum += asm_fp_conditions(neg_inf_val, inf_val, nan_val);
    
    /* Additional unordered comparisons */
    volatile double v1 = normal1;
    volatile double v_nan = nan_val;
    
    /* These should trigger UNORDERED/ORDERED cases */
    if (!(v1 == v_nan)) checksum += 1;      /* ORDERED */
    if (v1 != v_nan) checksum += 2;         /* UNORDERED */
    if (!(v1 < v_nan)) checksum += 4;       /* UNGE -> "nlt" */
    if (!(v1 > v_nan)) checksum += 8;       /* UNLE -> "ule" */
    
    /* UNEQ case: equal OR unordered */
    if ((v1 == v1) || (v1 != v1)) checksum += 16;
    
    /* UNGT case: greater AND ordered */
    if ((v1 > normal2) && (v1 == v1)) checksum += 32;
    
    /* LTGT case: less OR greater (but not equal, not unordered) */
    if ((v1 < normal2) || (v1 > normal2)) checksum += 64;
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all code paths were executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
