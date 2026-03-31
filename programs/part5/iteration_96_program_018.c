/* cc_coverage.c - Program to trigger x86 condition code name printing */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of various x86 condition codes */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Array with mixed NaN and normal values */
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i % 7 == 0) ? nan_val : (i * 1.5);
        arr2[i] = (i % 11 == 0) ? nan_val : (i * 0.75);
    }
    
    volatile int cc_accumulator = 0;
    volatile int temp_result;
    
    /* Loop performing various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all standard FP comparisons */
        temp_result = (a < b) ? 1 : 0;   /* May generate LT/UNLT */
        cc_accumulator += temp_result;
        
        temp_result = (a <= b) ? 2 : 0;  /* May generate LE/UNLE */
        cc_accumulator += temp_result;
        
        temp_result = (a > b) ? 3 : 0;   /* May generate GT/UNGT */
        cc_accumulator += temp_result;
        
        temp_result = (a >= b) ? 4 : 0;  /* May generate GE/UNGE */
        cc_accumulator += temp_result;
        
        temp_result = (a == b) ? 5 : 0;  /* May generate EQ/UNEQ */
        cc_accumulator += temp_result;
        
        temp_result = (a != b) ? 6 : 0;  /* May generate NEQ/LTGT */
        cc_accumulator += temp_result;
        
        /* Ordered/unordered checks */
        temp_result = (!isunordered(a, b)) ? 7 : 0;  /* ORDERED */
        cc_accumulator += temp_result;
        
        temp_result = (isunordered(a, b)) ? 8 : 0;   /* UNORDERED */
        cc_accumulator += temp_result;
    }
    
    /* Direct inline assembly with %C constraint to trigger printing */
    int var1 = 42;
    int var2 = 100;
    int result = 0;
    
    /* UNORDERED condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(result)
        : "r"(var1), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition code */
    asm volatile (
        "cmov%C1 %2, %0\n\t"
        : "+r"(result)
        : "i"(1), "r"(var2)  /* 1 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition code */
    asm volatile (
        "cmov%C2 %2, %0\n\t"
        : "+r"(result)
        : "i"(2), "r"(var1)  /* 2 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition code */
    asm volatile (
        "cmov%C3 %2, %0\n\t"
        : "+r"(result)
        : "i"(3), "r"(var2)  /* 3 = UNGE */
        : "cc"
    );
    
    /* UNGT condition code */
    asm volatile (
        "cmov%C4 %2, %0\n\t"
        : "+r"(result)
        : "i"(4), "r"(var1)  /* 4 = UNGT */
        : "cc"
    );
    
    /* UNLE condition code */
    asm volatile (
        "cmov%C5 %2, %0\n\t"
        : "+r"(result)
        : "i"(5), "r"(var2)  /* 5 = UNLE */
        : "cc"
    );
    
    /* UNLT condition code */
    asm volatile (
        "cmov%C6 %2, %0\n\t"
        : "+r"(result)
        : "i"(6), "r"(var1)  /* 6 = UNLT */
        : "cc"
    );
    
    /* LTGT condition code */
    asm volatile (
        "cmov%C7 %2, %0\n\t"
        : "+r"(result)
        : "i"(7), "r"(var2)  /* 7 = LTGT */
        : "cc"
    );
    
    /* More complex FP expressions that generate condition codes */
    volatile double x = nan_val;
    volatile double y = normal1;
    volatile double z = normal2;
    
    /* Chain of FP comparisons */
    if ((x < y) && (y > z) && (x != z)) {
        cc_accumulator += 1000;
    }
    
    /* Ternary with FP condition on integer target */
    int int_result = (x == y) ? 123 : 456;
    int_result = (x != y) ? int_result + 1 : int_result - 1;
    int_result = (x < y) ? int_result * 2 : int_result / 2;
    int_result = (x > y) ? int_result + 100 : int_result - 100;
    
    /* Use __builtin_constant_p to prevent elimination */
    if (__builtin_constant_p(cc_accumulator)) {
        /* This branch won't be taken, but prevents optimization */
        asm volatile ("nop");
    }
    
    printf("Accumulator: %d\n", cc_accumulator);
    printf("Result: %d\n", result);
    printf("Int result: %d\n", int_result);
}

/* Function using SSE comparisons */
void sse_comparisons(void) {
    __m128d vec1 = _mm_set_pd(__builtin_nan(""), 1.0);
    __m128d vec2 = _mm_set_pd(2.0, __builtin_nan(""));
    
    /* These intrinsics may generate condition codes */
    __m128d cmp_eq = _mm_cmpeq_pd(vec1, vec2);
    __m128d cmp_lt = _mm_cmplt_pd(vec1, vec2);
    __m128d cmp_le = _mm_cmple_pd(vec1, vec2);
    __m128d cmp_unord = _mm_cmpunord_pd(vec1, vec2);
    __m128d cmp_ord = _mm_cmpord_pd(vec1, vec2);
    
    volatile double result[4];
    _mm_storeu_pd(result, cmp_eq);
    _mm_storeu_pd(result + 2, cmp_lt);
    
    /* Conditional move based on FP comparison */
    double a = __builtin_nan("");
    double b = 3.14;
    double c = 2.71;
    double d = 0.0;
    
    /* This may generate conditional moves with FP condition codes */
    d = (a < b) ? c : d;
    d = (a != b) ? d + 1.0 : d - 1.0;
    d = (!isunordered(a, b)) ? d * 2.0 : d / 2.0;
    
    printf("SSE comparison result: %f\n", d);
}

int main(void) {
    printf("Generating x86 condition codes for coverage...\n");
    
    generate_condition_codes();
    sse_comparisons();
    
    /* Additional unordered scenarios */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0xdead");
    volatile double num = 42.0;
    
    /* All possible unordered comparisons */
    volatile int cmp_results[12];
    cmp_results[0] = (nan1 < nan2);   /* UNORDERED */
    cmp_results[1] = (nan1 <= nan2);  /* UNORDERED */
    cmp_results[2] = (nan1 > nan2);   /* UNORDERED */
    cmp_results[3] = (nan1 >= nan2);  /* UNORDERED */
    cmp_results[4] = (nan1 == nan2);  /* UNORDERED */
    cmp_results[5] = (nan1 != nan2);  /* UNORDERED */
    
    cmp_results[6] = (nan1 < num);    /* UNORDERED */
    cmp_results[7] = (nan1 <= num);   /* UNORDERED */
    cmp_results[8] = (nan1 > num);    /* UNORDERED */
    cmp_results[9] = (nan1 >= num);   /* UNORDERED */
    cmp_results[10] = (nan1 == num);  /* UNORDERED */
    cmp_results[11] = (nan1 != num);  /* UNORDERED */
    
    /* Mixed ordered comparisons */
    cmp_results[0] = (num < 100.0);   /* ORDERED LT */
    cmp_results[1] = (num <= 42.0);   /* ORDERED LE/EQ */
    cmp_results[2] = (num > 10.0);    /* ORDERED GT */
    cmp_results[3] = (num >= 42.0);   /* ORDERED GE/EQ */
    cmp_results[4] = (num == 42.0);   /* ORDERED EQ */
    cmp_results[5] = (num != 100.0);  /* ORDERED NEQ */
    
    printf("Comparison results array populated.\n");
    
    return 0;
}
