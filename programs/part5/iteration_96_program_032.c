/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, a, b) \
    do { \
        int result; \
        __asm__ volatile ( \
            "comisd %2, %1\n\t" \
            "set%C0 %0" \
            : "=r"(result) \
            : "x"(a), "x"(b), "i"(cond) \
            : "cc" \
        ); \
        cc_accumulator += result; \
    } while(0)

/* Direct inline assembly with %C constraint */
#define DIRECT_CC_PRINT(cond) \
    do { \
        int src = 42, dst = 0; \
        __asm__ volatile ( \
            "testl $1, %1\n\t" \
            "cmov%C0 %1, %0" \
            : "+r"(dst) \
            : "r"(src), "i"(cond) \
            : "cc" \
        ); \
        cc_accumulator += dst; \
    } while(0)

/* Mixed integer/float conditional move */
static int conditional_move_based_on_float(double a, double b, int code) {
    int result = 0;
    switch(code) {
        case 0: result = (a < b) ? 100 : 200; break;   /* LT */
        case 1: result = (a <= b) ? 101 : 201; break;  /* LE */
        case 2: result = (a > b) ? 102 : 202; break;   /* GT */
        case 3: result = (a >= b) ? 103 : 203; break;  /* GE */
        case 4: result = (a == b) ? 104 : 204; break;  /* EQ */
        case 5: result = (a != b) ? 105 : 205; break;  /* NEQ */
    }
    return result;
}

/* Generate unordered comparisons */
static volatile int cc_accumulator = 0;

int main(void) {
    double arr1[256], arr2[256];
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = nan_val;
        }
        if (i % 11 == 0) {
            arr2[i] = nan_val;
        }
        if (i % 13 == 0) {
            arr1[i] = inf_val;
        }
        if (i % 17 == 0) {
            arr2[i] = -inf_val;
        }
    }
    
    /* Perform all floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Standard comparisons that may generate various condition codes */
        volatile int lt = (a < b) ? 1 : 0;
        volatile int le = (a <= b) ? 1 : 0;
        volatile int gt = (a > b) ? 1 : 0;
        volatile int ge = (a >= b) ? 1 : 0;
        volatile int eq = (a == b) ? 1 : 0;
        volatile int neq = (a != b) ? 1 : 0;
        
        cc_accumulator += lt + le + gt + ge + eq + neq;
        
        /* Force conditional move generation */
        cc_accumulator += conditional_move_based_on_float(a, b, i % 6);
    }
    
    /* Directly trigger condition code printing with inline assembly */
    
    /* UNORDERED - comparisons involving NaN */
    DIRECT_CC_PRINT(UNORDERED);
    
    /* ORDERED - normal comparisons */
    DIRECT_CC_PRINT(ORDERED);
    
    /* UNEQ - unordered or equal */
    {
        double nan1 = __builtin_nan("");
        double nan2 = __builtin_nan("");
        FORCE_CC_PRINTING(UNEQ, nan1, nan2);
    }
    
    /* UNGE - unordered or greater than or equal */
    {
        double a = 10.5;
        double b = __builtin_nan("");
        FORCE_CC_PRINTING(UNGE, a, b);
    }
    
    /* UNGT - unordered or greater than */
    {
        double a = __builtin_nan("");
        double b = 5.5;
        FORCE_CC_PRINTING(UNGT, a, b);
    }
    
    /* UNLE - unordered or less than or equal */
    {
        double a = 3.14;
        double b = __builtin_nan("");
        FORCE_CC_PRINTING(UNLE, a, b);
    }
    
    /* UNLT - unordered or less than */
    {
        double a = __builtin_nan("");
        double b = 2.71;
        FORCE_CC_PRINTING(UNLT, a, b);
    }
    
    /* LTGT - less than or greater than (ordered and not equal) */
    {
        double a = 10.0;
        double b = 20.0;
        FORCE_CC_PRINTING(LTGT, a, b);
    }
    
    /* Additional complex floating-point expressions */
    {
        volatile double x = 0.0;
        volatile double y = 0.0;
        
        /* Generate division by zero for INF */
        y = 1.0 / x;  /* +INF */
        x = -1.0 / x; /* -INF */
        
        /* Comparisons with INF should be ordered */
        volatile int inf_cmp = (y > x) ? 1 : 0;
        cc_accumulator += inf_cmp;
        
        /* NaN comparisons should be unordered */
        volatile double nan = __builtin_nan("");
        volatile int nan_cmp = (nan == nan) ? 1 : 0;  /* Always false */
        cc_accumulator += nan_cmp;
    }
    
    /* Use SSE intrinsics for additional condition code patterns */
    {
        __m128d v1 = _mm_set_pd(1.0, __builtin_nan(""));
        __m128d v2 = _mm_set_pd(__builtin_nan(""), 2.0);
        
        int mask = _mm_movemask_pd(_mm_cmpord_pd(v1, v2));
        cc_accumulator += mask;
        
        mask = _mm_movemask_pd(_mm_cmpunord_pd(v1, v2));
        cc_accumulator += mask;
    }
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Prevent optimization of the entire computation */
    volatile int* volatile_ptr = &cc_accumulator;
    __asm__ volatile ("" : : "r"(*volatile_ptr));
    
    return cc_accumulator != 0 ? 0 : 1;
}
