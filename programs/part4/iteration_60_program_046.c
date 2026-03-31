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
static void update_checksum(int value) {
    checksum = (checksum << 3) ^ (checksum >> 29) ^ (uint32_t)value;
}

/* Test function for scalar unordered comparisons */
static void test_scalar_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[32];
    int idx = 0;
    
    /* Direct comparisons with NaN to trigger various condition codes */
    
    /* UNORDERED cases - comparisons involving NaN */
    results[idx++] = __builtin_isunordered(nan, inf);      /* Should be true */
    results[idx++] = __builtin_isunordered(inf, nan);      /* Should be true */
    results[idx++] = __builtin_isunordered(nan, nan);      /* Should be true */
    
    /* ORDERED cases - comparisons not involving NaN */
    results[idx++] = __builtin_isordered(inf, one);        /* Should be true */
    results[idx++] = __builtin_isordered(one, zero);       /* Should be true */
    results[idx++] = __builtin_isordered(neg_inf, inf);    /* Should be true */
    
    /* UNEQ (unordered or equal) */
    results[idx++] = (nan == nan) ? 1 : 0;                 /* NaN != NaN, but UNEQ might be used */
    results[idx++] = __builtin_isunordered(nan, nan) || (one == one);
    
    /* UNGE (unordered or greater or equal) */
    results[idx++] = (nan >= inf) ? 1 : 0;                 /* Triggers UNGE */
    results[idx++] = __builtin_isunordered(inf, nan) || (inf >= one);
    
    /* UNGT (unordered or greater than) */
    results[idx++] = (nan > inf) ? 1 : 0;                  /* Triggers UNGT */
    results[idx++] = __builtin_isunordered(one, nan) || (inf > one);
    
    /* UNLE (unordered or less or equal) */
    results[idx++] = (nan <= inf) ? 1 : 0;                 /* Triggers UNLE */
    results[idx++] = __builtin_isunordered(nan, one) || (one <= inf);
    
    /* UNLT (unordered or less than) */
    results[idx++] = (nan < inf) ? 1 : 0;                  /* Triggers UNLT */
    results[idx++] = __builtin_isunordered(inf, nan) || (one < inf);
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    results[idx++] = __builtin_islessgreater(one, zero);   /* Should be true */
    results[idx++] = __builtin_islessgreater(inf, one);    /* Should be true */
    results[idx++] = __builtin_islessgreater(one, inf);    /* Should be false */
    
    /* Mixed operators to cover different code paths */
    results[idx++] = (one < inf) && !__builtin_isunordered(one, inf);
    results[idx++] = (inf > one) && !__builtin_isunordered(inf, one);
    results[idx++] = (one != nan) || __builtin_isunordered(one, nan);
    
    /* Complex expressions with arithmetic that might produce NaN */
    volatile double maybe_nan = inf - inf;
    results[idx++] = __builtin_isunordered(maybe_nan, zero);
    results[idx++] = (maybe_nan == maybe_nan) ? 1 : 0;
    
    /* Division that might produce infinity */
    volatile double div_result = one / zero;
    results[idx++] = __builtin_isunordered(div_result, nan);
    results[idx++] = (div_result > one) ? 1 : 0;
    
    /* Update checksum with all results */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
        global_counter += results[i];
    }
}

/* Test function for vector comparisons */
static void test_vector_comparisons(void) {
    v4sf vec_a = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf vec_b = {2.0f, 2.0f, 3.0f, __builtin_nanf("")};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Various vector comparisons that might generate condition codes */
    vec_c = vec_a > vec_b;    /* Element-wise greater than */
    int mask1 = __builtin_ia32_movmskps((__v4sf)vec_c);
    update_checksum(mask1);
    
    vec_c = vec_a < vec_b;    /* Element-wise less than */
    int mask2 = __builtin_ia32_movmskps((__v4sf)vec_c);
    update_checksum(mask2);
    
    vec_c = vec_a == vec_b;   /* Element-wise equality */
    int mask3 = __builtin_ia32_movmskps((__v4sf)vec_c);
    update_checksum(mask3);
    
    vec_c = vec_a != vec_b;   /* Element-wise inequality */
    int mask4 = __builtin_ia32_movmskps((__v4sf)vec_c);
    update_checksum(mask4);
    
    /* Double precision vector */
    v2df vec_d = {__builtin_nan(""), 2.0};
    v2df vec_e = {1.0, __builtin_nan("")};
    v2df vec_f = vec_d > vec_e;
    
    /* Extract results to force code generation */
    double arr[2];
    memcpy(arr, &vec_f, sizeof(arr));
    update_checksum((int)(arr[0] != 0.0));
    update_checksum((int)(arr[1] != 0.0));
}

/* Test function with inline assembly */
static void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = 1.0;
    volatile double c = __builtin_inf();
    
    int result_p, result_a, result_b;
    
    /* Inline assembly with ucomisd and condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(result_p)
        : "x"(a), "x"(b)
        : "cc"
    );
    update_checksum(result_p);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0\n\t"
        : "=r"(result_a)
        : "x"(c), "x"(b)
        : "cc"
    );
    update_checksum(result_a);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0\n\t"
        : "=r"(result_b)
        : "x"(b), "x"(c)
        : "cc"
    );
    update_checksum(result_b);
    
    /* Test with fucomi instruction */
    int result_unordered;
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "setp %0\n\t"
        : "=r"(result_unordered)
        : "t"(a), "u"(b)
        : "cc"
    );
    update_checksum(result_unordered);
}

/* Control flow based on unordered comparisons */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {1.0, 2.0, nan, inf, -inf, 0.0};
    
    int switch_value = 0;
    
    /* Build a switch value based on comparison results */
    if (__builtin_isunordered(values[0], values[2])) switch_value |= 1;
    if (__builtin_islessgreater(values[0], values[1])) switch_value |= 2;
    if (values[3] > values[0]) switch_value |= 4;
    if (__builtin_isunordered(values[2], values[2])) switch_value |= 8;
    if (values[4] < values[0]) switch_value |= 16;
    
    /* Switch statement that depends on unordered comparison results */
    switch (switch_value & 0x1F) {
        case 0:
            update_checksum(100);
            break;
        case 1:
            update_checksum(101);  /* UNORDERED case */
            break;
        case 2:
            update_checksum(102);  /* LTGT case */
            break;
        case 4:
            update_checksum(103);  /* Normal ordered comparison */
            break;
        case 8:
            update_checksum(104);  /* UNORDERED with NaN-NaN */
            break;
        case 16:
            update_checksum(105);  /* Less than with -inf */
            break;
        default:
            update_checksum(switch_value);
            break;
    }
    
    /* Loop controlled by unordered comparison */
    int count = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (__builtin_isunordered(values[i], values[j])) {
                count++;
            }
        }
    }
    update_checksum(count);
}

/* Test with math functions that might produce NaN */
static void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA with NaN input */
    double fma_result = __builtin_fma(nan, 2.0, 3.0);
    update_checksum(__builtin_isunordered(fma_result, 0.0));
    
    /* Complex expression */
    double expr1 = (inf * 0.0) + nan;
    double expr2 = (inf - inf) * 1.0;
    
    /* Comparisons that should trigger various condition codes */
    int r1 = __builtin_isunordered(expr1, expr2);
    int r2 = __builtin_islessgreater(expr1, 0.0);
    int r3 = (expr2 == expr2) ? 1 : 0;  /* NaN comparison */
    int r4 = (expr1 > expr2) ? 1 : 0;   /* Unordered comparison */
    
    update_checksum(r1 + r2 + r3 + r4);
    
    /* Long double comparisons */
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    int r5 = __builtin_isunordered(ld_nan, ld_inf);
    int r6 = (ld_inf > 1.0L) ? 1 : 0;
    int r7 = __builtin_islessgreater(ld_nan, ld_nan);
    
    update_checksum(r5 + r6 + r7);
}

int main(void) {
    printf("Starting unordered comparison tests...\n");
    
    /* Run all test suites */
    test_scalar_unordered_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow();
    test_math_functions();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
}
#endif
