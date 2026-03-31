#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Function to use comparison results */
void use_result(int cond) {
    checksum ^= cond;
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
    
    /* 1. Direct unordered comparisons with operators */
    /* These should generate UNORDERED/ORDERED condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED likely */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT */
    results[idx++] = (nan > zero) ? 1 : 0;     /* UNORDERED */
    results[idx++] = (zero <= nan) ? 1 : 0;    /* UNORDERED */
    results[idx++] = (nan >= nan) ? 1 : 0;     /* UNORDERED/UNGE */
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to specific condition codes */
    results[idx++] = __builtin_isunordered(nan, inf);   /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, nan); /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);        /* UNLT */
    results[idx++] = __builtin_isgreater(inf, nan);     /* UNGT */
    results[idx++] = __builtin_islessequal(zero, nan);  /* UNLE */
    results[idx++] = __builtin_isgreaterequal(inf, nan);/* UNGE */
    
    /* 3. Complex expressions that may produce NaN */
    volatile double nan_prod = (inf * zero);  /* 0 * inf = NaN */
    volatile double nan_div = (inf / inf);    /* inf/inf = NaN */
    volatile double nan_sub = (inf - inf);    /* inf-inf = NaN */
    
    results[idx++] = (nan_prod == one) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (nan_div > zero) ? 1 : 0;      /* UNORDERED */
    results[idx++] = (nan_sub <= one) ? 1 : 0;      /* UNORDERED */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;          /* UNORDERED */
    results[idx++] = ((double)ld_nan == nan) ? 1 : 0;       /* UNORDERED/UNEQ */
    
    /* 5. Control flow based on unordered comparisons */
    for (int i = 0; i < 4; i++) {
        volatile double a = (i & 1) ? nan : inf;
        volatile double b = (i & 2) ? nan : zero;
        
        if (__builtin_isunordered(a, b)) {
            results[idx++] = 100 + i;  /* UNORDERED */
        } else if (__builtin_islessgreater(a, b)) {
            results[idx++] = 200 + i;  /* LTGT */
        } else if (a == b) {
            results[idx++] = 300 + i;  /* UNEQ */
        }
    }
    
    /* Use all results to prevent optimization */
    for (int i = 0; i < idx; i++) {
        use_result(results[i]);
    }
}

/* Vector comparisons using GCC extensions */
#ifdef __SSE__
void test_vector_comparisons(void) {
    typedef float v4sf __attribute__((vector_size(16)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v4sf vec_nan = (v4sf){__builtin_nanf(""), __builtin_nanf(""), 
                          __builtin_nanf(""), __builtin_nanf("")};
    v4sf vec_inf = (v4sf){__builtin_inff(), __builtin_inff(),
                          __builtin_inff(), __builtin_inff()};
    v4sf vec_zero = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    v4sf vec_one = (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
    
    /* Vector comparisons - may generate multiple condition checks */
    v4sf cmp1 = vec_nan < vec_inf;    /* UNORDERED */
    v4sf cmp2 = vec_nan == vec_nan;   /* UNORDERED/UNEQ */
    v4sf cmp3 = vec_inf > vec_zero;   /* ORDERED/UNGT */
    v4sf cmp4 = vec_one <= vec_nan;   /* UNORDERED */
    
    /* Extract comparison masks */
    int mask1, mask2, mask3, mask4;
    
    /* Using SSE intrinsics if available */
    #ifdef __SSE2__
        mask1 = __builtin_ia32_movmskps((__v4sf)cmp1);
        mask2 = __builtin_ia32_movmskps((__v4sf)cmp2);
        mask3 = __builtin_ia32_movmskps((__v4sf)cmp3);
        mask4 = __builtin_ia32_movmskps((__v4sf)cmp4);
    #else
        /* Fallback: store to memory and check */
        float store1[4], store2[4], store3[4], store4[4];
        memcpy(store1, &cmp1, sizeof(cmp1));
        memcpy(store2, &cmp2, sizeof(cmp2));
        memcpy(store3, &cmp3, sizeof(cmp3));
        memcpy(store4, &cmp4, sizeof(cmp4));
        
        mask1 = (store1[0] != 0) | ((store1[1] != 0) << 1) |
                ((store1[2] != 0) << 2) | ((store1[3] != 0) << 3);
        mask2 = (store2[0] != 0) | ((store2[1] != 0) << 1) |
                ((store2[2] != 0) << 2) | ((store2[3] != 0) << 3);
        mask3 = (store3[0] != 0) | ((store3[1] != 0) << 1) |
                ((store3[2] != 0) << 2) | ((store3[3] != 0) << 3);
        mask4 = (store4[0] != 0) | ((store4[1] != 0) << 1) |
                ((store4[2] != 0) << 2) | ((store4[3] != 0) << 3);
    #endif
    
    use_result(mask1);
    use_result(mask2);
    use_result(mask3);
    use_result(mask4);
}
#endif

/* Inline assembly with explicit condition codes */
#ifdef __x86_64__
void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1, result2, result3, result4;
    
    /* Using ucomisd with explicit condition code checks */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result1)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "x" (b), "x" (c)
        : "al", "cc"
    );
    
    /* fucomi instruction */
    asm volatile (
        "fucomi %%st(1), %%st\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result3)
        : "t" (c), "u" (d)
        : "al", "cc"
    );
    
    use_result(result1);
    use_result(result2);
    use_result(result3);
}
#endif

/* Switch statement driven by comparison results */
void test_switch_comparisons(void) {
    volatile double vals[] = {
        __builtin_nan(""),
        __builtin_inf(),
        -__builtin_inf(),
        0.0,
        1.0,
        -1.0
    };
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            volatile double a = vals[i];
            volatile double b = vals[j];
            int condition = 0;
            
            /* Determine condition code */
            if (__builtin_isunordered(a, b)) {
                condition = 0;  /* UNORDERED */
            } else if (a == b) {
                condition = 1;  /* UNEQ */
            } else if (a < b) {
                condition = 2;  /* UNLT */
            } else if (a > b) {
                condition = 3;  /* UNGT */
            } else if (a <= b) {
                condition = 4;  /* UNLE */
            } else if (a >= b) {
                condition = 5;  /* UNGE */
            } else {
                condition = 6;  /* LTGT */
            }
            
            /* Switch on condition - forces compiler to handle all cases */
            switch (condition) {
                case 0: use_result(1000 + i*10 + j); break; /* UNORDERED */
                case 1: use_result(2000 + i*10 + j); break; /* UNEQ */
                case 2: use_result(3000 + i*10 + j); break; /* UNLT */
                case 3: use_result(4000 + i*10 + j); break; /* UNGT */
                case 4: use_result(5000 + i*10 + j); break; /* UNLE */
                case 5: use_result(6000 + i*10 + j); break; /* UNGE */
                case 6: use_result(7000 + i*10 + j); break; /* LTGT */
            }
        }
    }
}

int main(void) {
    /* Only run x86-specific tests on x86 */
    #if defined(__i386__) || defined(__x86_64__)
    
    printf("Running x86 floating-point comparison tests...\n");
    
    /* Test 1: Basic unordered comparisons */
    test_unordered_comparisons();
    
    /* Test 2: Vector comparisons (SSE required) */
    #ifdef __SSE__
    test_vector_comparisons();
    #endif
    
    /* Test 3: Inline assembly (x86-64 specific) */
    #ifdef __x86_64__
    test_inline_asm();
    #endif
    
    /* Test 4: Switch-based comparisons */
    test_switch_comparisons();
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    #else
    /* Non-x86 fallback */
    printf("This test is for x86/x86-64 architectures only.\n");
    printf("Compile with: gcc -O2 -march=x86-64 -ffast-math -fno-trapping-math -S -o test.s test.c\n");
    #endif
    
    return 0;
}
