#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Function to use comparison results in non-trivial ways */
void use_result(int cond) {
    checksum ^= cond;
}

/* Function with complex control flow based on comparisons */
int complex_control(double a, double b) {
    int result = 0;
    
    /* Direct unordered comparisons */
    if (__builtin_isunordered(a, b)) result |= 1;
    if (__builtin_islessgreater(a, b)) result |= 2;
    if (__builtin_isless(a, b)) result |= 4;
    if (__builtin_isgreater(a, b)) result |= 8;
    if (__builtin_islessequal(a, b)) result |= 16;
    if (__builtin_isgreaterequal(a, b)) result |= 32;
    
    /* Ternary operators with builtins */
    result += __builtin_isunordered(a, b) ? 64 : 128;
    result += !__builtin_islessgreater(a, b) ? 256 : 512;
    
    return result;
}

int main() {
#ifdef __x86_64__ || __i386__
    printf("Running x86-specific unordered comparison tests\n");
    
    /* Volatile to prevent optimization */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    /* Array to store comparison results */
    int results[32];
    int idx = 0;
    
    /* ====== 1. Explicit unordered floating-point comparisons ====== */
    printf("Testing explicit unordered comparisons...\n");
    
    /* These should generate various condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED/LTGT */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED/LTGT */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED/UNLE */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED/UNGE */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (nan != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/NE */
    results[idx++] = (inf == inf) ? 1 : 0;     /* ORDERED/EQ */
    
    /* Comparisons with normal numbers */
    results[idx++] = (nan < one) ? 1 : 0;      /* UNORDERED/LTGT */
    results[idx++] = (one > nan) ? 1 : 0;      /* UNORDERED/LTGT */
    results[idx++] = (nan == one) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (one != nan) ? 1 : 0;     /* ORDERED/NE */
    
    /* ====== 2. Built-in unordered comparison functions ====== */
    printf("Testing built-in unordered comparison functions...\n");
    
    /* Direct built-in calls */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, inf);      /* ORDERED */
    results[idx++] = __builtin_isunordered(one, zero);     /* ORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);    /* UNORDERED/LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);    /* UNORDERED/LTGT */
    results[idx++] = __builtin_islessgreater(one, zero);   /* ORDERED/LTGT */
    
    results[idx++] = __builtin_isless(nan, inf);           /* UNORDERED/UNLT */
    results[idx++] = __builtin_isless(inf, nan);           /* UNORDERED/UNGT */
    results[idx++] = __builtin_isless(zero, one);          /* ORDERED/LT */
    
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNORDERED/UNGT */
    results[idx++] = __builtin_isgreater(inf, nan);        /* UNORDERED/UNLT */
    results[idx++] = __builtin_isgreater(one, zero);       /* ORDERED/GT */
    
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNORDERED/UNLE */
    results[idx++] = __builtin_islessequal(inf, nan);      /* UNORDERED/UNGE */
    results[idx++] = __builtin_islessequal(zero, one);     /* ORDERED/LE */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNORDERED/UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, nan);   /* UNORDERED/UNLE */
    results[idx++] = __builtin_isgreaterequal(one, zero);  /* ORDERED/GE */
    
    /* ====== 3. Vector comparisons with GCC extensions ====== */
    printf("Testing vector comparisons...\n");
    
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(),
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_one = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_zero = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons - these may generate multiple condition checks */
    v4sf cmp1 = vec_nan > vec_inf;     /* UNORDERED/UNGT */
    v4sf cmp2 = vec_nan < vec_inf;     /* UNORDERED/UNLT */
    v4sf cmp3 = vec_nan == vec_nan;    /* UNORDERED/UNEQ */
    v4sf cmp4 = vec_one > vec_zero;    /* ORDERED/GT */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3, mask4;
    
    /* Use inline assembly to extract masks (x86-specific) */
    #ifdef __SSE__
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask1)
        : "x"(cmp1)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask2)
        : "x"(cmp2)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask3)
        : "x"(cmp3)
    );
    asm volatile (
        "movmskps %1, %0\n\t"
        : "=r"(mask4)
        : "x"(cmp4)
    );
    
    results[idx++] = mask1;
    results[idx++] = mask2;
    results[idx++] = mask3;
    results[idx++] = mask4;
    #endif
    
    /* ====== 4. Mixed-type comparisons and arithmetic ====== */
    printf("Testing mixed-type comparisons...\n");
    
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    /* Mixed type comparisons */
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;
    results[idx++] = ((double)ld_nan > nan) ? 1 : 0;
    results[idx++] = (f_inf == (float)ld_inf) ? 1 : 0;
    
    /* Arithmetic that produces NaN */
    volatile double div_zero = one / zero;          /* inf */
    volatile double inf_minus_inf = inf - inf;      /* nan */
    volatile double nan_plus_one = nan + one;       /* nan */
    
    results[idx++] = (div_zero == inf) ? 1 : 0;     /* ORDERED/EQ */
    results[idx++] = (inf_minus_inf == inf_minus_inf) ? 1 : 0;  /* UNORDERED/UNEQ */
    results[idx++] = (nan_plus_one > zero) ? 1 : 0; /* UNORDERED/UNGT */
    
    /* Use FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(nan, one, zero);
    results[idx++] = (fma_result == fma_result) ? 1 : 0;  /* UNORDERED/UNEQ */
    #endif
    
    /* ====== 5. Inline assembly with explicit condition codes ====== */
    printf("Testing inline assembly...\n");
    
    /* Direct x86 floating-point compare instructions */
    int setp_result = 0, setnp_result = 0;
    double a = nan;
    double b = inf;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(setp_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0\n\t"
        : "=r"(setnp_result)
        : "x"(b), "x"(a)  /* swapped operands */
        : "cc"
    );
    
    results[idx++] = setp_result;   /* Should be 1 for unordered */
    results[idx++] = setnp_result;  /* Should be 0 for unordered */
    
    /* fucomi instruction */
    int fucomi_result = 0;
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %0\n\t"
        : "=r"(fucomi_result)
        : "t"(one), "f"(zero)
        : "cc"
    );
    
    results[idx++] = fucomi_result;  /* Should be 1 (one > zero) */
    
    /* ====== 6. Control flow driven by unordered results ====== */
    printf("Testing control flow...\n");
    
    /* Switch statement based on comparison combinations */
    for (int i = 0; i < 4; i++) {
        double x = (i == 0) ? nan : (i == 1) ? inf : (i == 2) ? zero : one;
        double y = (i == 3) ? nan : (i == 2) ? inf : (i == 1) ? zero : one;
        
        int cmp_flags = 0;
        cmp_flags |= __builtin_isunordered(x, y) ? 1 : 0;
        cmp_flags |= __builtin_isless(x, y) ? 2 : 0;
        cmp_flags |= __builtin_isgreater(x, y) ? 4 : 0;
        cmp_flags |= __builtin_isequal(x, y) ? 8 : 0;
        
        /* Switch on the combination of flags */
        switch (cmp_flags) {
            case 0:  /* ORDERED, !LT, !GT, !EQ (impossible for floats) */
                results[idx++] = 100;
                break;
            case 1:  /* UNORDERED */
                results[idx++] = 101;
                break;
            case 2:  /* ORDERED, LT */
                results[idx++] = 102;
                break;
            case 3:  /* UNORDERED with LT flag (UNLT) */
                results[idx++] = 103;
                break;
            case 4:  /* ORDERED, GT */
                results[idx++] = 104;
                break;
            case 5:  /* UNORDERED with GT flag (UNGT) */
                results[idx++] = 105;
                break;
            case 6:  /* ORDERED, LT|GT (LTGT) */
                results[idx++] = 106;
                break;
            case 7:  /* UNORDERED with LT|GT */
                results[idx++] = 107;
                break;
            case 8:  /* ORDERED, EQ */
                results[idx++] = 108;
                break;
            case 9:  /* UNORDERED with EQ (UNEQ) */
                results[idx++] = 109;
                break;
            default:
                results[idx++] = 110;
                break;
        }
    }
    
    /* Complex function calls */
    results[idx++] = complex_control(nan, inf);
    results[idx++] = complex_control(inf, nan);
    results[idx++] = complex_control(zero, one);
    results[idx++] = complex_control(nan, nan);
    
    /* ====== Final checksum calculation ====== */
    printf("Calculating checksum...\n");
    
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
        checksum += results[i];
    }
    
    /* Use results to affect control flow */
    if (checksum > 0) {
        printf("Checksum: %d\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Force use of all volatile variables */
    volatile double dummy = nan + inf + zero + one + neg_one + f_nan + f_inf;
    (void)dummy;
    
    return checksum != 0 ? 0 : 1;
    
#else
    /* Non-x86 fallback */
    printf("Not an x86 target, skipping unordered comparison tests\n");
    return 0;
#endif
}
