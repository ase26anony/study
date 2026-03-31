/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

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
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN - will trigger UNORDERED paths */
    if (v1 == v_nan) result ^= 64;      /* Should be false, may be unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true if v1 is not NaN */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v_nan <= v2) result ^= 512;     /* Unordered */
    if (v_nan > v_inf) result ^= 1024;  /* Unordered */
    if (v_neg_inf >= v_nan) result ^= 2048; /* Unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096; /* Always false */
    if (v_nan != v_nan) result ^= 8192; /* Always true (NaN != NaN) */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_neg_inf < v_inf) result ^= 32768;
    if (v1 <= v_inf) result ^= 65536;
    if (v_neg_inf <= v2) result ^= 131072;
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v_nan) ? 1.0 : 2.0;  /* UNORDERED path */
    result ^= (int)(cond_val * 1000);
    
    cond_val = (v_nan == v2) ? 3.0 : 4.0;        /* UNORDERED path */
    result ^= (int)(cond_val * 100);
    
    cond_val = (v1 != v_nan) ? 5.0 : 6.0;        /* ORDERED path */
    result ^= (int)(cond_val * 10);
    
    /* Goto-based control flow to prevent optimization */
    if (v1 < v2) goto label_lt;
    if (v1 == v2) goto label_eq;
    if (v1 > v2) goto label_gt;
    if (v_nan < v1) goto label_unordered;  /* This will jump */
    
label_lt:
    result += 1000000;
    goto label_continue;
    
label_eq:
    result += 2000000;
    goto label_continue;
    
label_gt:
    result += 3000000;
    goto label_continue;
    
label_unordered:
    result += 4000000;  /* UNORDERED condition */
    
label_continue:
    /* More comparisons in switch-like pattern */
    int cmp_result = 0;
    if (v1 < v2) cmp_result = 1;
    else if (v1 == v2) cmp_result = 2;
    else if (v1 > v2) cmp_result = 3;
    else if (v_nan < v1) cmp_result = 4;  /* Unordered case */
    
    result ^= cmp_result * 1000000000;
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df sum_mask = {0.0, 0.0};
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df v1 = {arr1[i], arr1[i+1]};
        v2df v2 = {arr2[i], arr2[i+1]};
        
        /* Various vector comparisons that generate condition codes */
        v2di cmp1 = (v2di)(v1 == v2);  /* EQ */
        v2di cmp2 = (v2di)(v1 != v2);  /* NEQ/UNEQ */
        v2di cmp3 = (v2di)(v1 < v2);   /* LT */
        v2di cmp4 = (v2di)(v1 <= v2);  /* LE */
        v2di cmp5 = (v2di)(v1 > v2);   /* GT */
        v2di cmp6 = (v2di)(v1 >= v2);  /* GE */
        
        /* Combine results */
        v2df mask1 = (v2df)(cmp1 | cmp2 | cmp3 | cmp4 | cmp5 | cmp6);
        sum_mask += mask1;
    }
    
    /* Extract result to prevent optimization */
    double res_arr[2];
    memcpy(res_arr, &sum_mask, sizeof(sum_mask));
    return (int)(res_arr[0] + res_arr[1]);
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that uses FP comparison condition codes */
    
    /* UNORDERED/ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (nan_val)
        : "al", "cc"
    );
    
    /* UNEQ test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* UNLT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    result ^= (cc_result << 8);
    
    /* UNLE test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    result ^= (cc_result << 16);
    
    /* UNGT test (nle) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    result ^= (cc_result << 24);
    
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
    
    /* Test all combinations of value pairs */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val};
    const char* value_names[] = {"normal1", "normal2", "nan", "inf", "-inf"};
    
    printf("Testing FP comparisons...\n");
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            printf("  %s vs %s: ", value_names[i], value_names[j]);
            int res = stress_fp_comparisons(test_values[i], test_values[j], 
                                           nan_val, inf_val, neg_inf_val);
            checksum ^= res;
            printf("result = %d\n", res);
        }
    }
    
    /* Vectorized comparisons */
    printf("\nTesting vectorized FP comparisons...\n");
    double arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = (i % 3) * 0.3;
        /* Insert some special values */
        if (i == 3) arr1[i] = nan_val;
        if (i == 7) arr2[i] = inf_val;
    }
    
    int vec_res = vector_fp_comparisons(arr1, arr2, 10);
    checksum ^= vec_res;
    printf("  Vector result = %d\n", vec_res);
    
    /* Inline assembly condition code tests */
    printf("\nTesting inline assembly with FP condition codes...\n");
    int asm_res = asm_fp_conditions(normal1, normal2, nan_val);
    checksum ^= asm_res;
    printf("  Assembly result = %d\n", asm_res);
    
    /* Additional unordered comparison stress */
    printf("\nAdditional unordered comparison tests...\n");
    
    /* These should trigger UNORDERED, UNEQ, UNLT, etc. */
    volatile double v = 1.0;
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    
    /* Complex expression mixing ordered and unordered */
    int complex_result = 0;
    
    if (!(v < nan1)) complex_result |= 1;      /* UNORDERED -> nlt */
    if (!(nan1 <= v)) complex_result |= 2;     /* UNORDERED -> nle */
    if (v != nan1) complex_result |= 4;        /* ORDERED */
    if (!(v == nan1)) complex_result |= 8;     /* ORDERED */
    if (nan1 != nan2) complex_result |= 16;    /* Always true */
    if (!(nan1 == nan2)) complex_result |= 32; /* Always true */
    
    checksum ^= complex_result;
    printf("  Complex unordered result = %d\n", complex_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum = %d\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int* dummy = (volatile int*)&checksum;
    return *dummy & 255;  /* Return non-zero to indicate execution */
}
