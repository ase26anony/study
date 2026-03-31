/* test_unordered_comparisons.c
 * Designed to trigger x86 floating-point unordered comparison condition codes
 * Specifically targets lines 13992-14017 in i386.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* GCC vector extension for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Test 1: Direct unordered comparisons with operators */
void test_direct_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_inf = -__builtin_inf();
    
    int results[16];
    int idx = 0;
    
    /* These should generate various condition codes */
    
    /* UNORDERED: nan compared with anything is unordered */
    results[idx++] = (nan < inf) ? 1 : 0;        /* Should be false (unordered) */
    results[idx++] = (nan > inf) ? 1 : 0;        /* Should be false (unordered) */
    results[idx++] = (nan == nan) ? 1 : 0;       /* Should be false (unordered) */
    
    /* ORDERED: normal comparisons */
    results[idx++] = (inf > zero) ? 1 : 0;       /* Should be true (ordered) */
    results[idx++] = (zero < one) ? 1 : 0;       /* Should be true (ordered) */
    
    /* UNEQ: unordered or equal */
    results[idx++] = !(nan != nan) ? 1 : 0;      /* NaN != NaN is true, so this is false */
    
    /* UNGE: unordered or greater or equal */
    results[idx++] = !(nan < zero) ? 1 : 0;      /* NaN < 0 is false (unordered) */
    
    /* UNGT: unordered or greater */
    results[idx++] = !(nan <= inf) ? 1 : 0;      /* NaN <= inf is false (unordered) */
    
    /* UNLE: unordered or less or equal */
    results[idx++] = !(nan > neg_inf) ? 1 : 0;   /* NaN > -inf is false (unordered) */
    
    /* UNLT: unordered or less */
    results[idx++] = !(nan >= zero) ? 1 : 0;     /* NaN >= 0 is false (unordered) */
    
    /* LTGT: less or greater (but not equal, not unordered) */
    results[idx++] = (inf != zero) ? 1 : 0;      /* Should be true */
    results[idx++] = (one != zero) ? 1 : 0;      /* Should be true */
    
    /* Complex expressions that might generate multiple condition codes */
    volatile double inf_minus_inf = inf - inf;  /* Should produce NaN */
    results[idx++] = (inf_minus_inf == inf_minus_inf) ? 1 : 0;  /* NaN == NaN is false */
    results[idx++] = (inf_minus_inf != inf_minus_inf) ? 1 : 0;  /* NaN != NaN is true */
    
    /* Division by zero producing infinity */
    volatile double div_zero = one / zero;
    results[idx++] = (div_zero == inf) ? 1 : 0;  /* Should be true */
    
    /* Prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        global_counter += results[i];
    }
}

/* Test 2: Built-in unordered comparison functions */
void test_builtin_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    int results[12];
    int idx = 0;
    
    /* These built-ins directly map to the condition codes */
    
    /* __builtin_isunordered - maps to UNORDERED */
    results[idx++] = __builtin_isunordered(nan, inf) ? 1 : 0;    /* Should be true */
    results[idx++] = __builtin_isunordered(zero, one) ? 1 : 0;   /* Should be false */
    
    /* __builtin_isless - maps to LT */
    results[idx++] = __builtin_isless(zero, one) ? 1 : 0;        /* Should be true */
    results[idx++] = __builtin_isless(nan, zero) ? 1 : 0;        /* Should be false */
    
    /* __builtin_isgreater - maps to GT */
    results[idx++] = __builtin_isgreater(one, zero) ? 1 : 0;     /* Should be true */
    results[idx++] = __builtin_isgreater(nan, inf) ? 1 : 0;      /* Should be false */
    
    /* __builtin_islessequal - maps to LE */
    results[idx++] = __builtin_islessequal(zero, one) ? 1 : 0;   /* Should be true */
    results[idx++] = __builtin_islessequal(zero, zero) ? 1 : 0;  /* Should be true */
    
    /* __builtin_isgreaterequal - maps to GE */
    results[idx++] = __builtin_isgreaterequal(one, zero) ? 1 : 0; /* Should be true */
    results[idx++] = __builtin_isgreaterequal(zero, zero) ? 1 : 0; /* Should be true */
    
    /* __builtin_islessgreater - maps to LTGT */
    results[idx++] = __builtin_islessgreater(one, zero) ? 1 : 0;  /* Should be true */
    results[idx++] = __builtin_islessgreater(nan, zero) ? 1 : 0;  /* Should be false */
    
    /* Nested built-ins to force complex condition code generation */
    if (__builtin_isunordered(nan, zero) || __builtin_isless(one, zero)) {
        results[0] = results[0];  /* Use result to prevent elimination */
    }
    
    /* Ternary operator with built-ins */
    int temp = __builtin_isunordered(nan, nan) ? 
               (__builtin_isless(zero, one) ? 1 : 0) : 
               (__builtin_isgreater(one, zero) ? 2 : 0);
    results[idx-1] += temp;
    
    /* Prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        global_counter += results[i];
    }
}

/* Test 3: Vector comparisons using GCC extensions */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {0.0f, 1.0f, 3.0f, __builtin_inff()};
    v4sf vec_c = {__builtin_nanf(""), __builtin_nanf(""), 0.0f, 0.0f};
    
    /* These vector comparisons may generate multiple condition codes */
    v4sf cmp_result;
    
    /* Greater than comparison - may generate GT/UNGT condition codes */
    cmp_result = vec_a > vec_b;
    
    /* Less than comparison - may generate LT/UNLT condition codes */
    v4sf cmp_result2 = vec_a < vec_b;
    
    /* Equal comparison - may generate EQ/UNEQ condition codes */
    v4sf cmp_result3 = vec_a == vec_c;
    
    /* Not equal comparison - may generate NE/LTGT condition codes */
    v4sf cmp_result4 = vec_a != vec_b;
    
    /* Extract comparison masks using x86-specific intrinsic */
    #ifdef __SSE__
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    int mask2 = __builtin_ia32_movmskps(cmp_result2);
    int mask3 = __builtin_ia32_movmskps(cmp_result3);
    int mask4 = __builtin_ia32_movmskps(cmp_result4);
    
    global_counter += mask1 + mask2 + mask3 + mask4;
    #endif
    
    /* Alternative: store to memory and check */
    float stored_results[4];
    memcpy(stored_results, &cmp_result, sizeof(cmp_result));
    for (int i = 0; i < 4; i++) {
        global_counter += (stored_results[i] != 0.0f);
    }
}

/* Test 4: Mixed-type comparisons and arithmetic */
void test_mixed_type_comparisons(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    int results[10];
    int idx = 0;
    
    /* Cross-type comparisons */
    results[idx++] = (f_nan < d_inf) ? 1 : 0;      /* float NaN vs double inf */
    results[idx++] = (d_nan == ld_nan) ? 1 : 0;    /* double NaN vs long double NaN */
    
    /* Arithmetic that produces NaN */
    volatile double inf_minus_inf = d_inf - d_inf;
    volatile double zero_div_zero = 0.0 / 0.0;
    volatile double sqrt_neg = __builtin_sqrt(-1.0);
    
    /* Comparisons with arithmetic results */
    results[idx++] = (inf_minus_inf == d_nan) ? 1 : 0;
    results[idx++] = (zero_div_zero != sqrt_neg) ? 1 : 0;
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(d_nan, 2.0, 3.0);
    results[idx++] = (fma_result == fma_result) ? 1 : 0;  /* NaN == NaN is false */
    #endif
    
    /* Complex expression mixing types and operations */
    results[idx++] = ((f_inf * 0.0) != (d_inf * 0.0f)) ? 1 : 0;  /* NaN != NaN is true */
    
    /* Long double comparisons */
    results[idx++] = (ld_nan < ld_inf) ? 1 : 0;
    results[idx++] = (ld_inf > 0.0L) ? 1 : 0;
    
    /* Prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        global_counter += results[i];
    }
}

/* Test 5: Inline assembly with explicit condition codes */
void test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    volatile double d = 1.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly using ucomisd (unordered compare scalar double) */
    #ifdef __x86_64__
    /* Test UNORDERED: compare NaN with INF */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result1)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    /* Test ORDERED: compare two normal numbers */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result2)
        : "x" (c), "x" (d)
        : "al", "cc"
    );
    
    /* Test for equality/unordered */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result3)
        : "x" (c), "x" (c)  /* Compare zero with zero */
        : "al", "cc"
    );
    
    /* Test for less/greater (LTGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result4)
        : "x" (d), "x" (c)  /* Compare 1.0 with 0.0 */
        : "al", "cc"
    );
    #endif
    
    global_counter += result1 + result2 + result3 + result4;
}

/* Test 6: Control flow driven by unordered results */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {0.0, 1.0, nan, inf, -inf};
    
    int checksum = 0;
    
    /* Switch statement based on comparison results */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int condition = 0;
            
            /* Determine condition code through comparisons */
            if (__builtin_isunordered(values[i], values[j])) {
                condition = 1;  /* UNORDERED */
            } else if (values[i] == values[j]) {
                condition = 2;  /* EQ */
            } else if (values[i] < values[j]) {
                condition = 3;  /* LT */
            } else if (values[i] > values[j]) {
                condition = 4;  /* GT */
            } else {
                condition = 5;  /* Should not happen */
            }
            
            /* Switch on the condition - forces generation of jump tables */
            switch (condition) {
                case 1:  /* UNORDERED */
                    checksum += 100;
                    break;
                case 2:  /* EQ */
                    checksum += 200;
                    break;
                case 3:  /* LT */
                    checksum += 300;
                    if (__builtin_islessgreater(values[i], values[j])) {
                        checksum += 50;  /* LTGT */
                    }
                    break;
                case 4:  /* GT */
                    checksum += 400;
                    if (!__builtin_isunordered(values[i], values[j]) && 
                        !__builtin_islessequal(values[i], values[j])) {
                        checksum += 75;  /* UNLE inverse */
                    }
                    break;
                default:
                    checksum += 999;
            }
            
            /* Complex if-else chain with mixed conditions */
            if (__builtin_isunordered(values[i], values[j]) || 
                values[i] == values[j]) {
                checksum += 10;  /* UNEQ-like */
            } else if (!__builtin_isless(values[i], values[j])) {
                checksum += 20;  /* UNLT inverse */
            } else if (!__builtin_isgreater(values[i], values[j])) {
                checksum += 30;  /* UNGT inverse */
            }
        }
    }
    
    global_counter += checksum;
}

int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    /* Run all tests */
    test_direct_comparisons();
    test_builtin_comparisons();
    test_vector_comparisons();
    test_mixed_type_comparisons();
    test_inline_assembly();
    test_control_flow();
    
    printf("Global counter: %d\n", global_counter);
    printf("Tests completed.\n");
    
    return global_counter != 0 ? 0 : 1;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is designed for x86/x86-64 architectures only.\n");
    return 0;
}

#endif
