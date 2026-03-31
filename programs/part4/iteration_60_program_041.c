#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int global_result = 0;

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED/UNLE */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED/UNGE */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED/UNGT */
    results[idx++] = (inf < nan) ? 1 : 0;      /* UNORDERED/UNLT */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);  /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);         /* UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);      /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);    /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNGE */
    
    /* 3. Ordered comparisons that may still use condition codes */
    results[idx++] = __builtin_isless(one, inf);         /* LT */
    results[idx++] = __builtin_isgreater(inf, one);      /* GT */
    results[idx++] = __builtin_islessequal(zero, one);   /* LE */
    results[idx++] = __builtin_isgreaterequal(one, zero);/* GE */
    
    /* 4. Complex expressions with NaN propagation */
    volatile double expr1 = (inf - inf) / zero;  /* NaN */
    volatile double expr2 = zero / zero;         /* NaN */
    results[idx++] = (expr1 == expr2) ? 1 : 0;   /* UNORDERED/UNEQ */
    results[idx++] = (expr1 != expr1) ? 1 : 0;   /* UNORDERED */
    results[idx++] = (expr1 < expr2) ? 1 : 0;    /* UNORDERED/UNLT */
    results[idx++] = (expr1 > expr2) ? 1 : 0;    /* UNORDERED/UNGT */
    
    /* 5. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    results[idx++] = (f_nan == (float)nan) ? 1 : 0;
    results[idx++] = ((double)ld_inf > nan) ? 1 : 0;
    
    /* 6. FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, one, inf);
    results[idx++] = (fma_result == fma_result) ? 1 : 0;
    results[idx++] = (fma_result < inf) ? 1 : 0;
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i] << (i % 16);
    }
    global_result = checksum;
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), 2.0f, 1.0f, __builtin_nanf("")};
    
    /* Vector comparisons generate multiple condition codes */
    v4sf cmp_gt = vec_a > vec_b;    /* UNGT/UNLT/GT/LT combinations */
    v4sf cmp_lt = vec_a < vec_b;
    v4sf cmp_eq = vec_a == vec_b;   /* UNORDERED/UNEQ combinations */
    v4sf cmp_ne = vec_a != vec_b;   /* ORDERED/LTGT combinations */
    
    /* Extract comparison masks - forces scalar code generation */
    int mask_gt, mask_lt, mask_eq, mask_ne;
    
    /* Use inline assembly for movmskps if available */
    #ifdef __SSE__
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask_gt)
        : "x"(cmp_gt)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask_lt)
        : "x"(cmp_lt)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask_eq)
        : "x"(cmp_eq)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask_ne)
        : "x"(cmp_ne)
    );
    #else
    /* Fallback: store to memory and check */
    float store[4];
    memcpy(store, &cmp_gt, sizeof(cmp_gt));
    mask_gt = (store[0] != 0.0f) | ((store[1] != 0.0f) << 1) |
              ((store[2] != 0.0f) << 2) | ((store[3] != 0.0f) << 3);
    #endif
    
    global_result ^= mask_gt ^ mask_lt ^ mask_eq ^ mask_ne;
}

/* Test function with inline assembly for explicit condition codes */
void test_asm_condition_codes(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result1, result2, result3, result4;
    
    /* Inline assembly with ucomisd and condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result2)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "x"(c), "x"(d)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result4)
        : "x"(c), "x"(d)
        : "al", "cc"
    );
    
    global_result ^= result1 ^ result2 ^ result3 ^ result4;
}

/* Control flow based on unordered comparisons */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, 1.0, 2.0, -inf};
    
    int switch_result = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Complex condition that uses multiple comparison types */
        int condition = 
            (__builtin_isunordered(values[i], nan) ? 1 : 0) |
            (__builtin_isless(values[i], inf) ? 2 : 0) |
            (__builtin_isgreater(values[i], -inf) ? 4 : 0) |
            (values[i] == values[i] ? 8 : 0);
        
        /* Switch on combined comparison results */
        switch (condition) {
            case 0:  /* UNORDERED and nothing else */
                switch_result += 1;
                break;
            case 1:  /* Only UNORDERED */
                switch_result += 2;
                break;
            case 2:  /* Only UNLT */
                switch_result += 3;
                break;
            case 4:  /* Only UNGT */
                switch_result += 4;
                break;
            case 8:  /* ORDERED/EQ */
                switch_result += 5;
                break;
            case 3:  /* UNORDERED + UNLT */
                switch_result += 6;
                break;
            case 5:  /* UNORDERED + UNGT */
                switch_result += 7;
                break;
            case 10: /* UNLT + EQ (impossible but compiler doesn't know) */
                switch_result += 8;
                break;
            default:
                switch_result += 9;
                break;
        }
    }
    
    global_result ^= switch_result;
}

int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    /* Run all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_asm_condition_codes();
    test_control_flow();
    
    printf("Result checksum: %d\n", global_result);
    return global_result != 0 ? 0 : 1;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
