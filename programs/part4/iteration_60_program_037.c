#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int sink;

/* Test function that uses various unordered comparison patterns */
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
    results[idx++] = (nan < inf) ? 1 : 0;        /* UNORDERED path */
    results[idx++] = (nan > inf) ? 2 : 0;        /* UNORDERED path */
    results[idx++] = (nan <= inf) ? 3 : 0;       /* UNORDERED path */
    results[idx++] = (nan >= inf) ? 4 : 0;       /* UNORDERED path */
    results[idx++] = (nan == nan) ? 5 : 0;       /* UNEQ path */
    results[idx++] = (nan != nan) ? 6 : 0;       /* LTGT path? Actually UNORDERED */
    results[idx++] = (inf != nan) ? 7 : 0;       /* ORDERED path */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf) ? 8 : 0;    /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan) ? 9 : 0;    /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan) ? 10 : 0;   /* UNORDERED */
    results[idx++] = !__builtin_isunordered(inf, zero) ? 11 : 0; /* ORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf) ? 12 : 0; /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan) ? 13 : 0; /* LTGT */
    results[idx++] = __builtin_islessgreater(nan, nan) ? 14 : 0; /* LTGT */
    
    results[idx++] = __builtin_isless(nan, inf) ? 15 : 0;        /* UNLT */
    results[idx++] = __builtin_isless(inf, nan) ? 16 : 0;        /* UNGT */
    results[idx++] = __builtin_isless(neg_inf, nan) ? 17 : 0;    /* UNLT */
    
    results[idx++] = __builtin_isgreater(nan, inf) ? 18 : 0;     /* UNGT */
    results[idx++] = __builtin_isgreater(inf, nan) ? 19 : 0;     /* UNLT */
    results[idx++] = __builtin_isgreater(nan, neg_inf) ? 20 : 0; /* UNGT */
    
    results[idx++] = __builtin_islessequal(nan, inf) ? 21 : 0;   /* UNLE */
    results[idx++] = __builtin_islessequal(inf, nan) ? 22 : 0;   /* UNGE */
    results[idx++] = __builtin_islessequal(nan, nan) ? 23 : 0;   /* UNLE */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf) ? 24 : 0; /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, nan) ? 25 : 0; /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, nan) ? 26 : 0; /* UNGE */
    
    /* 3. Complex expressions that may produce NaN */
    volatile double nan_prod = zero / zero;
    volatile double inf_minus_inf = inf - inf;
    
    results[idx++] = (nan_prod < one) ? 27 : 0;          /* UNORDERED */
    results[idx++] = (inf_minus_inf == nan) ? 28 : 0;    /* UNEQ */
    results[idx++] = (inf_minus_inf != inf_minus_inf) ? 29 : 0; /* LTGT */
    
    /* 4. Mixed-type comparisons */
    volatile float nan_f = __builtin_nanf("");
    volatile float inf_f = __builtin_inff();
    
    results[idx++] = (nan_f < inf_f) ? 30 : 0;           /* UNORDERED (float) */
    results[idx++] = (nan_f == nan_f) ? 31 : 0;          /* UNEQ (float) */
    
    /* Store to prevent dead code elimination */
    for (int i = 0; i < idx; i++) {
        sink = results[i];
    }
}

/* Test function using vector extensions */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {__builtin_nanf(""), __builtin_inff(), 1.0f, -__builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate unordered condition codes */
    v4sf cmp_result;
    
    /* These comparisons with NaN elements will need unordered handling */
    cmp_result = vec_a > vec_b;   /* UNGT/UNLT for NaN elements */
    cmp_result = vec_a < vec_b;   /* UNLT/UNGT for NaN elements */
    cmp_result = vec_a == vec_b;  /* UNEQ for NaN elements */
    cmp_result = vec_a != vec_b;  /* LTGT for NaN elements */
    
    /* Extract comparison mask - forces actual comparison codegen */
    int mask;
    mask = __builtin_ia32_movmskps(cmp_result);
    sink = mask;
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    
    v2df cmp_result_d = vec_da > vec_db;
    mask = __builtin_ia32_movmskpd(cmp_result_d);
    sink = mask;
}

/* Test function using inline assembly */
void test_asm_comparisons(void) {
    double a = __builtin_nan("");
    double b = __builtin_inf();
    double c = 1.0;
    int result;
    
    /* Inline assembly that uses explicit condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    sink = result;
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "al", "cc"
    );
    sink = result;
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(a)
        : "al", "cc"
    );
    sink = result;
}

/* Control flow based on unordered comparisons */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double val = 1.0;
    
    int counter = 0;
    
    /* Switch-like behavior using comparison results */
    if (__builtin_isunordered(nan, inf)) {
        counter += 1;  /* UNORDERED */
    }
    
    if (!__builtin_isunordered(val, inf)) {
        counter += 2;  /* ORDERED */
    }
    
    if (__builtin_islessgreater(nan, val)) {
        counter += 4;  /* LTGT */
    }
    
    if (__builtin_isless(nan, inf)) {
        counter += 8;  /* UNLT */
    }
    
    if (__builtin_isgreater(inf, nan)) {
        counter += 16; /* UNGT */
    }
    
    if (__builtin_islessequal(nan, nan)) {
        counter += 32; /* UNLE */
    }
    
    if (__builtin_isgreaterequal(nan, nan)) {
        counter += 64; /* UNGE */
    }
    
    /* Ternary operators with unordered comparisons */
    int result = __builtin_isunordered(nan, inf) ? 100 : 
                 __builtin_islessgreater(nan, val) ? 200 : 300;
    sink = result;
    
    sink = counter;
}

/* Main test driver */
int main(void) {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    /* Run all test functions */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_asm_comparisons();
    test_control_flow();
    
    /* Create a checksum to prevent optimization */
    int checksum = sink;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
