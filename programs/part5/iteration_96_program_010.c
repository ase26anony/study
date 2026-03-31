/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, var, src) \
    do { \
        int __cond = (cond); \
        int __var = (var); \
        int __src = (src); \
        /* Use %C to print condition code name */ \
        __asm__ volatile ("cmov%C0 %2, %0" \
                         : "+r" (__var) \
                         : "i" (__cond), "r" (__src) \
                         : "cc"); \
        (var) = __var; \
    } while(0)

/* Generate all condition codes via inline assembly */
static void generate_all_condition_codes(void) {
    int result = 0;
    int src = 42;
    
    /* Direct inline assembly with %C constraint */
    __asm__ volatile ("# UNORDERED condition code" : : :);
    FORCE_CC_PRINTING(UNORDERED, result, src);
    
    __asm__ volatile ("# ORDERED condition code" : : :);
    FORCE_CC_PRINTING(ORDERED, result, src);
    
    __asm__ volatile ("# UNEQ condition code" : : :);
    FORCE_CC_PRINTING(UNEQ, result, src);
    
    __asm__ volatile ("# UNGE condition code" : : :);
    FORCE_CC_PRINTING(UNGE, result, src);
    
    __asm__ volatile ("# UNGT condition code" : : :);
    FORCE_CC_PRINTING(UNGT, result, src);
    
    __asm__ volatile ("# UNLE condition code" : : :);
    FORCE_CC_PRINTING(UNLE, result, src);
    
    __asm__ volatile ("# UNLT condition code" : : :);
    FORCE_CC_PRINTING(UNLT, result, src);
    
    __asm__ volatile ("# LTGT condition code" : : :);
    FORCE_CC_PRINTING(LTGT, result, src);
}

/* Create unordered floating-point comparisons */
static volatile int cc_accumulator = 0;

int main(void) {
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.5;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("");
            arr2[i] = __builtin_nan("");
        }
    }
    
    /* Perform various floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        volatile int cmp_result;
        
        /* Generate all standard FP comparisons */
        cmp_result = (a < b) ? 1 : 0;   /* May generate UNLT/UNORDERED */
        cc_accumulator += cmp_result;
        
        cmp_result = (a <= b) ? 2 : 0;  /* May generate UNLE/UNORDERED */
        cc_accumulator += cmp_result;
        
        cmp_result = (a > b) ? 3 : 0;   /* May generate UNGT/UNORDERED */
        cc_accumulator += cmp_result;
        
        cmp_result = (a >= b) ? 4 : 0;  /* May generate UNGE/UNORDERED */
        cc_accumulator += cmp_result;
        
        cmp_result = (a == b) ? 5 : 0;  /* May generate UNEQ/UNORDERED */
        cc_accumulator += cmp_result;
        
        cmp_result = (a != b) ? 6 : 0;  /* May generate LTGT/UNORDERED */
        cc_accumulator += cmp_result;
        
        /* Ordered comparison using isnan check */
        int ordered = (!isnan(a) && !isnan(b)) ? 7 : 0;
        cc_accumulator += ordered;
        
        /* Unordered comparison */
        int unordered = (isnan(a) || isnan(b)) ? 8 : 0;
        cc_accumulator += unordered;
    }
    
    /* Force conditional move generation with FP conditions */
    {
        double x = __builtin_nan("");
        double y = 3.14159;
        int target = 0;
        int source = 99;
        
        /* These may generate cmov with condition codes */
        target = (x < y) ? source : target;      /* UNORDERED/UNLT */
        target = (x == x) ? source : target;     /* UNORDERED/UNEQ */
        target = (y != y) ? source : target;     /* UNORDERED/LTGT */
        target = (!isnan(x)) ? source : target;  /* ORDERED */
        
        cc_accumulator += target;
    }
    
    /* Generate condition codes via builtins if available */
#ifdef __SSE__
    {
        __m128d v1 = _mm_set_pd(__builtin_nan(""), 1.0);
        __m128d v2 = _mm_set_pd(2.0, __builtin_nan(""));
        __m128d cmp;
        
        /* Various SSE comparisons that generate condition codes */
        cmp = _mm_cmpeq_pd(v1, v2);    /* EQ/UNEQ */
        cmp = _mm_cmplt_pd(v1, v2);    /* LT/UNLT */
        cmp = _mm_cmple_pd(v1, v2);    /* LE/UNLE */
        cmp = _mm_cmpgt_pd(v1, v2);    /* GT/UNGT */
        cmp = _mm_cmpge_pd(v1, v2);    /* GE/UNGE */
        cmp = _mm_cmpneq_pd(v1, v2);   /* NEQ/LTGT */
        cmp = _mm_cmpord_pd(v1, v2);   /* ORDERED */
        cmp = _mm_cmpunord_pd(v1, v2); /* UNORDERED */
        
        /* Store to prevent elimination */
        volatile __m128d store_cmp = cmp;
        (void)store_cmp;
    }
#endif
    
    /* Call function to generate condition codes via inline assembly */
    generate_all_condition_codes();
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
