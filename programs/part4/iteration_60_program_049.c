/* test_unordered_comparisons.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Feature detection for x86 */
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

/* Test 1: Direct unordered comparisons with operators */
static void test_direct_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[16];
    int idx = 0;
    
    /* These should generate various condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED? */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED? */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED? */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED? */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ? */
    results[idx++] = (nan != nan) ? 1 : 0;     /* ORDERED/LTGT? */
    results[idx++] = (inf == inf) ? 1 : 0;     /* ORDERED/EQ */
    results[idx++] = (inf != inf) ? 1 : 0;     /* UNORDERED? */
    
    /* Comparisons with normal numbers */
    results[idx++] = (nan < one) ? 1 : 0;
    results[idx++] = (nan > one) ? 1 : 0;
    results[idx++] = (one < nan) ? 1 : 0;
    results[idx++] = (one > nan) ? 1 : 0;
    
    /* Infinity comparisons */
    results[idx++] = (inf > neg_inf) ? 1 : 0;
    results[idx++] = (neg_inf < inf) ? 1 : 0;
    
    /* Division that could produce NaN */
    volatile double div_result = zero / zero;
    results[idx++] = (div_result == div_result) ? 1 : 0;
    results[idx++] = (div_result != div_result) ? 1 : 0;
    
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
}

/* Test 2: Built-in unordered comparison functions */
static void test_builtin_comparisons(void) {
    volatile float fnan = __builtin_nanf("");
    volatile float finf = __builtin_inff();
    volatile float fval = 3.14f;
    
    volatile double dnan = __builtin_nan("");
    volatile double dinf = __builtin_inf();
    volatile double dval = 2.71828;
    
    long double lnan = __builtin_nanl("");
    long double linf = __builtin_infl();
    long double lval = 1.618034L;
    
    int results[32];
    int idx = 0;
    
    /* __builtin_isunordered - should generate UNORDERED */
    results[idx++] = __builtin_isunordered(fnan, finf);
    results[idx++] = __builtin_isunordered(dnan, dval);
    results[idx++] = __builtin_isunordered(lnan, lval);
    
    /* __builtin_islessgreater - should generate LTGT */
    results[idx++] = __builtin_islessgreater(fnan, finf);
    results[idx++] = __builtin_islessgreater(dval, dnan);
    results[idx++] = __builtin_islessgreater(lval, lnan);
    
    /* __builtin_isless - should generate LT or UNLT */
    results[idx++] = __builtin_isless(fnan, fval);
    results[idx++] = __builtin_isless(dval, dinf);
    results[idx++] = __builtin_isless(lval, linf);
    
    /* __builtin_isgreater - should generate GT or UNGT */
    results[idx++] = __builtin_isgreater(finf, fval);
    results[idx++] = __builtin_isgreater(dinf, dnan);
    results[idx++] = __builtin_isgreater(linf, lval);
    
    /* __builtin_islessequal - should generate LE or UNLE */
    results[idx++] = __builtin_islessequal(fnan, fval);
    results[idx++] = __builtin_islessequal(dval, dval);
    results[idx++] = __builtin_islessequal(lval, linf);
    
    /* __builtin_isgreaterequal - should generate GE or UNGE */
    results[idx++] = __builtin_isgreaterequal(finf, fval);
    results[idx++] = __builtin_isgreaterequal(dval, dnan);
    results[idx++] = __builtin_isgreaterequal(linf, lval);
    
    /* Nested in ternary operators */
    results[idx++] = __builtin_isunordered(fnan, finf) ? 
                     __builtin_isless(fval, finf) : 
                     __builtin_isgreater(finf, fval);
    
    results[idx++] = __builtin_islessgreater(dnan, dval) ?
                     __builtin_islessequal(dval, dinf) :
                     __builtin_isgreaterequal(dinf, dval);
    
    /* Complex expressions */
    results[idx++] = __builtin_isunordered(lnan, lval) && 
                     __builtin_isless(lval, linf);
    
    results[idx++] = __builtin_islessgreater(lnan, linf) ||
                     __builtin_isgreaterequal(linf, lval);
    
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
}

/* Test 3: Vector comparisons with GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(),
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_val = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_neg = (v4sf){-1.0f, -2.0f, -3.0f, -4.0f};
    
    /* These vector comparisons may generate multiple condition checks */
    v4sf cmp_result;
    int mask;
    
    /* Greater than - may generate UNGT/GT */
    cmp_result = vec_val > vec_nan;
    /* Extract comparison mask */
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Less than - may generate UNLT/LT */
    cmp_result = vec_nan < vec_val;
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Equal - may generate UNEQ/EQ */
    cmp_result = vec_nan == vec_nan;
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Not equal - may generate LTGT/NE */
    cmp_result = vec_val != vec_nan;
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Greater or equal - may generate UNGE/GE */
    cmp_result = vec_inf >= vec_val;
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Less or equal - may generate UNLE/LE */
    cmp_result = vec_neg <= vec_val;
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    
    /* Double precision vectors */
    v2df dvec_nan = (v2df){__builtin_nan(""), __builtin_nan("")};
    v2df dvec_inf = (v2df){__builtin_inf(), __builtin_inf()};
    v2df dvec_val = (v2df){1.0, 2.0};
    
    v2df dcmp_result = dvec_val > dvec_nan;
    /* Use inline asm to force use of condition codes */
    asm volatile ("" : "+x" (dcmp_result));
    
    dcmp_result = dvec_nan == dvec_nan;
    asm volatile ("" : "+x" (dcmp_result));
}

/* Test 4: Mixed-type comparisons and arithmetic */
static void test_mixed_type_comparisons(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    int results[20];
    int idx = 0;
    
    /* Mixed type comparisons */
    results[idx++] = (f_nan < d_inf) ? 1 : 0;
    results[idx++] = (d_nan > f_inf) ? 1 : 0;
    results[idx++] = (ld_nan == d_nan) ? 1 : 0;
    results[idx++] = (f_inf != ld_nan) ? 1 : 0;
    
    /* Arithmetic that produces NaN */
    volatile double a = 0.0;
    volatile double b = 0.0;
    volatile double nan1 = a / b;
    volatile double nan2 = __builtin_inf() - __builtin_inf();
    
    results[idx++] = (nan1 < nan2) ? 1 : 0;
    results[idx++] = (nan1 > nan2) ? 1 : 0;
    results[idx++] = (nan1 == nan2) ? 1 : 0;
    results[idx++] = (nan1 != nan2) ? 1 : 0;
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(d_nan, 2.0, 3.0);
    results[idx++] = (fma_result == fma_result) ? 1 : 0;
    results[idx++] = __builtin_isunordered(fma_result, d_nan);
    
    /* Complex expression with multiple comparisons */
    volatile int complex_result = 
        (__builtin_isunordered(f_nan, d_inf) || 
         __builtin_islessgreater(ld_nan, f_inf)) &&
        (__builtin_isless(f_inf, ld_inf) ||
         __builtin_isgreaterequal(d_nan, f_nan));
    
    results[idx++] = complex_result;
    
    /* Switch statement driven by comparison results */
    volatile int switch_val = 0;
    switch_val |= __builtin_isunordered(f_nan, d_nan) ? 1 : 0;
    switch_val |= __builtin_islessgreater(f_inf, ld_nan) ? 2 : 0;
    switch_val |= __builtin_isless(f_nan, f_inf) ? 4 : 0;
    switch_val |= __builtin_isgreater(d_inf, d_nan) ? 8 : 0;
    
    switch (switch_val & 0xF) {
        case 0: results[idx++] = 0; break;
        case 1: results[idx++] = 1; break;
        case 2: results[idx++] = 2; break;
        case 3: results[idx++] = 3; break;
        case 4: results[idx++] = 4; break;
        case 5: results[idx++] = 5; break;
        case 6: results[idx++] = 6; break;
        case 7: results[idx++] = 7; break;
        case 8: results[idx++] = 8; break;
        case 9: results[idx++] = 9; break;
        default: results[idx++] = 10; break;
    }
    
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
}

/* Test 5: Inline assembly with explicit condition codes */
static void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    int result_u, result_p, result_np;
    int result_eq, result_neq, result_lt, result_le, result_gt, result_ge;
    
    /* Using ucomisd instruction which sets condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r" (result_p)
        : "x" (a), "x" (b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0\n\t"
        : "=r" (result_np)
        : "x" (a), "x" (c)
        : "cc"
    );
    
    /* Testing various condition code mnemonics */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0\n\t"
        : "=r" (result_eq)
        : "x" (c), "x" (d)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0\n\t"
        : "=r" (result_neq)
        : "x" (a), "x" (b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0\n\t"
        : "=r" (result_lt)
        : "x" (c), "x" (d)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0\n\t"
        : "=r" (result_le)
        : "x" (c), "x" (d)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0\n\t"
        : "=r" (result_gt)
        : "x" (d), "x" (c)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0\n\t"
        : "=r" (result_ge)
        : "x" (d), "x" (c)
        : "cc"
    );
    
    /* fucomi instruction (x87) */
    volatile long double ld_a = __builtin_nanl("");
    volatile long double ld_b = __builtin_infl();
    
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setp %0\n\t"
        : "=r" (result_u)
        : "t" (ld_a), "u" (ld_b)
        : "cc"
    );
    
    update_checksum(result_p);
    update_checksum(result_np);
    update_checksum(result_eq);
    update_checksum(result_neq);
    update_checksum(result_lt);
    update_checksum(result_le);
    update_checksum(result_gt);
    update_checksum(result_ge);
    update_checksum(result_u);
}

/* Test 6: Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double val = 42.0;
    
    int loop_counter = 0;
    
    /* Loop condition based on unordered comparison */
    for (int i = 0; i < 10; i++) {
        if (__builtin_isunordered(nan, inf)) {
            loop_counter++;
        }
        if (__builtin_islessgreater(val, nan)) {
            loop_counter--;
        }
    }
    update_checksum(loop_counter);
    
    /* Nested if-else chain */
    volatile int branch_result = 0;
    if (__builtin_isunordered(nan, val)) {
        branch_result = 1;
    } else if (__builtin_isless(inf, val)) {
        branch_result = 2;
    } else if (__builtin_isgreater(val, nan)) {
        branch_result = 3;
    } else if (__builtin_islessequal(nan, inf)) {
        branch_result = 4;
    } else if (__builtin_isgreaterequal(inf, val)) {
        branch_result = 5;
    }
    update_checksum(branch_result);
    
    /* Function calls conditional on comparison results */
    auto void dummy_func(int x) {
        global_counter += x;
    }
    
    (__builtin_isunordered(nan, inf)) ? dummy_func(1) : dummy_func(2);
    (__builtin_islessgreater(val, nan)) ? dummy_func(3) : dummy_func(4);
    (__builtin_isless(val, inf)) ? dummy_func(5) : dummy_func(6);
    (__builtin_isgreater(inf, val)) ? dummy_func(7) : dummy_func(8);
    
    update_checksum(global_counter);
}

int main(void) {
    printf("Testing x86 unordered floating-point comparisons...\n");
    
    /* Run all tests */
    test_direct_comparisons();
    test_builtin_comparisons();
    test_vector_comparisons();
    test_mixed_type_comparisons();
    test_inline_assembly();
    test_control_flow();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}

#else /* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
