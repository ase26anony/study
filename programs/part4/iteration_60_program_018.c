/* test_unordered_comparisons.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test scalar unordered comparisons using operators */
void test_scalar_operator_comparisons() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double neg_inf = -__builtin_inf();
    
    /* These should generate various condition codes */
    if (nan < inf) checksum += 1;      /* UNORDERED case */
    if (nan == nan) checksum += 2;     /* UNORDERED/UNEQ cases */
    if (inf != nan) checksum += 4;     /* ORDERED/LTGT cases */
    if (nan <= zero) checksum += 8;    /* UNORDERED/UNLE cases */
    if (nan >= neg_inf) checksum += 16; /* UNORDERED/UNGE cases */
    if (nan > zero) checksum += 32;    /* UNORDERED/UNGT cases */
    if (zero < nan) checksum += 64;    /* UNORDERED/UNLT cases */
    
    /* Complex expressions that might produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    if (inf_minus_inf == nan) checksum += 128;  /* UNORDERED/UNEQ */
    if (inf_minus_inf != zero) checksum += 256; /* UNORDERED/LTGT */
    if (zero_div_zero < inf) checksum += 512;   /* UNORDERED/UNLT */
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    
    if ((double)f_nan == nan) checksum += 1024;
    if (nan != (double)ld_nan) checksum += 2048;
}

/* Test GCC built-in unordered comparison functions */
void test_builtin_unordered_comparisons() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* Direct use of built-ins that map to condition codes */
    if (__builtin_isunordered(nan, inf)) checksum += 4096;      /* UNORDERED */
    if (!__builtin_isunordered(one, zero)) checksum += 8192;    /* ORDERED */
    if (__builtin_islessgreater(nan, zero)) checksum += 16384;  /* LTGT */
    if (__builtin_isless(nan, inf)) checksum += 32768;          /* UNLT */
    if (__builtin_isgreater(inf, nan)) checksum += 65536;       /* UNGT */
    if (__builtin_islessequal(zero, nan)) checksum += 131072;   /* UNLE */
    if (__builtin_isgreaterequal(nan, zero)) checksum += 262144; /* UNGE */
    
    /* Nested built-ins in ternary expressions */
    int result = __builtin_isunordered(nan, zero) ? 
                 (__builtin_islessgreater(inf, nan) ? 1 : 2) : 
                 (__builtin_isless(zero, nan) ? 3 : 4);
    checksum += result;
    
    /* Built-ins in loop conditions */
    for (int i = 0; i < 3 && __builtin_isunordered(nan, (double)i); i++) {
        checksum += i * 100;
    }
}

/* Test vector comparisons using GCC extensions */
void test_vector_comparisons() {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {0.0f, __builtin_nanf(""), 2.0f, __builtin_inff()};
    v4sf vec_c = {__builtin_nanf(""), __builtin_nanf(""), 3.0f, 4.0f};
    
    /* Vector comparisons that generate multiple condition checks */
    v4sf cmp_result = vec_a > vec_b;   /* Should generate UNORDERED/UNGT checks */
    v4sf cmp_result2 = vec_a == vec_c; /* Should generate UNORDERED/UNEQ checks */
    v4sf cmp_result3 = vec_a <= vec_b; /* Should generate UNORDERED/UNLE checks */
    
    /* Extract comparison masks - forces actual comparison codegen */
    int mask1, mask2, mask3;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps(cmp_result);
    mask2 = __builtin_ia32_movmskps(cmp_result2);
    mask3 = __builtin_ia32_movmskps(cmp_result3);
    #else
    /* Fallback: store to memory and check */
    float store1[4], store2[4], store3[4];
    memcpy(store1, &cmp_result, sizeof(cmp_result));
    memcpy(store2, &cmp_result2, sizeof(cmp_result2));
    memcpy(store3, &cmp_result3, sizeof(cmp_result3));
    mask1 = (store1[0] != 0) | ((store1[1] != 0) << 1) | 
            ((store1[2] != 0) << 2) | ((store1[3] != 0) << 3);
    mask2 = (store2[0] != 0) | ((store2[1] != 0) << 1) | 
            ((store2[2] != 0) << 2) | ((store2[3] != 0) << 3);
    mask3 = (store3[0] != 0) | ((store3[1] != 0) << 1) | 
            ((store3[2] != 0) << 2) | ((store3[3] != 0) << 3);
    #endif
    
    checksum += mask1 + mask2 + mask3;
    
    /* Double precision vector comparisons */
    v2df vec_d = {__builtin_nan(""), 1.0};
    v2df vec_e = {0.0, __builtin_nan("")};
    v2df vec_f = vec_d != vec_e;  /* Should generate UNORDERED/LTGT checks */
    
    double store_d[2];
    memcpy(store_d, &vec_f, sizeof(vec_f));
    if (store_d[0] != 0.0) checksum += 1000;
    if (store_d[1] != 0.0) checksum += 2000;
}

/* Test inline assembly with explicit condition codes */
void test_inline_assembly_comparisons() {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 0.0;
    volatile double d = __builtin_nan("");
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Explicit ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Compare with normal number */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result2)
        : "x"(a), "x"(c)
        : "cc"
    );
    
    /* Compare NaN with NaN */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(result3)
        : "x"(a), "x"(d)
        : "cc"
    );
    
    /* fucomi instruction (x87) */
    #ifdef __i386__
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "seta %0"
        : "=r"(result4)
        : "m"(b), "m"(c)
        : "cc"
    );
    #endif
    
    checksum += result1 + result2 + result3 + result4;
}

/* Test control flow driven by unordered results */
void test_control_flow_comparisons() {
    volatile double nan = __builtin_nan("");
    volatile double vals[6] = {
        __builtin_nan(""),
        __builtin_inf(),
        -__builtin_inf(),
        0.0,
        1.0,
        -1.0
    };
    
    /* Switch based on comparison results */
    for (int i = 0; i < 6; i++) {
        int condition = 0;
        
        /* Build condition code from multiple comparisons */
        if (__builtin_isunordered(nan, vals[i])) condition |= 1;
        if (nan == vals[i]) condition |= 2;
        if (nan != vals[i]) condition |= 4;
        if (nan < vals[i]) condition |= 8;
        if (nan > vals[i]) condition |= 16;
        
        switch (condition) {
            case 1:  /* UNORDERED only */
                checksum += i * 10;
                break;
            case 2:  /* UNORDERED + EQ -> UNEQ */
                checksum += i * 20;
                break;
            case 4:  /* UNORDERED + NE -> LTGT */
                checksum += i * 30;
                break;
            case 8:  /* UNORDERED + LT -> UNLT */
                checksum += i * 40;
                break;
            case 16: /* UNORDERED + GT -> UNGT */
                checksum += i * 50;
                break;
            case 0:  /* ORDERED */
                checksum += i * 60;
                break;
            default:
                checksum += i * 70;
                break;
        }
    }
    
    /* Complex conditional with arithmetic that may produce NaN */
    volatile double x = __builtin_inf();
    volatile double y = __builtin_inf();
    
    for (int i = 0; i < 4; i++) {
        volatile double expr = (x - y) / (i == 0 ? 0.0 : 1.0);
        
        if (__builtin_islessgreater(expr, 0.0)) {
            checksum += 10000;
        } else if (__builtin_isunordered(expr, expr)) {
            checksum += 20000;
        } else if (__builtin_isless(expr, 100.0)) {
            checksum += 30000;
        } else if (__builtin_isgreaterequal(expr, -100.0)) {
            checksum += 40000;
        }
        
        /* Mix with integer comparisons to prevent optimization */
        if (global_counter++ < 10) {
            x = x * 0.5;
        }
    }
}

/* Test math functions with NaN inputs */
void test_math_function_comparisons() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA with NaN inputs */
    #ifdef __FMA__
    volatile double fma_result = __builtin_fma(nan, 2.0, 3.0);
    #else
    volatile double fma_result = nan * 2.0 + 3.0;
    #endif
    
    /* Comparisons after math operations */
    if (fma_result == nan) checksum += 500000;
    if (fma_result != inf) checksum += 600000;
    if (fma_result < 0.0) checksum += 700000;
    if (fma_result > 0.0) checksum += 800000;
    
    /* Division that may produce NaN/Inf */
    volatile double div_result = inf / 0.0;
    if (__builtin_isunordered(div_result, div_result)) checksum += 900000;
    
    /* sqrt of negative number */
    volatile double sqrt_result = __builtin_sqrt(-1.0);
    if (__builtin_islessgreater(sqrt_result, 0.0)) checksum += 1000000;
}

int main() {
    printf("Testing unordered floating-point comparisons on x86...\n");
    
    /* Run all test suites */
    test_scalar_operator_comparisons();
    test_builtin_unordered_comparisons();
    test_vector_comparisons();
    test_inline_assembly_comparisons();
    test_control_flow_comparisons();
    test_math_function_comparisons();
    
    printf("Checksum: %u\n", (unsigned int)checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}

#else /* Non-x86 fallback */
int main() {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
