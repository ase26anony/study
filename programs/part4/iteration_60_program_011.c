/* test_fp_conditions.c - Trigger x86 floating-point unordered condition codes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile int global_result = 0;

/* Function to accumulate results with side effects */
static void accumulate_result(int cond) {
    global_result ^= (cond << 4) | 0x1;
}

/* Test 1: Direct unordered comparisons with operators */
static void test_direct_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[16];
    int idx = 0;
    
    /* These should generate various condition codes */
    results[idx++] = (nan < inf);      /* UNORDERED? */
    results[idx++] = (nan == nan);     /* UNORDERED/UNEQ? */
    results[idx++] = (inf != nan);     /* ORDERED/LTGT? */
    results[idx++] = (nan <= zero);    /* UNORDERED/UNLE? */
    results[idx++] = (nan >= neg_inf); /* UNORDERED/UNGE? */
    results[idx++] = (zero > nan);     /* UNORDERED/UNLT? */
    results[idx++] = (inf < nan);      /* UNORDERED/UNGT? */
    
    /* Arithmetic that produces NaN */
    volatile double nan2 = inf - inf;
    volatile double nan3 = zero / zero;
    
    results[idx++] = (nan2 == nan3);   /* UNORDERED/UNEQ? */
    results[idx++] = (nan2 != nan3);   /* UNORDERED/LTGT? */
    results[idx++] = (nan2 < 1.0);     /* UNORDERED */
    results[idx++] = (nan3 > -1.0);    /* UNORDERED */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    
    results[idx++] = (f_nan == (float)nan);    /* UNORDERED/UNEQ? */
    results[idx++] = (ld_inf != (long double)nan); /* ORDERED/LTGT? */
    
    /* Control flow based on unordered results */
    if (nan == nan) {  /* Always false with NaN, but compiler may not know */
        accumulate_result(1);
    }
    
    if (inf > nan) {   /* Unordered comparison */
        accumulate_result(2);
    }
    
    /* Ternary operator with unordered comparison */
    int val = (nan != zero) ? 3 : 4;
    accumulate_result(val);
    
    /* Store results to prevent elimination */
    for (int i = 0; i < idx; i++) {
        global_result += results[i];
    }
}

/* Test 2: Built-in unordered comparison functions */
static void test_builtin_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double num = 3.14159;
    
    /* These built-ins map directly to condition codes */
    int r1 = __builtin_isunordered(nan, inf);      /* UNORDERED */
    int r2 = __builtin_islessgreater(nan, num);    /* LTGT */
    int r3 = __builtin_isless(nan, inf);           /* UNLT */
    int r4 = __builtin_isgreater(inf, nan);        /* UNGT */
    int r5 = __builtin_islessequal(num, nan);      /* UNLE */
    int r6 = __builtin_isgreaterequal(nan, num);   /* UNGE */
    int r7 = __builtin_isunordered(nan, nan);      /* UNORDERED */
    
    /* Complex expressions with built-ins */
    int r8 = __builtin_islessgreater(inf, -inf) && __builtin_isunordered(nan, 0.0);
    int r9 = __builtin_isless(nan, nan) || __builtin_isgreaterequal(inf, inf);
    
    /* Nested built-in calls */
    if (__builtin_isunordered(nan, inf)) {
        if (__builtin_islessgreater(inf, nan)) {
            accumulate_result(5);
        }
    }
    
    /* Switch based on comparison results */
    int code = (r1 << 0) | (r2 << 1) | (r3 << 2) | (r4 << 3);
    switch (code & 0xF) {
        case 0: accumulate_result(10); break;
        case 1: accumulate_result(11); break;
        case 2: accumulate_result(12); break;
        case 3: accumulate_result(13); break;
        case 4: accumulate_result(14); break;
        case 5: accumulate_result(15); break;
        default: accumulate_result(16); break;
    }
    
    global_result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
}

/* Test 3: Vector comparisons with GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = { __builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f };
    v4sf vec_b = { 2.0f, __builtin_nanf(""), 3.0f, __builtin_nanf("") };
    v4sf vec_c = { __builtin_inff(), __builtin_inff(), __builtin_nanf(""), 0.0f };
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp1 = vec_a > vec_b;    /* Element-wise UNGT/UNORDERED */
    v4sf cmp2 = vec_a == vec_c;   /* Element-wise UNEQ/UNORDERED */
    v4sf cmp3 = vec_b <= vec_a;   /* Element-wise UNLE/UNORDERED */
    v4sf cmp4 = vec_c != vec_b;   /* Element-wise LTGT/UNORDERED */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3, mask4;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp1);
    mask2 = __builtin_ia32_movmskps(cmp2);
    mask3 = __builtin_ia32_movmskps(cmp3);
    mask4 = __builtin_ia32_movmskps(cmp4);
    #else
    /* Fallback: store to memory and check */
    float m1[4], m2[4], m3[4], m4[4];
    memcpy(m1, &cmp1, 16);
    memcpy(m2, &cmp2, 16);
    memcpy(m3, &cmp3, 16);
    memcpy(m4, &cmp4, 16);
    mask1 = (m1[0] != 0) | ((m1[1] != 0) << 1) | ((m1[2] != 0) << 2) | ((m1[3] != 0) << 3);
    mask2 = (m2[0] != 0) | ((m2[1] != 0) << 1) | ((m2[2] != 0) << 2) | ((m2[3] != 0) << 3);
    mask3 = (m3[0] != 0) | ((m3[1] != 0) << 1) | ((m3[2] != 0) << 2) | ((m3[3] != 0) << 3);
    mask4 = (m4[0] != 0) | ((m4[1] != 0) << 1) | ((m4[2] != 0) << 2) | ((m4[3] != 0) << 3);
    #endif
    
    /* Double precision vectors */
    v2df vec_d1 = { __builtin_nan(""), __builtin_inf() };
    v2df vec_d2 = { __builtin_inf(), __builtin_nan("") };
    v2df cmp_d = vec_d1 < vec_d2;  /* UNLT/UNORDERED */
    
    double dmask[2];
    memcpy(dmask, &cmp_d, 16);
    int dmask_int = (dmask[0] != 0) | ((dmask[1] != 0) << 1);
    
    global_result += mask1 + mask2 + mask3 + mask4 + dmask_int;
}

/* Test 4: Mixed-type and arithmetic comparisons */
static void test_mixed_arithmetic(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_inf = __builtin_inf();
    volatile long double ld_zero = 0.0L;
    
    /* Arithmetic that produces NaN */
    volatile double d1 = d_inf / ld_zero;  /* May produce NaN */
    volatile float f1 = f_nan * 2.0f;
    volatile long double ld1 = (long double)d_inf - (long double)d_inf;
    
    /* Comparisons after arithmetic */
    int r1 = (d1 == f1);                    /* UNORDERED/UNEQ */
    int r2 = (ld1 > ld_zero);               /* UNORDERED/UNGT */
    int r3 = (f_nan <= (float)d1);          /* UNORDERED/UNLE */
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_nan = __builtin_fma(__builtin_nan(""), 2.0, 3.0);
    int r4 = (fma_nan < 0.0);               /* UNORDERED/UNLT */
    global_result += r4;
    #endif
    
    /* Complex expression */
    int r5 = ((f_nan + d_inf) != (ld_zero * ld1)) && (d1 == d1);
    
    /* Loop with unordered comparison condition */
    for (int i = 0; i < 3; i++) {
        volatile double x = (i == 0) ? __builtin_nan("") : 
                           (i == 1) ? __builtin_inf() : 0.0;
        volatile double y = (i == 2) ? __builtin_nan("") : 1.0;
        
        if (x > y) {  /* May be UNORDERED/UNGT */
            accumulate_result(20 + i);
        }
    }
    
    global_result += r1 + r2 + r3 + r5;
}

/* Test 5: Inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    
    int result_p, result_z, result_c;
    
    /* ucomisd with explicit condition code checking */
    #ifdef __x86_64__
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(result_p)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setz %0\n\t"
        : "=r"(result_z)
        : "x"(b), "x"(c)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setc %0\n\t"
        : "=r"(result_c)
        : "x"(c), "x"(a)
        : "cc"
    );
    
    /* fucomi instruction */
    int fucomi_result;
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %0\n\t"
        : "=r"(fucomi_result)
        : "t"(a), "t"(b)
        : "cc"
    );
    
    global_result += result_p + result_z + result_c + fucomi_result;
    #endif
}

/* Test 6: Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 0.0, -inf, 1.0};
    
    int checksum = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch on comparison results */
            int cmp_result = 0;
            if (__builtin_isunordered(vals[i], vals[j])) cmp_result |= 1;
            if (vals[i] == vals[j]) cmp_result |= 2;      /* UNEQ */
            if (vals[i] != vals[j]) cmp_result |= 4;      /* LTGT */
            if (vals[i] < vals[j]) cmp_result |= 8;       /* UNLT */
            if (vals[i] > vals[j]) cmp_result |= 16;      /* UNGT */
            if (vals[i] <= vals[j]) cmp_result |= 32;     /* UNLE */
            if (vals[i] >= vals[j]) cmp_result |= 64;     /* UNGE */
            
            switch (cmp_result & 0x7) {
                case 0: checksum += 1; break;  /* ORDERED and equal? */
                case 1: checksum += 2; break;  /* UNORDERED */
                case 2: checksum += 3; break;  /* UNEQ */
                case 4: checksum += 4; break;  /* LTGT */
                case 5: checksum += 5; break;  /* UNORDERED | LTGT */
                default: checksum += 6; break;
            }
        }
    }
    
    /* Conditional function calls */
    void (*funcs[6])(void) = {
        test_direct_comparisons,
        test_builtin_comparisons,
        test_vector_comparisons,
        test_mixed_arithmetic,
        test_inline_asm,
        test_control_flow
    };
    
    /* Call functions based on unordered comparison results */
    if (nan != nan) {  /* Always true with NaN != NaN */
        funcs[0]();
    }
    
    if (!__builtin_isunordered(inf, inf)) {  /* inf ordered with itself */
        funcs[1]();
    }
    
    global_result += checksum;
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    test_direct_comparisons();
    test_builtin_comparisons();
    test_vector_comparisons();
    test_mixed_arithmetic();
    test_inline_asm();
    test_control_flow();
    
    /* Final result to prevent optimization */
    printf("Result checksum: %d\n", global_result);
    
    return global_result != 0 ? 0 : 1;
}

#else /* Non-x86 target */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}
#endif
