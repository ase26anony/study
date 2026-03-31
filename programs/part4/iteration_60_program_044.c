#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

void test_scalar_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[32];
    int idx = 0;
    
    /* Direct unordered comparisons using operators */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (nan > zero) ? 1 : 0;     /* UNORDERED */
    results[idx++] = (zero <= nan) ? 1 : 0;    /* UNORDERED */
    results[idx++] = (nan >= one) ? 1 : 0;     /* UNORDERED */
    
    /* Comparisons that might produce ORDERED */
    results[idx++] = (inf > zero) ? 1 : 0;     /* ORDERED */
    results[idx++] = (zero < inf) ? 1 : 0;     /* ORDERED */
    results[idx++] = (neg_inf < zero) ? 1 : 0; /* ORDERED */
    
    /* Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);   /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf); /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);        /* UNORDERED/UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);     /* UNORDERED/UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);   /* UNORDERED/UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);/* UNORDERED/UNGE */
    
    /* Ordered comparisons using built-ins */
    results[idx++] = __builtin_isless(inf, zero);       /* ORDERED */
    results[idx++] = __builtin_isgreater(inf, zero);    /* ORDERED */
    
    /* Complex expressions with NaN propagation */
    volatile double nan_div = zero / zero;
    volatile double inf_minus_inf = inf - inf;
    
    results[idx++] = (nan_div == nan_div) ? 1 : 0;      /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != inf_minus_inf) ? 1 : 0; /* UNORDERED/LTGT */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;      /* UNORDERED */
    results[idx++] = (ld_nan == ld_nan) ? 1 : 0;        /* UNORDERED/UNEQ */
    
    /* Arithmetic that could produce NaN */
    volatile double complex_expr = (inf * zero) + (nan / one);
    results[idx++] = (complex_expr > zero) ? 1 : 0;     /* UNORDERED */
    
    /* FMA with NaN inputs */
    results[idx++] = __builtin_fma(nan, one, zero) == nan ? 1 : 0; /* UNORDERED/UNEQ */
    
    /* Store results in checksum */
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i] << (i % 16);
    }
    
    use(&results);
}

void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -__builtin_inff()};
    v4sf vec_b = {__builtin_inff(), __builtin_nanf(""), 2.0f, 0.0f};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that generate condition codes */
    v4sf cmp_gt = vec_a > vec_b;   /* May generate UNGT/UNLT/UNORDERED */
    v4sf cmp_lt = vec_a < vec_b;   /* May generate UNLT/UNGT/UNORDERED */
    v4sf cmp_eq = vec_a == vec_b;  /* May generate UNEQ/UNORDERED */
    v4sf cmp_ne = vec_a != vec_b;  /* May generate LTGT/UNORDERED */
    
    /* Extract comparison masks */
    int mask_gt = __builtin_ia32_movmskps(cmp_gt);
    int mask_lt = __builtin_ia32_movmskps(cmp_lt);
    int mask_eq = __builtin_ia32_movmskps(cmp_eq);
    int mask_ne = __builtin_ia32_movmskps(cmp_ne);
    
    /* Store to memory to force scalar comparisons */
    float mem[4];
    memcpy(mem, &cmp_gt, sizeof(cmp_gt));
    
    /* Check individual elements */
    int vec_results = 0;
    for (int i = 0; i < 4; i++) {
        if (mem[i] != 0.0f) {
            vec_results |= 1 << i;
        }
    }
    
    checksum ^= mask_gt ^ mask_lt ^ mask_eq ^ mask_ne ^ vec_results;
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    
    v2df cmp_d = vec_da > vec_db;
    double dmem[2];
    memcpy(dmem, &cmp_d, sizeof(cmp_d));
    
    checksum ^= ((dmem[0] != 0.0) ? 1 : 0) ^ ((dmem[1] != 0.0) ? 2 : 0);
}

void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with explicit condition codes */
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
        : "x"(a), "x"(c)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "x"(b), "x"(c)
        : "al", "cc"
    );
    
    /* fucomi instruction */
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result4)
        : "t"(d), "u"(c)
        : "al", "cc"
    );
    
    checksum ^= result1 ^ result2 ^ result3 ^ result4;
}

void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0, -inf};
    
    int switch_result = 0;
    
    /* Switch based on comparison results */
    for (int i = 0; i < 5; i++) {
        int code = 0;
        
        if (__builtin_isunordered(vals[i], nan)) code |= 1;
        if (__builtin_islessgreater(vals[i], inf)) code |= 2;
        if (__builtin_isless(vals[i], 0.0)) code |= 4;
        if (__builtin_isgreaterequal(vals[i], 0.0)) code |= 8;
        
        switch (code) {
            case 0:  switch_result += 1; break;  /* ORDERED, EQ, GE, LE */
            case 1:  switch_result += 2; break;  /* UNORDERED */
            case 2:  switch_result += 3; break;  /* LTGT */
            case 4:  switch_result += 4; break;  /* UNLT/ORDERED LT */
            case 8:  switch_result += 5; break;  /* UNGE/ORDERED GE */
            case 5:  switch_result += 6; break;  /* UNORDERED + UNLT */
            case 9:  switch_result += 7; break;  /* UNORDERED + UNGE */
            case 10: switch_result += 8; break;  /* LTGT + UNGE */
            default: switch_result += 9; break;
        }
    }
    
    /* Loop controlled by unordered comparison */
    int loop_count = 0;
    volatile double x = nan;
    while (__builtin_isunordered(x, inf) && loop_count < 10) {
        loop_count++;
        x = (loop_count % 2 == 0) ? __builtin_nan("") : __builtin_inf();
    }
    
    checksum ^= switch_result ^ loop_count;
}

int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    test_scalar_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return 0;
}

#else /* Non-x86 target */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}
#endif
