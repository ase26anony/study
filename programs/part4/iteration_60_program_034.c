/* test_float_conditions.c - Trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Feature detection for x86 */
#if defined(__x86_64__) || defined(__i386__) || defined(__i686__)

/* Prevent optimization of floating-point values */
static volatile int global_counter = 0;

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to create complex floating expressions */
static double create_nan_expression(double a, double b) {
    /* Operations that can produce NaN */
    volatile double zero = 0.0;
    volatile double inf = __builtin_inf();
    
    /* Various NaN-producing expressions */
    double result = (a - a) / zero;      /* 0/0 = NaN */
    result += (inf - inf);               /* inf-inf = NaN */
    result += zero / zero;               /* Another NaN */
    
    return result + b; /* Mix with parameter */
}

/* Function using built-in unordered comparisons */
static int test_builtin_unordered_comparisons(double nan, double inf, double normal) {
    int results[16] = {0};
    int idx = 0;
    
    /* Direct unordered comparisons using built-ins */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT -> "une" */
    results[idx++] = __builtin_islessgreater(inf, nan);    /* LTGT -> "une" */
    
    /* Ordered comparisons */
    results[idx++] = !__builtin_isunordered(inf, normal);  /* ORDERED -> "ord" */
    results[idx++] = !__builtin_isunordered(normal, 1.0);  /* ORDERED -> "ord" */
    
    /* Complex expressions with unordered results */
    double nan_expr = create_nan_expression(inf, normal);
    results[idx++] = __builtin_isunordered(nan_expr, normal); /* UNORDERED */
    
    /* UNEQ: unordered or equal */
    volatile double zero = 0.0;
    results[idx++] = !__builtin_islessgreater(zero, -zero); /* UNEQ -> "ueq" (0 == -0) */
    
    /* UNGE: unordered or greater or equal -> "nlt" */
    results[idx++] = !__builtin_isless(inf, normal);        /* UNGE -> "nlt" */
    
    /* UNGT: unordered or greater -> "nle" */
    results[idx++] = !__builtin_islessequal(normal, 0.5);   /* UNGT -> "nle" */
    
    /* UNLE: unordered or less or equal -> "ule" */
    results[idx++] = __builtin_islessequal(nan, inf) || 
                     __builtin_isunordered(nan, inf);       /* UNLE -> "ule" */
    
    /* UNLT: unordered or less -> "ult" */
    results[idx++] = __builtin_isless(nan, inf) || 
                     __builtin_isunordered(nan, inf);       /* UNLT -> "ult" */
    
    /* Combine results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test with GCC vector extensions */
static int test_vector_comparisons(void) {
    v4sf vec_a, vec_b;
    float *a = (float*)&vec_a;
    float *b = (float*)&vec_b;
    
    /* Initialize vectors with mixed values */
    a[0] = __builtin_nanf("");
    a[1] = __builtin_inff();
    a[2] = 1.5f;
    a[3] = -__builtin_inff();
    
    b[0] = __builtin_inff();
    b[1] = __builtin_nanf("");
    b[2] = 1.5f;
    b[3] = 0.0f;
    
    /* Vector comparisons that generate condition codes */
    v4sf cmp_result;
    int mask;
    
    /* This should generate multiple condition checks */
    cmp_result = vec_a > vec_b;      /* UNORDERED/ORDERED checks */
    
    /* Extract comparison mask - forces evaluation */
    mask = __builtin_ia32_movmskps(cmp_result);
    
    /* More vector comparisons */
    cmp_result = vec_a == vec_b;     /* UNEQ checks */
    mask += __builtin_ia32_movmskps(cmp_result);
    
    cmp_result = vec_a >= vec_b;     /* UNGE -> "nlt" checks */
    mask += __builtin_ia32_movmskps(cmp_result);
    
    cmp_result = vec_a <= vec_b;     /* UNLE -> "ule" checks */
    mask += __builtin_ia32_movmskps(cmp_result);
    
    return mask;
}

/* Test with inline assembly */
static int test_inline_asm_comparisons(double nan, double inf, double normal) {
    int results = 0;
    uint8_t byte_result;
    
    /* Explicit ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %[nan], %[inf]\n\t"
        "setp %[result]\n\t"
        : [result] "=r" (byte_result)
        : [nan] "x" (nan), [inf] "x" (inf)
        : "cc"
    );
    results += byte_result;  /* UNORDERED */
    
    /* Compare normal numbers - ordered */
    asm volatile (
        "ucomisd %[a], %[b]\n\t"
        "setnp %[result]\n\t"  /* Not parity = ordered */
        : [result] "=r" (byte_result)
        : [a] "x" (normal), [b] "x" (2.0)
        : "cc"
    );
    results += byte_result;  /* ORDERED */
    
    /* LTGT comparison using fucomi */
    int int_result;
    asm volatile (
        "fucomi %[nan], %[inf]\n\t"
        "setne %[result]\n\t"  /* LTGT -> "une" */
        : [result] "=r" (int_result)
        : [nan] "t" (nan), [inf] "t" (inf)
        : "cc"
    );
    results += int_result;
    
    return results;
}

/* Control flow based on unordered comparisons */
static void test_control_flow(double nan, double inf, double normal) {
    int counter = 0;
    
    /* Switch based on comparison results */
    for (int i = 0; i < 8; i++) {
        double a, b;
        
        /* Vary the operands */
        switch (i % 4) {
            case 0: a = nan; b = inf; break;
            case 1: a = inf; b = nan; break;
            case 2: a = nan; b = normal; break;
            case 3: a = normal; b = inf; break;
        }
        
        /* Complex conditional with multiple comparisons */
        if (__builtin_isunordered(a, b)) {
            counter += 1;  /* UNORDERED path */
        } else if (!__builtin_islessgreater(a, b)) {
            counter += 2;  /* UNEQ path */
        } else if (__builtin_isless(a, b)) {
            counter += 3;  /* UNLT path */
        } else if (__builtin_isgreater(a, b)) {
            counter += 4;  /* UNGT path */
        }
        
        /* Ternary operator with unordered comparison */
        counter += (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) ? 5 : 0;
        counter += (__builtin_isgreaterequal(a, b) || __builtin_isunordered(a, b)) ? 7 : 0;
    }
    
    global_counter += counter;
}

/* Mixed type comparisons */
static int test_mixed_type_comparisons(void) {
    int results = 0;
    
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile double d_nan = __builtin_nan("");
    volatile double d_inf = __builtin_inf();
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    /* Cross-type comparisons */
    results += (f_nan < d_inf) ? 0 : 1;      /* UNORDERED/UNLT */
    results += (d_nan == ld_nan) ? 0 : 1;    /* UNORDERED/UNEQ */
    results += (ld_inf > f_nan) ? 0 : 1;     /* UNORDERED/UNGT */
    
    /* Arithmetic producing NaN followed by comparison */
    volatile float f_zero = 0.0f;
    float f_div = f_inf / f_zero;  /* Should produce inf */
    f_div = f_div - f_inf;         /* inf - inf = NaN */
    
    results += (f_div == f_div) ? 0 : 1;     /* UNORDERED/UNEQ (NaN != NaN) */
    
    /* FMA with NaN inputs */
    double fma_result = __builtin_fma(d_nan, 2.0, d_inf);
    results += (fma_result > 0.0) ? 0 : 1;   /* UNORDERED/UNGT */
    
    return results;
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Initialize volatile floating-point values */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double normal = 3.14159;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    int checksum = 0;
    
    /* Test 1: Built-in unordered comparisons */
    checksum ^= test_builtin_unordered_comparisons(nan, inf, normal);
    
    /* Test 2: Vector comparisons */
    checksum ^= test_vector_comparisons();
    
    /* Test 3: Inline assembly */
    checksum ^= test_inline_asm_comparisons(nan, inf, normal);
    
    /* Test 4: Control flow */
    test_control_flow(nan, inf, normal);
    checksum ^= global_counter;
    
    /* Test 5: Mixed type comparisons */
    checksum ^= test_mixed_type_comparisons();
    
    /* Test 6: Direct operator comparisons (may be optimized differently) */
    volatile int cmp_results[8];
    cmp_results[0] = (nan < inf);      /* UNORDERED/UNLT */
    cmp_results[1] = (nan == nan);     /* UNORDERED/UNEQ */
    cmp_results[2] = (inf != nan);     /* ORDERED/LTGT */
    cmp_results[3] = (normal >= nan);  /* UNORDERED/UNGE */
    cmp_results[4] = (nan > normal);   /* UNORDERED/UNGT */
    cmp_results[5] = (normal <= inf);  /* ORDERED/UNLE */
    cmp_results[6] = (zero == neg_zero); /* ORDERED/UNEQ */
    cmp_results[7] = !(nan < nan) && !(nan > nan); /* LTGT -> "une" */
    
    for (int i = 0; i < 8; i++) {
        checksum += cmp_results[i] * (i + 1);
    }
    
    /* Test 7: Complex nested comparisons */
    double complex_nan = (inf * zero) / zero;
    if (__builtin_isunordered(complex_nan, normal) || 
        (!__builtin_islessgreater(complex_nan, inf) && 
         __builtin_islessequal(normal, inf))) {
        checksum += 0xABCD;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    return checksum != 0 ? 0 : 1;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 architectures */
int main(void) {
    printf("This test is for x86 architectures only.\n");
    return 0;
}

#endif
