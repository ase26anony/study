#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Helper to update checksum */
static void update_checksum(int val) {
    checksum = (checksum << 3) ^ (checksum >> 29) ^ (uint32_t)val;
}

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    /* These should generate UNORDERED/ORDERED condition codes */
    results[idx++] = (nan < inf);      /* UNORDERED case */
    results[idx++] = (nan > inf);      /* UNORDERED case */
    results[idx++] = (nan <= inf);     /* UNORDERED case */
    results[idx++] = (nan >= inf);     /* UNORDERED case */
    results[idx++] = (nan == nan);     /* UNEQ case */
    results[idx++] = (nan != nan);     /* LTGT case */
    results[idx++] = (inf != nan);     /* LTGT case */
    results[idx++] = (inf == inf);     /* EQ case but may use ordered */
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to specific condition codes */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);      /* UNORDERED */
    results[idx++] = !__builtin_isunordered(one, zero);    /* ORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isless(inf, nan);           /* UNLT (inverted) */
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNGE */
    
    /* 3. Complex expressions with arithmetic that may produce NaN */
    volatile double nan_prod = zero * inf;          /* 0 * inf = NaN */
    volatile double nan_div = inf / inf;            /* inf/inf = NaN */
    volatile double inf_minus_inf = inf - inf;      /* inf - inf = NaN */
    
    results[idx++] = (nan_prod == nan_div);         /* UNEQ */
    results[idx++] = (nan_prod != inf_minus_inf);   /* LTGT */
    results[idx++] = (nan_prod < one);              /* UNLT */
    results[idx++] = (nan_prod > neg_one);          /* UNGT */
    results[idx++] = (nan_prod <= zero);            /* UNLE */
    results[idx++] = (nan_prod >= zero);            /* UNGE */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan < f_inf);               /* UNLT (float) */
    results[idx++] = (ld_nan == ld_nan);            /* UNEQ (long double) */
    results[idx++] = (f_nan != (float)nan);         /* LTGT (mixed) */
    
    /* 5. Control flow based on unordered comparisons */
    /* Switch statement to force multiple condition code uses */
    int switch_val = 0;
    if (__builtin_isunordered(nan, inf)) switch_val |= 1;
    if (!__builtin_isunordered(one, zero)) switch_val |= 2;
    if (__builtin_islessgreater(nan, nan)) switch_val |= 4;
    
    switch (switch_val) {
        case 0:
            results[idx++] = 0;
            break;
        case 1:  /* UNORDERED true */
            results[idx++] = 1;
            break;
        case 2:  /* ORDERED true */
            results[idx++] = 2;
            break;
        case 3:  /* Both UNORDERED and ORDERED (impossible) */
            results[idx++] = 3;
            break;
        case 4:  /* LTGT true */
            results[idx++] = 4;
            break;
        default:
            results[idx++] = -1;
    }
    
    /* 6. Ternary operators with unordered comparisons */
    results[idx++] = (__builtin_isunordered(nan, inf) ? 100 : 200);
    results[idx++] = (!__builtin_isunordered(one, zero) ? 300 : 400);
    results[idx++] = (__builtin_islessgreater(nan, inf) ? 500 : 600);
    
    /* Update checksum with all results */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
        global_counter += results[i];
    }
}

/* Test function for vector comparisons */
void test_vector_comparisons(void) {
    /* Initialize vectors with NaN and normal values */
    v4sf vec_nan = {__builtin_nanf(""), 1.0f, __builtin_nanf(""), 2.0f};
    v4sf vec_inf = {__builtin_inff(), 2.0f, 3.0f, __builtin_inff()};
    v4sf vec_normal = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Perform vector comparisons - these may generate multiple condition checks */
    v4sf cmp_result1 = vec_nan > vec_inf;      /* UNGT for NaN elements */
    v4sf cmp_result2 = vec_nan < vec_normal;   /* UNLT for NaN elements */
    v4sf cmp_result3 = vec_nan == vec_nan;     /* UNEQ for NaN elements */
    v4sf cmp_result4 = vec_inf != vec_nan;     /* LTGT for NaN elements */
    
    /* Extract comparison masks to force code generation */
    int mask1, mask2, mask3, mask4;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result1);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    mask4 = __builtin_ia32_movmskps(cmp_result4);
    #else
    /* Fallback: store to memory and check */
    float temp[4];
    memcpy(temp, &cmp_result1, sizeof(cmp_result1));
    mask1 = (temp[0] != 0) | ((temp[1] != 0) << 1) | 
            ((temp[2] != 0) << 2) | ((temp[3] != 0) << 3);
    #endif
    
    update_checksum(mask1);
    update_checksum(mask2);
    update_checksum(mask3);
    update_checksum(mask4);
    global_counter += mask1 + mask2 + mask3 + mask4;
}

/* Test function with inline assembly */
void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with explicit condition codes */
    /* These force the compiler to handle the condition mnemonics */
    
    /* UNORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result2)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* UNEQ test (parity or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setpe %0"
        : "=r"(result3)
        : "x"(a), "x"(a)
        : "cc"
    );
    
    /* LTGT test (not equal and ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result4)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    update_checksum(result1);
    update_checksum(result2);
    update_checksum(result3);
    update_checksum(result4);
    global_counter += result1 + result2 + result3 + result4;
}

/* Test with FMA and other math functions */
void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* FMA operations that might produce NaN */
    double fma_result = __builtin_fma(nan, inf, zero);
    double fma_result2 = __builtin_fma(inf, zero, inf);
    
    /* Comparisons of FMA results */
    int r1 = __builtin_isunordered(fma_result, fma_result2);  /* UNORDERED */
    int r2 = __builtin_islessgreater(fma_result, zero);       /* LTGT */
    int r3 = (fma_result < fma_result2);                      /* UNLT */
    int r4 = (fma_result > zero);                             /* UNGT */
    int r5 = (fma_result <= inf);                             /* UNLE */
    int r6 = (fma_result >= -inf);                            /* UNGE */
    
    update_checksum(r1);
    update_checksum(r2);
    update_checksum(r3);
    update_checksum(r4);
    update_checksum(r5);
    update_checksum(r6);
    global_counter += r1 + r2 + r3 + r4 + r5 + r6;
}

int main(void) {
    printf("Testing x86 floating-point unordered condition codes...\n");
    
    /* Run all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_asm();
    test_math_functions();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0 || global_counter != 0) ? 0 : 1;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is for x86/x86-64 architectures only.\n");
    return 0;
}

#endif
