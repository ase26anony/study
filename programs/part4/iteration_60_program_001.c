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
static void update_checksum(int val) {
    checksum = (checksum << 1) ^ (uint32_t)val;
}

/* Test function with various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;           /* UNORDERED case */
    results[idx++] = (nan > inf) ? 1 : 0;           /* UNORDERED case */
    results[idx++] = (nan <= inf) ? 1 : 0;          /* UNORDERED case */
    results[idx++] = (nan >= inf) ? 1 : 0;          /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;          /* UNORDERED/UNEQ case */
    results[idx++] = (inf != nan) ? 1 : 0;          /* ORDERED/LTGT case */
    results[idx++] = (nan != nan) ? 1 : 0;          /* UNORDERED case */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);    /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);  /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);  /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);         /* UNLT */
    results[idx++] = __builtin_isless(inf, nan);         /* UNGT */
    results[idx++] = __builtin_isgreater(nan, inf);      /* UNGT */
    results[idx++] = __builtin_isgreater(inf, nan);      /* UNLT */
    results[idx++] = __builtin_islessequal(nan, inf);    /* UNLE */
    results[idx++] = __builtin_islessequal(inf, nan);    /* UNGE */
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, nan); /* UNLE */
    
    /* 3. Complex expressions with arithmetic that could produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    volatile double inf_div_inf = inf / inf;
    
    results[idx++] = (inf_minus_inf == inf_minus_inf) ? 1 : 0;  /* UNORDERED/UNEQ */
    results[idx++] = (zero_div_zero < one) ? 1 : 0;             /* UNORDERED */
    results[idx++] = (inf_div_inf > neg_one) ? 1 : 0;           /* UNORDERED */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;              /* UNORDERED */
    results[idx++] = ((double)ld_nan > nan) ? 1 : 0;            /* UNORDERED */
    results[idx++] = (f_inf != (float)nan) ? 1 : 0;             /* ORDERED/LTGT */
    
    /* 5. Control flow based on unordered comparisons */
    for (int i = 0; i < 8; i++) {
        volatile double a = (i & 1) ? nan : inf;
        volatile double b = (i & 2) ? nan : zero;
        
        if (__builtin_isunordered(a, b)) {
            results[idx++] = 1;  /* UNORDERED */
        } else if (__builtin_isless(a, b)) {
            results[idx++] = 2;  /* UNLT */
        } else if (__builtin_isgreater(a, b)) {
            results[idx++] = 3;  /* UNGT */
        } else if (__builtin_islessequal(a, b)) {
            results[idx++] = 4;  /* UNLE */
        } else if (__builtin_isgreaterequal(a, b)) {
            results[idx++] = 5;  /* UNGE */
        } else if (__builtin_islessgreater(a, b)) {
            results[idx++] = 6;  /* LTGT */
        } else {
            results[idx++] = 7;  /* UNEQ or ORDERED */
        }
    }
    
    /* Update checksum with all results */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {__builtin_inff(), __builtin_nanf(""), 1.0f, -__builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result;
    
    /* Various comparison operations */
    cmp_result = vec_a > vec_b;   /* May generate UNGT/UNLT condition codes */
    cmp_result = vec_a < vec_b;   /* May generate UNLT/UNGT condition codes */
    cmp_result = vec_a >= vec_b;  /* May generate UNGE/UNLE condition codes */
    cmp_result = vec_a <= vec_b;  /* May generate UNLE/UNGE condition codes */
    cmp_result = vec_a == vec_b;  /* May generate UNEQ condition codes */
    cmp_result = vec_a != vec_b;  /* May generate LTGT condition codes */
    
    /* Extract comparison mask (x86-specific) */
    int mask;
    #ifdef __SSE__
    mask = __builtin_ia32_movmskps(cmp_result);
    update_checksum(mask);
    #endif
    
    /* Store to memory and check individual elements */
    float stored[4];
    memcpy(stored, &cmp_result, sizeof(stored));
    for (int i = 0; i < 4; i++) {
        update_checksum(stored[i] != 0.0f);
    }
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    v2df vec_dc;
    
    vec_dc = vec_da > vec_db;
    vec_dc = vec_da < vec_db;
    vec_dc = vec_da >= vec_db;
    vec_dc = vec_da <= vec_db;
    vec_dc = vec_da == vec_db;
    vec_dc = vec_da != vec_db;
    
    #ifdef __SSE2__
    int mask_d = __builtin_ia32_movmskpd(vec_dc);
    update_checksum(mask_d);
    #endif
}

/* Test function with inline assembly */
void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result_p, result_z, result_c;
    
    /* Inline assembly with explicit condition codes */
    #ifdef __x86_64__
    /* ucomisd - unordered compare scalar double */
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
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* fucomi - floating point compare and set flags */
    int fpu_result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %0\n\t"
        : "=r"(fpu_result)
        : "m"(a), "m"(b)
        : "cc"
    );
    
    update_checksum(result_p);
    update_checksum(result_z);
    update_checksum(result_c);
    update_checksum(fpu_result);
    #endif
}

/* Switch statement driven by unordered comparison results */
void test_switch_based_on_comparisons(void) {
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
            volatile double a = values[i];
            volatile double b = values[j];
            int condition = 0;
            
            /* Determine condition code through comparisons */
            if (__builtin_isunordered(a, b)) {
                condition = 0;  /* UNORDERED */
            } else if (a < b) {
                condition = 1;  /* UNLT or LT */
            } else if (a > b) {
                condition = 2;  /* UNGT or GT */
            } else if (a <= b) {
                condition = 3;  /* UNLE or LE */
            } else if (a >= b) {
                condition = 4;  /* UNGE or GE */
            } else if (a != b) {
                condition = 5;  /* LTGT */
            } else {
                condition = 6;  /* UNEQ or EQ */
            }
            
            /* Switch on the condition */
            switch (condition) {
                case 0:  /* UNORDERED */
                    global_counter += 1;
                    break;
                case 1:  /* UNLT */
                    global_counter += 2;
                    break;
                case 2:  /* UNGT */
                    global_counter += 3;
                    break;
                case 3:  /* UNLE */
                    global_counter += 4;
                    break;
                case 4:  /* UNGE */
                    global_counter += 5;
                    break;
                case 5:  /* LTGT */
                    global_counter += 6;
                    break;
                case 6:  /* UNEQ */
                    global_counter += 7;
                    break;
            }
        }
    }
    
    update_checksum(global_counter);
}

/* Main test driver */
int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    /* Run all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_inline_assembly();
    test_switch_based_on_comparisons();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is for x86/x86-64 architectures only.\n");
    return 0;
}

#endif
