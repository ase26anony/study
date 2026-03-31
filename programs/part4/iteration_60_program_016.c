#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __x86_64__ || __i386__

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test scalar unordered comparisons using operators */
void test_scalar_operators() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf) checksum += 1;      /* UNORDERED? */
    if (nan == nan) checksum += 2;     /* UNORDERED/UNEQ? */
    if (inf != nan) checksum += 4;     /* ORDERED/LTGT? */
    if (nan <= one) checksum += 8;     /* UNORDERED/UNLE? */
    if (nan >= zero) checksum += 16;   /* UNORDERED/UNGE? */
    if (nan > inf) checksum += 32;     /* UNORDERED/UNGT? */
    
    /* More complex expressions */
    volatile double inf_minus_inf = inf - inf;
    if (inf_minus_inf == nan) checksum += 64;  /* UNORDERED/UNEQ? */
    if (inf_minus_inf != zero) checksum += 128; /* UNORDERED/LTGT? */
    
    /* Division by zero producing inf */
    volatile double div_zero = one / zero;
    if (div_zero > nan) checksum += 256;  /* UNORDERED/UNGT? */
}

/* Test built-in unordered comparison functions */
void test_builtin_functions() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* Direct use of built-ins that map to condition codes */
    if (__builtin_isunordered(nan, inf)) checksum += 512;      /* UNORDERED */
    if (!__builtin_isunordered(one, zero)) checksum += 1024;   /* ORDERED */
    if (__builtin_islessgreater(nan, inf)) checksum += 2048;   /* LTGT */
    if (__builtin_isless(nan, one)) checksum += 4096;          /* UNLT */
    if (__builtin_isgreater(nan, zero)) checksum += 8192;      /* UNGT */
    if (__builtin_islessequal(inf, nan)) checksum += 16384;    /* UNLE */
    if (__builtin_isgreaterequal(zero, nan)) checksum += 32768; /* UNGE */
    
    /* Nested in ternary operators */
    int result = __builtin_isunordered(nan, nan) ? 
                 (__builtin_islessgreater(inf, nan) ? 1 : 2) : 3;
    checksum += result;
}

/* Test vector comparisons using GCC extensions */
void test_vector_comparisons() {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), __builtin_nanf(""), 2.0f, 1.0f};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_result;
    
    cmp_result = vec_a > vec_b;  /* May generate UNGT/UNLT/UNORDERED */
    cmp_result = vec_a < vec_b;  /* May generate UNLT/UNGT/UNORDERED */
    cmp_result = vec_a == vec_b; /* May generate UNEQ/UNORDERED */
    cmp_result = vec_a != vec_b; /* May generate LTGT/UNORDERED */
    
    /* Extract comparison mask */
    int mask = __builtin_ia32_movmskps(cmp_result);
    checksum += mask;
    
    /* Double precision vector */
    v2df vec_d1 = {__builtin_nan(""), __builtin_inf()};
    v2df vec_d2 = {__builtin_inf(), __builtin_nan("")};
    v2df vec_d3 = vec_d1 > vec_d2;
    
    /* Access elements to force computation */
    volatile double elem = vec_d3[0];
    checksum += (uint32_t)elem;
}

/* Test mixed-type comparisons and arithmetic */
void test_mixed_types() {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    volatile long double ld_inf = __builtin_infl();
    
    /* Cross-type comparisons */
    if ((double)f_nan < d_inf) checksum += 65536;
    if (f_nan == (float)d_nan) checksum += 131072;
    if (ld_nan != d_nan) checksum += 262144;
    
    /* Arithmetic that produces NaN */
    volatile double complex_expr = (d_inf - d_inf) / (f_inf * 0.0);
    if (complex_expr == d_nan) checksum += 524288;
    if (complex_expr != 0.0) checksum += 1048576;
    
    /* Use FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(d_nan, d_inf, 1.0);
    if (fma_result > 0.0) checksum += 2097152;
}

/* Test inline assembly with explicit condition codes */
void test_inline_asm() {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1, result2, result3, result4;
    
    /* Using ucomisd with various condition codes */
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
    
    /* Using fucomi instruction */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "t"(a), "f"(d)
        : "al", "cc"
    );
    
    checksum += result1 + result2 + result3;
}

/* Control flow based on unordered comparison results */
void test_control_flow() {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0, -inf};
    
    int results[5][5] = {0};
    
    /* Nested comparisons controlling flow */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (__builtin_isunordered(vals[i], vals[j])) {
                results[i][j] = 1;  /* UNORDERED */
            } else if (__builtin_isless(vals[i], vals[j])) {
                results[i][j] = 2;  /* UNLT */
            } else if (__builtin_isgreater(vals[i], vals[j])) {
                results[i][j] = 3;  /* UNGT */
            } else if (__builtin_islessequal(vals[i], vals[j])) {
                results[i][j] = 4;  /* UNLE */
            } else if (__builtin_isgreaterequal(vals[i], vals[j])) {
                results[i][j] = 5;  /* UNGE */
            } else if (__builtin_islessgreater(vals[i], vals[j])) {
                results[i][j] = 6;  /* LTGT */
            } else {
                results[i][j] = 7;  /* UNEQ or ORDERED equal */
            }
            checksum += results[i][j];
        }
    }
    
    /* Switch statement based on comparison results */
    volatile double x = nan;
    volatile double y = inf;
    
    int code = 0;
    code |= __builtin_isunordered(x, y) ? 0x1 : 0;
    code |= __builtin_isless(x, y) ? 0x2 : 0;
    code |= __builtin_isgreater(x, y) ? 0x4 : 0;
    
    switch (code) {
        case 0x1: checksum += 1000; break;  /* UNORDERED */
        case 0x2: checksum += 2000; break;  /* UNLT */
        case 0x3: checksum += 3000; break;  /* UNORDERED + UNLT */
        case 0x4: checksum += 4000; break;  /* UNGT */
        case 0x5: checksum += 5000; break;  /* UNORDERED + UNGT */
        default:  checksum += 6000; break;
    }
}

int main() {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_types();
    test_inline_asm();
    test_control_flow();
    
    printf("Checksum: %u\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}

#else /* Non-x86 target */

int main() {
    printf("This test is for x86 targets only.\n");
    return 0;
}

#endif
