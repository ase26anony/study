/* test_unordered_comparisons.c */
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

/* Function to force condition code generation */
void __attribute__((noinline)) use_result(int cond) {
    checksum ^= cond;
}

/* Main test function */
int main(void) {
    /* Volatile to prevent optimization */
    volatile double nan_d = __builtin_nan("");
    volatile double inf_d = __builtin_inf();
    volatile double zero_d = 0.0;
    volatile double one_d = 1.0;
    volatile double neg_inf_d = -__builtin_inf();
    
    volatile float nan_f = __builtin_nanf("");
    volatile float inf_f = __builtin_inff();
    volatile float zero_f = 0.0f;
    
    volatile long double nan_ld = __builtin_nanl("");
    volatile long double inf_ld = __builtin_infl();
    
    int results[32];
    int idx = 0;
    
    /* ============================================
       1. Direct unordered comparisons with operators
       ============================================ */
    
    /* UNORDERED cases: comparisons involving NaN */
    results[idx++] = (nan_d < inf_d);      /* Should be false (unordered) */
    results[idx++] = (nan_d > inf_d);      /* Should be false (unordered) */
    results[idx++] = (nan_d <= inf_d);     /* Should be false (unordered) */
    results[idx++] = (nan_d >= inf_d);     /* Should be false (unordered) */
    results[idx++] = (nan_d == nan_d);     /* Should be false (unordered) */
    results[idx++] = (nan_d != nan_d);     /* Should be true (unordered) */
    
    /* ORDERED cases: normal comparisons */
    results[idx++] = (inf_d > zero_d);     /* Should be true (ordered) */
    results[idx++] = (neg_inf_d < zero_d); /* Should be true (ordered) */
    results[idx++] = (one_d == one_d);     /* Should be true (ordered) */
    
    /* ============================================
       2. Built-in unordered comparison functions
       ============================================ */
    
    /* __builtin_isunordered - maps to UNORDERED */
    results[idx++] = __builtin_isunordered(nan_d, inf_d);  /* Should be 1 */
    results[idx++] = __builtin_isunordered(inf_d, zero_d); /* Should be 0 */
    
    /* __builtin_islessgreater - maps to LTGT */
    results[idx++] = __builtin_islessgreater(nan_d, inf_d); /* Should be 0 */
    results[idx++] = __builtin_islessgreater(inf_d, zero_d); /* Should be 1 */
    
    /* __builtin_isless - maps to UNLT or LT */
    results[idx++] = __builtin_isless(nan_d, inf_d);        /* Should be 0 */
    results[idx++] = __builtin_isless(zero_d, inf_d);       /* Should be 1 */
    
    /* __builtin_isgreater - maps to UNGT or GT */
    results[idx++] = __builtin_isgreater(inf_d, nan_d);     /* Should be 0 */
    results[idx++] = __builtin_isgreater(inf_d, zero_d);    /* Should be 1 */
    
    /* __builtin_islessequal - maps to UNLE or LE */
    results[idx++] = __builtin_islessequal(nan_d, inf_d);   /* Should be 0 */
    results[idx++] = __builtin_islessequal(zero_d, inf_d);  /* Should be 1 */
    results[idx++] = __builtin_islessequal(inf_d, inf_d);   /* Should be 1 */
    
    /* __builtin_isgreaterequal - maps to UNGE or GE */
    results[idx++] = __builtin_isgreaterequal(nan_d, inf_d); /* Should be 0 */
    results[idx++] = __builtin_isgreaterequal(inf_d, zero_d); /* Should be 1 */
    results[idx++] = __builtin_isgreaterequal(inf_d, inf_d);  /* Should be 1 */
    
    /* ============================================
       3. Complex expressions with mixed types
       ============================================ */
    
    /* Arithmetic that can produce NaN */
    volatile double nan_prod = zero_d / zero_d;
    volatile double inf_minus_inf = inf_d - inf_d;
    
    /* UNEQ: unordered or equal */
    results[idx++] = (nan_prod == nan_prod) || __builtin_isunordered(nan_prod, nan_prod);
    
    /* UNGE: unordered or greater or equal (nlt) */
    results[idx++] = (inf_minus_inf >= zero_d) || __builtin_isunordered(inf_minus_inf, zero_d);
    
    /* UNGT: unordered or greater than (nle) */
    results[idx++] = (inf_d > nan_d) || __builtin_isunordered(inf_d, nan_d);
    
    /* UNLE: unordered or less or equal (ule) */
    results[idx++] = (nan_d <= inf_d) || __builtin_isunordered(nan_d, inf_d);
    
    /* UNLT: unordered or less than (ult) */
    results[idx++] = (neg_inf_d < nan_d) || __builtin_isunordered(neg_inf_d, nan_d);
    
    /* ============================================
       4. Vector comparisons with GCC extensions
       ============================================ */
    {
        v4sf vec_a = {nan_f, inf_f, zero_f, 1.0f};
        v4sf vec_b = {inf_f, nan_f, 1.0f, zero_f};
        v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec_d = {4.0f, 3.0f, 2.0f, 1.0f};
        
        /* Vector comparisons generate multiple condition checks */
        v4sf cmp_result = vec_a > vec_b;  /* Should trigger UNGT/GT */
        v4sf cmp_result2 = vec_c < vec_d; /* Should trigger UNLT/LT */
        v4sf cmp_result3 = vec_a == vec_b; /* Should trigger UNEQ/EQ */
        
        /* Extract comparison masks */
        int mask1, mask2, mask3;
        
        /* Use inline assembly to extract mask (x86-specific) */
        #ifdef __SSE__
        asm volatile ("movmskps %1, %0" : "=r"(mask1) : "x"(cmp_result));
        asm volatile ("movmskps %1, %0" : "=r"(mask2) : "x"(cmp_result2));
        asm volatile ("movmskps %1, %0" : "=r"(mask3) : "x"(cmp_result3));
        #else
        /* Fallback: store to memory and check */
        float temp[4];
        memcpy(temp, &cmp_result, sizeof(cmp_result));
        mask1 = (temp[0] != 0.0f) | ((temp[1] != 0.0f) << 1) | 
                ((temp[2] != 0.0f) << 2) | ((temp[3] != 0.0f) << 3);
        #endif
        
        results[idx++] = mask1;
        results[idx++] = mask2;
        results[idx++] = mask3;
    }
    
    /* ============================================
       5. Inline assembly with explicit condition codes
       ============================================ */
    {
        double a = nan_d;
        double b = inf_d;
        int result_u, result_p, result_c;
        
        /* ucomisd sets flags: ZF=equal, PF=unordered, CF=less */
        asm volatile (
            "ucomisd %2, %1\n\t"
            "setp %0\n\t"
            : "=r"(result_p)
            : "x"(a), "x"(b)
            : "cc"
        );
        
        asm volatile (
            "ucomisd %2, %1\n\t"
            "setc %0\n\t"
            : "=r"(result_c)
            : "x"(b), "x"(a)  /* swapped for different comparison */
            : "cc"
        );
        
        asm volatile (
            "ucomisd %2, %1\n\t"
            "sete %0\n\t"
            : "=r"(result_u)
            : "x"(zero_d), "x"(zero_d)
            : "cc"
        );
        
        results[idx++] = result_p;  /* UNORDERED test */
        results[idx++] = result_c;  /* UNLT/UNLE test */
        results[idx++] = result_u;  /* ORDERED/EQ test */
    }
    
    /* ============================================
       6. Control flow based on unordered results
       ============================================ */
    {
        /* Switch based on comparison results */
        int cmp_flags = 0;
        
        /* Build a bitmask of comparison results */
        if (__builtin_isunordered(nan_d, inf_d)) cmp_flags |= 1;
        if (__builtin_islessgreater(inf_d, zero_d)) cmp_flags |= 2;
        if (__builtin_isless(zero_d, inf_d)) cmp_flags |= 4;
        if (__builtin_isgreater(inf_d, nan_d)) cmp_flags |= 8;
        
        /* Switch forces generation of multiple condition codes */
        switch (cmp_flags) {
            case 0:  results[idx++] = 100; break;
            case 1:  results[idx++] = 101; break;  /* UNORDERED */
            case 2:  results[idx++] = 102; break;  /* LTGT */
            case 3:  results[idx++] = 103; break;  /* UNORDERED | LTGT */
            case 4:  results[idx++] = 104; break;  /* UNLT/LT */
            case 5:  results[idx++] = 105; break;  /* UNORDERED | UNLT */
            case 6:  results[idx++] = 106; break;  /* LTGT | UNLT */
            case 7:  results[idx++] = 107; break;  /* All three */
            default: results[idx++] = 108; break;
        }
        
        /* Loop with unordered comparison as condition */
        int count = 0;
        volatile double x = nan_d;
        volatile double y = 0.0;
        
        for (int i = 0; i < 3; i++) {
            if (__builtin_isunordered(x, y)) {
                count++;
                x = (i == 0) ? inf_d : zero_d;  /* Change value */
            }
            if (__builtin_islessgreater(x, y)) {
                count++;
            }
        }
        results[idx++] = count;
    }
    
    /* ============================================
       7. Math functions with NaN inputs
       ============================================ */
    {
        /* FMA with NaN input */
        double fma_result = __builtin_fma(nan_d, one_d, zero_d);
        results[idx++] = __builtin_isunordered(fma_result, zero_d);
        
        /* Complex expression */
        double expr1 = (nan_d * inf_d) / zero_d;
        double expr2 = (inf_d - inf_d) * one_d;
        
        /* Multiple unordered comparisons in one expression */
        int complex_cmp = (__builtin_isunordered(expr1, expr2) ? 1 : 0) |
                          (__builtin_islessgreater(expr1, zero_d) ? 2 : 0) |
                          (__builtin_isless(expr2, expr1) ? 4 : 0) |
                          (__builtin_isgreaterequal(inf_d, expr1) ? 8 : 0);
        results[idx++] = complex_cmp;
    }
    
    /* ============================================
       Combine all results into checksum
       ============================================ */
    for (int i = 0; i < idx; i++) {
        use_result(results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed (x86 mode)\n");
    
    return 0;

#else
    /* Non-x86 fallback */
    int main(void) {
        printf("This test is for x86 targets only\n");
        return 0;
    }
#endif
}
