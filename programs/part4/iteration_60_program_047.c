#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int sink;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Helper to update checksum */
static void update_checksum(int value) {
    checksum = (checksum << 1) ^ (uint32_t)value;
}

/* Test 1: Direct unordered comparisons with operators */
static void test_direct_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf) update_checksum(1);  /* UNORDERED/UNLT? */
    if (nan == nan) update_checksum(2); /* UNORDERED/UNEQ? */
    if (inf != nan) update_checksum(3); /* ORDERED/LTGT? */
    if (nan <= one) update_checksum(4); /* UNORDERED/UNLE? */
    if (nan >= zero) update_checksum(5); /* UNORDERED/UNGE? */
    if (nan > inf) update_checksum(6);  /* UNORDERED/UNGT? */
    
    /* More complex expressions */
    volatile double inf_minus_inf = inf - inf;
    if (inf_minus_inf == zero) update_checksum(7); /* UNORDERED/UNEQ? */
    if (zero / zero == nan) update_checksum(8);    /* UNORDERED/UNEQ? */
    
    /* Ordered comparisons */
    if (one < inf) update_checksum(9);   /* ORDERED/LT */
    if (inf > one) update_checksum(10);  /* ORDERED/GT */
}

/* Test 2: Built-in unordered comparison functions */
static void test_builtin_comparisons(void) {
    volatile float fnan = __builtin_nanf("");
    volatile float finf = __builtin_inff();
    volatile float fone = 1.0f;
    
    /* Each built-in maps to specific condition codes */
    if (__builtin_isunordered(fnan, finf)) update_checksum(11); /* UNORDERED */
    if (__builtin_islessgreater(fnan, fone)) update_checksum(12); /* LTGT */
    if (__builtin_isless(fnan, finf)) update_checksum(13); /* UNLT */
    if (__builtin_isgreater(fnan, fone)) update_checksum(14); /* UNGT */
    if (__builtin_islessequal(fnan, finf)) update_checksum(15); /* UNLE */
    if (__builtin_isgreaterequal(fnan, fone)) update_checksum(16); /* UNGE */
    
    /* Ordered built-ins */
    if (!__builtin_isunordered(fone, finf)) update_checksum(17); /* ORDERED */
    
    /* Combined in ternary expressions */
    int result = __builtin_isunordered(fnan, fnan) ? 
                 __builtin_islessgreater(fone, finf) : 
                 __builtin_isless(finf, fone);
    update_checksum(18 + result);
}

/* Test 3: Vector comparisons with GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), 2.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), 2.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons generate multiple condition checks */
    v4sf cmp_result = vec_a > vec_b;  /* Should generate UNGT/UNLE etc. */
    
    /* Extract comparison mask - forces scalarization */
    int mask;
#if defined(__SSE__)
    mask = __builtin_ia32_movmskps(cmp_result);
#else
    /* Fallback: store to memory and check */
    float stored[4];
    memcpy(stored, &cmp_result, sizeof(stored));
    mask = (stored[0] != 0) | ((stored[1] != 0) << 1) | 
           ((stored[2] != 0) << 2) | ((stored[3] != 0) << 3);
#endif
    update_checksum(20 + mask);
    
    /* Double precision vector */
    v2df vec_d = {__builtin_nan(""), __builtin_inf()};
    v2df vec_e = {__builtin_inf(), __builtin_nan("")};
    v2df vec_f = vec_d < vec_e;  /* UNLT */
    
    double stored_d[2];
    memcpy(stored_d, &vec_f, sizeof(stored_d));
    update_checksum(30 + (stored_d[0] != 0) + (stored_d[1] != 0));
}

/* Test 4: Mixed-type comparisons and arithmetic */
static void test_mixed_type_comparisons(void) {
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    volatile double d_val = 3.14159;
    volatile float f_val = 2.71828f;
    
    /* Cross-type comparisons */
    if (ld_nan < d_val) update_checksum(40); /* UNLT */
    if (f_val > ld_nan) update_checksum(41); /* UNGT */
    
    /* Arithmetic producing NaN */
    volatile double nan_prod = ld_inf * 0.0;
    if (nan_prod == nan_prod) update_checksum(42); /* UNORDERED/UNEQ? */
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(__builtin_nan(""), 2.0, 3.0);
    if (fma_result <= d_val) update_checksum(43); /* UNLE */
    
    /* Complex expression */
    volatile int complex_cmp = (ld_nan < ld_inf) && (f_val != d_val) || 
                               (nan_prod == __builtin_nan(""));
    update_checksum(44 + complex_cmp);
}

/* Test 5: Inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    int result;
    
    /* Using ucomisd which sets unordered flags */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    update_checksum(50 + result);  /* UNORDERED test */
    
    /* Compare with normal number */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "al", "cc"
    );
    update_checksum(60 + result);  /* LTGT test */
    
    /* fucomi instruction */
    volatile long double ld_a = __builtin_nanl("");
    volatile long double ld_b = __builtin_infl();
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)"
        : 
        : "t"(ld_a), "u"(ld_b)
        : "cc"
    );
    update_checksum(70);
}

/* Test 6: Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0};
    
    int state = 0;
    for (int i = 0; i < 4; i++) {
        /* Switch on comparison results */
        int cmp_result = 0;
        if (__builtin_isunordered(vals[i], nan)) cmp_result |= 1;
        if (__builtin_islessgreater(vals[i], inf)) cmp_result |= 2;
        if (!__builtin_isunordered(vals[i], 0.0)) cmp_result |= 4;
        
        switch (cmp_result) {
            case 0: /* UNORDERED + not LTGT + not ORDERED */
                state += 1;
                break;
            case 1: /* UNORDERED only */
                state += 2;
                break;
            case 2: /* LTGT only */
                state += 3;
                break;
            case 3: /* UNORDERED + LTGT */
                state += 4;
                break;
            case 4: /* ORDERED only */
                state += 5;
                break;
            case 5: /* UNORDERED + ORDERED (impossible?) */
                state += 6;
                break;
            case 6: /* LTGT + ORDERED */
                state += 7;
                break;
            case 7: /* All three */
                state += 8;
                break;
        }
    }
    update_checksum(80 + state);
}

int main(void) {
    printf("Starting unordered comparison tests...\n");
    
    /* Run all tests */
    test_direct_comparisons();
    test_builtin_comparisons();
    test_vector_comparisons();
    test_mixed_type_comparisons();
    test_inline_asm();
    test_control_flow();
    
    /* Use checksum to prevent optimization */
    sink = checksum;
    printf("Checksum: %u\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}

#else /* Non-x86 target */
int main(void) {
    printf("This test is for x86/x86-64 targets only.\n");
    return 0;
}
#endif
