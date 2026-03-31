#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile uint32_t checksum = 0;

/* Function to accumulate results into checksum */
static void accumulate(int result) {
    checksum = (checksum << 1) ^ result;
}

#ifdef __x86_64__ || __i386__

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test scalar unordered comparisons using operators */
static void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These comparisons with NaN should generate unordered condition codes */
    
    /* UNORDERED: nan < inf (always false when unordered) */
    if (!(nan < inf)) {
        accumulate(1);
    }
    
    /* UNORDERED: nan > inf */
    if (!(nan > inf)) {
        accumulate(2);
    }
    
    /* UNEQ: nan == nan (false for NaN) */
    if (!(nan == nan)) {
        accumulate(3);
    }
    
    /* LTGT: inf != nan (true) */
    if (inf != nan) {
        accumulate(4);
    }
    
    /* UNLT: nan < one */
    if (!(nan < one)) {
        accumulate(5);
    }
    
    /* UNGT: nan > one */
    if (!(nan > one)) {
        accumulate(6);
    }
    
    /* UNLE: nan <= one */
    if (!(nan <= one)) {
        accumulate(7);
    }
    
    /* UNGE: nan >= one */
    if (!(nan >= one)) {
        accumulate(8);
    }
    
    /* ORDERED: zero < inf (true and ordered) */
    if (zero < inf) {
        accumulate(9);
    }
}

/* Test using GCC built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* __builtin_isunordered - maps to UNORDERED */
    if (__builtin_isunordered(nan, inf)) {
        accumulate(10);
    }
    
    /* __builtin_islessgreater - maps to LTGT */
    if (__builtin_islessgreater(inf, nan)) {
        accumulate(11);
    }
    
    /* __builtin_isless - ordered less than */
    if (__builtin_isless(zero, inf)) {
        accumulate(12);
    }
    
    /* __builtin_isgreater - ordered greater than */
    if (__builtin_isgreater(inf, zero)) {
        accumulate(13);
    }
    
    /* __builtin_islessequal - ordered less or equal */
    if (__builtin_islessequal(zero, zero)) {
        accumulate(14);
    }
    
    /* __builtin_isgreaterequal - ordered greater or equal */
    if (__builtin_isgreaterequal(inf, zero)) {
        accumulate(15);
    }
    
    /* Complex expression using ternary operator */
    int result = __builtin_isunordered(nan, one) ? 
                 (__builtin_islessgreater(one, nan) ? 1 : 2) : 0;
    accumulate(16 + result);
}

/* Test vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {1.0f, __builtin_nanf(""), 2.0f, 3.0f};
    
    /* Vector comparison - may generate multiple condition checks */
    v4sf cmp_result = vec_a > vec_b;
    
    /* Extract comparison mask to force code generation */
    int mask;
#if defined(__SSE__)
    mask = __builtin_ia32_movmskps(cmp_result);
#else
    /* Fallback: store to memory and check */
    float mem_result[4];
    memcpy(mem_result, &cmp_result, sizeof(cmp_result));
    mask = (mem_result[0] != 0) | ((mem_result[1] != 0) << 1) |
           ((mem_result[2] != 0) << 2) | ((mem_result[3] != 0) << 3);
#endif
    
    accumulate(20 + mask);
    
    /* Double precision vector test */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {1.0, __builtin_nan("")};
    v2df cmp_dresult = vec_da < vec_db;
    
    double dmem_result[2];
    memcpy(dmem_result, &cmp_dresult, sizeof(cmp_dresult));
    if (dmem_result[0] == 0.0 && dmem_result[1] == 0.0) {
        accumulate(30);
    }
}

/* Test mixed-type comparisons and arithmetic */
static void test_mixed_types(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile double d_nan = __builtin_nan("");
    volatile long double ld_nan = __builtin_nanl("");
    
    volatile float f_inf = __builtin_inff();
    volatile double d_inf = __builtin_inf();
    
    /* Cross-type comparisons */
    if (!(f_nan < d_inf)) {
        accumulate(40);
    }
    
    if (!(d_nan == ld_nan)) {
        accumulate(41);
    }
    
    /* Arithmetic that produces NaN */
    volatile double a = d_inf;
    volatile double b = d_inf;
    volatile double c = a - b;  /* inf - inf = NaN */
    
    if (__builtin_isunordered(c, 0.0)) {
        accumulate(42);
    }
    
    /* Division by zero producing inf */
    volatile double d = 1.0;
    volatile double e = 0.0;
    volatile double f = d / e;  /* 1.0 / 0.0 = inf */
    
    if (f > 1000.0) {
        accumulate(43);
    }
    
    /* Using fma with NaN input */
    volatile double g = __builtin_fma(d_nan, 2.0, 3.0);
    if (__builtin_isunordered(g, g)) {
        accumulate(44);
    }
}

/* Inline assembly with explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = 1.0;
    volatile double c = __builtin_inf();
    
    int result_p, result_z, result_c;
    
    /* Using ucomisd - sets parity flag for unordered */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result_p)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    accumulate(50 + result_p);
    
    /* Compare inf with nan */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result_z)
        : "x"(c), "x"(a)
        : "cc"
    );
    
    accumulate(60 + result_z);
    
    /* Using fucomi instruction */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %0"
        : "=r"(result_c)
        : "t"(b), "t"(c)
        : "cc"
    );
    
    accumulate(70 + result_c);
}

/* Control flow driven by unordered results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {nan, inf, 1.0, 0.0};
    
    int results[4];
    
    /* Generate various condition codes through comparisons */
    for (int i = 0; i < 4; i++) {
        if (__builtin_isunordered(vals[i], vals[(i+1)%4])) {
            results[i] = 1;  /* UNORDERED */
        } else if (__builtin_islessgreater(vals[i], vals[(i+1)%4])) {
            results[i] = 2;  /* LTGT */
        } else if (vals[i] == vals[(i+1)%4]) {
            results[i] = 3;  /* EQ (but with potential UNEQ) */
        } else {
            results[i] = 0;
        }
    }
    
    /* Switch on combined results */
    int combined = results[0] + results[1] * 4 + results[2] * 16 + results[3] * 64;
    
    switch (combined & 7) {
        case 0:
            accumulate(100);
            break;
        case 1:
            accumulate(101);  /* UNORDERED path */
            break;
        case 2:
            accumulate(102);  /* LTGT path */
            break;
        case 3:
            accumulate(103);  /* EQ/UNEQ path */
            break;
        default:
            accumulate(104);
            break;
    }
}

#endif /* __x86_64__ || __i386__ */

int main(void) {
#ifdef __x86_64__ || __i386__
    /* Run all tests to trigger various condition codes */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_types();
    test_inline_asm();
    test_control_flow();
    
    printf("Checksum: %u\n", checksum);
    return 0;
#else
    printf("x86-specific tests skipped on non-x86 platform\n");
    return 0;
#endif
}
