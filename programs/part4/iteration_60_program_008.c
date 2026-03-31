#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to force generation of specific condition codes */
void generate_unordered_comparisons(void) {
    /* Volatile to prevent optimization */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    /* Results array to store comparison outcomes */
    volatile int results[32];
    int idx = 0;
    
    /* ====== 1. Direct unordered comparisons with operators ====== */
    
    /* UNORDERED: nan < inf (always false, unordered) */
    results[idx++] = (nan < inf) ? 1 : 0;
    
    /* UNORDERED: nan > inf (always false, unordered) */
    results[idx++] = (nan > inf) ? 2 : 0;
    
    /* UNORDERED: nan <= inf (always false, unordered) */
    results[idx++] = (nan <= inf) ? 3 : 0;
    
    /* UNORDERED: nan >= inf (always false, unordered) */
    results[idx++] = (nan >= inf) ? 4 : 0;
    
    /* UNEQ: nan == nan (always false for quiet NaN, but may generate UNEQ) */
    results[idx++] = (nan == nan) ? 5 : 0;
    
    /* LTGT: inf != nan (true, generates LTGT/UNE) */
    results[idx++] = (inf != nan) ? 6 : 0;
    
    /* LTGT: nan != inf (true, generates LTGT/UNE) */
    results[idx++] = (nan != inf) ? 7 : 0;
    
    /* Complex expressions that may produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    /* UNORDERED: (inf - inf) < 1.0 */
    results[idx++] = (inf_minus_inf < one) ? 8 : 0;
    
    /* UNORDERED: (0.0/0.0) > -1.0 */
    results[idx++] = (zero_div_zero > neg_one) ? 9 : 0;
    
    /* ====== 2. Built-in unordered comparison functions ====== */
    
    /* __builtin_isunordered - generates UNORDERED condition */
    results[idx++] = __builtin_isunordered(nan, inf) ? 10 : 0;
    results[idx++] = __builtin_isunordered(inf, nan) ? 11 : 0;
    results[idx++] = __builtin_isunordered(nan, nan) ? 12 : 0;
    
    /* __builtin_islessgreater - generates LTGT condition */
    results[idx++] = __builtin_islessgreater(inf, nan) ? 13 : 0;
    results[idx++] = __builtin_islessgreater(one, neg_one) ? 14 : 0;
    
    /* __builtin_isless - may generate UNLT or LT */
    results[idx++] = __builtin_isless(nan, inf) ? 15 : 0;
    results[idx++] = __builtin_isless(neg_one, one) ? 16 : 0;
    
    /* __builtin_isgreater - may generate UNGT or GT */
    results[idx++] = __builtin_isgreater(inf, nan) ? 17 : 0;
    results[idx++] = __builtin_isgreater(one, neg_one) ? 18 : 0;
    
    /* __builtin_islessequal - may generate UNLE or LE */
    results[idx++] = __builtin_islessequal(nan, inf) ? 19 : 0;
    results[idx++] = __builtin_islessequal(neg_one, one) ? 20 : 0;
    
    /* __builtin_isgreaterequal - may generate UNGE or GE */
    results[idx++] = __builtin_isgreaterequal(inf, nan) ? 21 : 0;
    results[idx++] = __builtin_isgreaterequal(one, neg_one) ? 22 : 0;
    
    /* ====== 3. Nested built-ins for complex condition codes ====== */
    
    /* Generate UNEQ: !isunordered(x,y) && !islessgreater(x,y) */
    results[idx++] = (!__builtin_isunordered(one, one) && 
                      !__builtin_islessgreater(one, one)) ? 23 : 0;
    
    /* Generate UNGE: !isless(x,y) */
    results[idx++] = !__builtin_isless(inf, nan) ? 24 : 0;
    
    /* Generate UNGT: !islessequal(x,y) */
    results[idx++] = !__builtin_islessequal(nan, inf) ? 25 : 0;
    
    /* Generate UNLE: islessequal(x,y) || isunordered(x,y) */
    results[idx++] = (__builtin_islessequal(nan, inf) || 
                      __builtin_isunordered(nan, inf)) ? 26 : 0;
    
    /* Generate UNLT: isless(x,y) || isunordered(x,y) */
    results[idx++] = (__builtin_isless(nan, inf) || 
                      __builtin_isunordered(nan, inf)) ? 27 : 0;
    
    /* ====== 4. Vector comparisons using GCC extensions ====== */
    
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(), 
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_one = (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
    v4sf vec_neg_one = (v4sf){-1.0f, -1.0f, -1.0f, -1.0f};
    
    /* Vector comparisons generate multiple condition checks */
    v4sf vec_cmp1 = vec_nan > vec_inf;    /* UNORDERED/UNGT */
    v4sf vec_cmp2 = vec_nan < vec_inf;    /* UNORDERED/UNLT */
    v4sf vec_cmp3 = vec_one >= vec_neg_one; /* GE/UNGE */
    v4sf vec_cmp4 = vec_one <= vec_neg_one; /* LE/UNLE */
    
    /* Extract comparison masks - forces scalarization */
    int mask1, mask2, mask3, mask4;
    
    /* Use inline assembly to extract masks if available */
    #ifdef __SSE__
    asm volatile ("movmskps %1, %0" : "=r"(mask1) : "x"(vec_cmp1));
    asm volatile ("movmskps %1, %0" : "=r"(mask2) : "x"(vec_cmp2));
    asm volatile ("movmskps %1, %0" : "=r"(mask3) : "x"(vec_cmp3));
    asm volatile ("movmskps %1, %0" : "=r"(mask4) : "x"(vec_cmp4));
    #else
    /* Fallback: store to memory and check */
    float mem_cmp1[4], mem_cmp2[4], mem_cmp3[4], mem_cmp4[4];
    memcpy(mem_cmp1, &vec_cmp1, sizeof(vec_cmp1));
    memcpy(mem_cmp2, &vec_cmp2, sizeof(vec_cmp2));
    memcpy(mem_cmp3, &vec_cmp3, sizeof(vec_cmp3));
    memcpy(mem_cmp4, &vec_cmp4, sizeof(vec_cmp4));
    mask1 = (mem_cmp1[0] != 0.0f) | ((mem_cmp1[1] != 0.0f) << 1) |
            ((mem_cmp1[2] != 0.0f) << 2) | ((mem_cmp1[3] != 0.0f) << 3);
    mask2 = (mem_cmp2[0] != 0.0f) | ((mem_cmp2[1] != 0.0f) << 1) |
            ((mem_cmp2[2] != 0.0f) << 2) | ((mem_cmp2[3] != 0.0f) << 3);
    #endif
    
    results[idx++] = mask1;
    results[idx++] = mask2;
    results[idx++] = mask3;
    results[idx++] = mask4;
    
    /* ====== 5. Inline assembly with explicit condition codes ====== */
    
    /* Direct x86 floating-point compare instructions */
    double a = __builtin_nan("");
    double b = __builtin_inf();
    int result_ucomisd, result_fucomi;
    
    /* ucomisd - sets ZF, PF, CF flags */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%cl\n\t"
        "setb %%dl\n\t"
        "and $1, %%al\n\t"
        "and $1, %%cl\n\t"
        "and $1, %%dl\n\t"
        "shl $2, %%cl\n\t"
        "shl $1, %%dl\n\t"
        "or %%cl, %%al\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_ucomisd)
        : "x"(a), "x"(b)
        : "al", "cl", "dl", "cc"
    );
    
    /* fucomi - compare and set EFLAGS directly */
    asm volatile (
        "fucomi %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%cl\n\t"
        "setb %%dl\n\t"
        "and $1, %%al\n\t"
        "and $1, %%cl\n\t"
        "and $1, %%dl\n\t"
        "shl $2, %%cl\n\t"
        "shl $1, %%dl\n\t"
        "or %%cl, %%al\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_fucomi)
        : "t"(a), "u"(b)
        : "al", "cl", "dl", "cc"
    );
    
    results[idx++] = result_ucomisd;
    results[idx++] = result_fucomi;
    
    /* ====== 6. Control flow based on unordered comparisons ====== */
    
    /* Switch statement where cases depend on comparison results */
    int case_selector = 0;
    
    /* Build selector from comparison results */
    if (__builtin_isunordered(nan, inf)) case_selector |= 1;     /* UNORDERED */
    if (!__builtin_islessgreater(one, one)) case_selector |= 2;  /* UNEQ */
    if (!__builtin_isless(inf, nan)) case_selector |= 4;         /* UNGE */
    if (!__builtin_islessequal(nan, inf)) case_selector |= 8;    /* UNGT */
    if (__builtin_islessequal(nan, inf) || 
        __builtin_isunordered(nan, inf)) case_selector |= 16;    /* UNLE */
    if (__builtin_isless(nan, inf) || 
        __builtin_isunordered(nan, inf)) case_selector |= 32;    /* UNLT */
    if (__builtin_islessgreater(inf, nan)) case_selector |= 64;  /* LTGT */
    
    /* Switch that forces generation of conditional jumps */
    switch (case_selector & 0x7F) {
        case 1:  /* UNORDERED */
            results[idx++] = 100;
            break;
        case 2:  /* UNEQ */
            results[idx++] = 101;
            break;
        case 4:  /* UNGE */
            results[idx++] = 102;
            break;
        case 8:  /* UNGT */
            results[idx++] = 103;
            break;
        case 16: /* UNLE */
            results[idx++] = 104;
            break;
        case 32: /* UNLT */
            results[idx++] = 105;
            break;
        case 64: /* LTGT */
            results[idx++] = 106;
            break;
        default:
            results[idx++] = 107;
            break;
    }
    
    /* ====== 7. Mixed-type comparisons ====== */
    
    /* Compare different floating-point types */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    /* Mixed type comparisons */
    results[idx++] = (f_nan < (float)inf) ? 200 : 0;
    results[idx++] = ((double)ld_nan > nan) ? 201 : 0;
    results[idx++] = (f_inf != (float)nan) ? 202 : 0;
    results[idx++] = ((long double)inf == ld_nan) ? 203 : 0;
    
    /* Arithmetic that produces NaN, then compare */
    volatile double div_by_zero = one / zero;
    volatile double sqrt_neg = __builtin_sqrt(-one);
    
    results[idx++] = (div_by_zero < inf) ? 204 : 0;    /* UNORDERED */
    results[idx++] = (sqrt_neg > neg_inf) ? 205 : 0;   /* UNORDERED */
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_nan = __builtin_fma(nan, one, inf);
    results[idx++] = (fma_nan == fma_nan) ? 206 : 0;   /* UNEQ */
    #endif
    
    /* Update checksum to prevent dead code elimination */
    for (int i = 0; i < idx && i < 32; i++) {
        checksum ^= results[i];
    }
}

/* Main function with x86-specific code */
int main(void) {
    #if defined(__x86_64__) || defined(__i386__)
    generate_unordered_comparisons();
    printf("Checksum: %d\n", checksum);
    return 0;
    #else
    /* Non-x86 fallback */
    printf("This test is for x86/x86-64 only.\n");
    return 1;
    #endif
}

#else
/* Non-x86 main */
int main(void) {
    printf("This test requires x86/x86-64 architecture.\n");
    return 1;
}
#endif
