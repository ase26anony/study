/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of all condition codes via floating-point comparisons */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Array with mixed NaN and normal values */
    double arr1[256];
    double arr2[256];
    
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i % 7 == 0) ? nan_val : (i * 1.5);
        arr2[i] = (i % 11 == 0) ? nan_val : (i * 0.75);
    }
    
    volatile int cc_accumulator = 0;
    
    /* Perform all floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate UNORDERED and ORDERED codes */
        if (isunordered(a, b)) {
            cc_accumulator |= 0x1;  /* UNORDERED */
        } else {
            cc_accumulator |= 0x2;  /* ORDERED */
        }
        
        /* Generate UNEQ (unordered or equal) */
        if (!(a > b) && !(a < b)) {
            cc_accumulator |= 0x4;  /* UNEQ */
        }
        
        /* Generate UNGE (unordered or greater-or-equal) */
        if (!(a < b)) {
            cc_accumulator |= 0x8;  /* UNGE */
        }
        
        /* Generate UNGT (unordered or greater) */
        if (!(a <= b)) {
            cc_accumulator |= 0x10; /* UNGT */
        }
        
        /* Generate UNLE (unordered or less-or-equal) */
        if (!(a > b)) {
            cc_accumulator |= 0x20; /* UNLE */
        }
        
        /* Generate UNLT (unordered or less) */
        if (!(a >= b)) {
            cc_accumulator |= 0x40; /* UNLT */
        }
        
        /* Generate LTGT (less or greater, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            cc_accumulator |= 0x80; /* LTGT */
        }
    }
    
    /* Direct inline assembly with %C constraint to trigger printing */
    int var1 = 42, var2 = 99, result = 0;
    
    /* UNORDERED condition */
    asm volatile (
        "cmov%C0 %2, %0\n\t"
        : "+r"(result)
        : "i"(0), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* ORDERED condition */
    asm volatile (
        "cmov%C1 %3, %0\n\t"
        : "+r"(result)
        : "i"(1), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* UNEQ condition */
    asm volatile (
        "cmov%C2 %3, %0\n\t"
        : "+r"(result)
        : "i"(2), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* UNGE condition */
    asm volatile (
        "cmov%C3 %3, %0\n\t"
        : "+r"(result)
        : "i"(3), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* UNGT condition */
    asm volatile (
        "cmov%C4 %3, %0\n\t"
        : "+r"(result)
        : "i"(4), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* UNLE condition */
    asm volatile (
        "cmov%C5 %3, %0\n\t"
        : "+r"(result)
        : "i"(5), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* UNLT condition */
    asm volatile (
        "cmov%C6 %3, %0\n\t"
        : "+r"(result)
        : "i"(6), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* LTGT condition */
    asm volatile (
        "cmov%C7 %3, %0\n\t"
        : "+r"(result)
        : "i"(7), "r"(var1), "0"(var2)
        : "cc"
    );
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("CMOV result: %d\n", result);
}

/* Additional test with ternary operators forcing conditional moves */
void test_ternary_cmov(void) {
    volatile double a = __builtin_nan("");
    volatile double b = 1.0;
    volatile double c = 2.0;
    
    int res1 = (a < b) ? 100 : 200;   /* May generate UNLT */
    int res2 = (a <= b) ? 101 : 201;  /* May generate UNLE */
    int res3 = (a > b) ? 102 : 202;   /* May generate UNGT */
    int res4 = (a >= b) ? 103 : 203;  /* May generate UNGE */
    int res5 = (a == b) ? 104 : 204;  /* May generate UNEQ */
    int res6 = (a != b) ? 105 : 205;  /* May generate LTGT */
    
    printf("Ternary results: %d %d %d %d %d %d\n", 
           res1, res2, res3, res4, res5, res6);
}

/* Test with SSE comparisons */
void test_sse_comparisons(void) {
    __m128d v1 = _mm_set_pd(__builtin_nan(""), 1.0);
    __m128d v2 = _mm_set_pd(2.0, __builtin_nan(""));
    
    /* These intrinsics expand to comparisons with condition codes */
    __m128d cmp_eq = _mm_cmpeq_pd(v1, v2);
    __m128d cmp_lt = _mm_cmplt_pd(v1, v2);
    __m128d cmp_le = _mm_cmple_pd(v1, v2);
    __m128d cmp_gt = _mm_cmpgt_pd(v1, v2);
    __m128d cmp_ge = _mm_cmpge_pd(v1, v2);
    __m128d cmp_neq = _mm_cmpneq_pd(v1, v2);
    
    volatile double* p = (double*)&cmp_eq;
    printf("SSE compare results: %f %f\n", p[0], p[1]);
}

int main(void) {
    printf("Generating x86 condition codes for coverage...\n");
    
    generate_condition_codes();
    test_ternary_cmov();
    test_sse_comparisons();
    
    /* Additional unordered comparisons */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    volatile double num = 42.0;
    
    /* Force generation of all condition code variants */
    if (nan1 < num) printf("LT with NaN\n");
    if (nan1 <= num) printf("LE with NaN\n");
    if (nan1 > num) printf("GT with NaN\n");
    if (nan1 >= num) printf("GE with NaN\n");
    if (nan1 == num) printf("EQ with NaN\n");
    if (nan1 != num) printf("NEQ with NaN\n");
    if (nan1 == nan2) printf("NaN == NaN\n");
    if (nan1 != nan2) printf("NaN != NaN\n");
    
    /* Mixed ordered/unordered checks */
    if (isunordered(nan1, num)) printf("Unordered detected\n");
    if (isordered(num, num)) printf("Ordered detected\n");
    
    return 0;
}
