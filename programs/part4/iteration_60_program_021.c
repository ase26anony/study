#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static void use_result(int result) {
    volatile int sink = result;
    (void)sink;
}

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
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED/UNLT path */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED/UNGT path */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED/UNLE path */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED/UNGE path */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ path */
    results[idx++] = (nan != nan) ? 1 : 0;     /* ORDERED/LTGT path */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/NE path */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNGE */
    
    /* 3. Ordered comparisons that may still use condition codes */
    results[idx++] = __builtin_isless(one, inf);           /* LT */
    results[idx++] = __builtin_isgreater(inf, one);        /* GT */
    results[idx++] = __builtin_islessequal(one, one);      /* LE/EQ */
    results[idx++] = __builtin_isgreaterequal(inf, inf);   /* GE/EQ */
    
    /* 4. Complex expressions with arithmetic */
    volatile double nan2 = zero / zero;                    /* Create NaN */
    volatile double inf_minus_inf = inf - inf;             /* Creates NaN */
    
    results[idx++] = (nan2 < inf_minus_inf) ? 1 : 0;       /* UNORDERED */
    results[idx++] = (inf_minus_inf == nan2) ? 1 : 0;      /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != nan2) ? 1 : 0;      /* ORDERED/LTGT */
    
    /* 5. Mixed type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;         /* UNORDERED/UNLT */
    results[idx++] = (ld_nan == ld_nan) ? 1 : 0;           /* UNORDERED/UNEQ */
    
    /* 6. Vector comparisons using GCC extensions */
    v4sf vec_a = {nan, inf, 1.0f, 2.0f};
    v4sf vec_b = {inf, nan, 1.0f, 3.0f};
    v4sf vec_cmp = vec_a > vec_b;                         /* Vector comparison */
    
    /* Extract comparison results - forces scalar code gen */
    float vec_results[4];
    memcpy(vec_results, &vec_cmp, sizeof(vec_results));
    results[idx++] = vec_results[0] != 0.0f ? 1 : 0;       /* UNORDERED/UNGT */
    results[idx++] = vec_results[1] != 0.0f ? 1 : 0;       /* UNORDERED/UNGT */
    results[idx++] = vec_results[2] != 0.0f ? 1 : 0;       /* EQ */
    results[idx++] = vec_results[3] != 0.0f ? 1 : 0;       /* LT */
    
    /* 7. Control flow based on unordered results */
    int switch_val = 0;
    if (__builtin_isunordered(nan, inf)) switch_val |= 1;     /* UNORDERED */
    if (__builtin_islessgreater(one, nan)) switch_val |= 2;   /* LTGT */
    if (__builtin_isless(nan, one)) switch_val |= 4;          /* UNLT */
    if (__builtin_isgreater(inf, nan)) switch_val |= 8;       /* UNGT */
    
    switch (switch_val) {
        case 0: results[idx++] = 0; break;
        case 1: results[idx++] = 1; break;  /* UNORDERED only */
        case 2: results[idx++] = 2; break;  /* LTGT only */
        case 3: results[idx++] = 3; break;  /* UNORDERED + LTGT */
        case 4: results[idx++] = 4; break;  /* UNLT only */
        case 5: results[idx++] = 5; break;  /* UNORDERED + UNLT */
        default: results[idx++] = 9; break;
    }
    
    /* 8. Inline assembly with explicit condition codes */
    #ifdef __x86_64__
    double a = nan;
    double b = inf;
    int asm_result = 0;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(asm_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    results[idx++] = asm_result;  /* Should be 1 for UNORDERED */
    
    /* fucomi with conditional move */
    asm_result = 0;
    asm volatile (
        "fucomi %2, %1\n\t"
        "setne %0\n\t"
        : "=r"(asm_result)
        : "t"(a), "u"(b)
        : "cc"
    );
    results[idx++] = asm_result;  /* NE condition */
    #endif
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
        use_result(results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
}

/* Additional test with FMA and math functions */
void test_math_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA operations that could produce NaN */
    double fma_result = __builtin_fma(nan, inf, 1.0);
    double fma_result2 = __builtin_fma(inf, 0.0, -inf);  /* inf*0 - inf = NaN */
    
    int results[4];
    results[0] = (fma_result == fma_result2) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[1] = (fma_result < fma_result2) ? 1 : 0;      /* UNORDERED/UNLT */
    results[2] = (fma_result > fma_result2) ? 1 : 0;      /* UNORDERED/UNGT */
    results[3] = __builtin_islessgreater(fma_result, fma_result2); /* LTGT */
    
    for (int i = 0; i < 4; i++) {
        use_result(results[i]);
    }
}

int main(void) {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    test_unordered_comparisons();
    test_math_functions();
    
    /* Additional tests in loops to increase coverage */
    volatile double values[] = {
        __builtin_nan(""),
        __builtin_inf(),
        -__builtin_inf(),
        0.0,
        1.0,
        -1.0
    };
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            /* Exercise all condition codes in loop */
            int r1 = __builtin_isunordered(values[i], values[j]);
            int r2 = __builtin_isless(values[i], values[j]);
            int r3 = __builtin_isgreater(values[i], values[j]);
            int r4 = __builtin_islessequal(values[i], values[j]);
            int r5 = __builtin_isgreaterequal(values[i], values[j]);
            int r6 = __builtin_islessgreater(values[i], values[j]);
            
            use_result(r1 + r2 + r3 + r4 + r5 + r6);
        }
    }
    
    printf("Tests completed.\n");
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
