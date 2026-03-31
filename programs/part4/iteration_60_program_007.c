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
    
    /* Complex expression that may require multiple condition codes */
    volatile double result = (inf - inf);  /* Creates NaN */
    if ((result < one) || (result > zero)) {
        accumulate(5);
    }
}

/* Test using GCC built-in unordered comparison functions */
static void test_builtin_functions(void) {
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile float f_one = 1.0f;
    
    /* __builtin_isunordered() - should generate UNORDERED condition */
    if (__builtin_isunordered(f_nan, f_inf)) {
        accumulate(10);
    }
    
    /* __builtin_islessgreater() - should generate LTGT condition */
    if (__builtin_islessgreater(f_one, f_nan)) {
        accumulate(11);
    }
    
    /* __builtin_isless() - should generate UNLT or similar */
    if (!__builtin_isless(f_nan, f_one)) {
        accumulate(12);
    }
    
    /* __builtin_isgreater() - should generate UNGT or similar */
    if (!__builtin_isgreater(f_nan, f_one)) {
        accumulate(13);
    }
    
    /* __builtin_islessequal() - should generate UNLE */
    if (!__builtin_islessequal(f_nan, f_one)) {
        accumulate(14);
    }
    
    /* __builtin_isgreaterequal() - should generate UNGE */
    if (!__builtin_isgreaterequal(f_nan, f_one)) {
        accumulate(15);
    }
    
    /* Nested built-ins to force multiple condition code evaluations */
    if (__builtin_isunordered(f_nan, f_inf) || __builtin_islessgreater(f_one, f_nan)) {
        accumulate(16);
    }
}

/* Test vector comparisons using GCC extensions */
static void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, 2.0f, __builtin_inff()};
    v4sf vec_b = {__builtin_inff(), __builtin_nanf(""), 2.0f, 1.0f};
    
    /* Vector comparison - may generate multiple condition code checks */
    v4sf cmp_result = vec_a > vec_b;
    
    /* Extract comparison mask to force code generation */
    int mask;
    #ifdef __SSE__
    mask = __builtin_ia32_movmskps(cmp_result);
    #else
    /* Fallback: store to memory and check */
    float result_array[4];
    memcpy(result_array, &cmp_result, sizeof(result_array));
    mask = (result_array[0] != 0) | ((result_array[1] != 0) << 1) |
           ((result_array[2] != 0) << 2) | ((result_array[3] != 0) << 3);
    #endif
    
    accumulate(mask);
    
    /* Double precision vector comparison */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    v2df dbl_cmp = vec_da < vec_db;
    
    /* Force use of the result */
    double dbl_result[2];
    memcpy(dbl_result, &dbl_cmp, sizeof(dbl_result));
    accumulate((int)dbl_result[0] + (int)dbl_result[1]);
}

/* Test mixed-type comparisons and arithmetic */
static void test_mixed_types(void) {
    volatile long double ld_nan = __builtin_nanl("");
    volatile double d_inf = __builtin_inf();
    volatile float f_val = 2.0f;
    
    /* Mixed type comparison */
    if (ld_nan < d_inf) {
        accumulate(20);
    }
    
    /* Arithmetic that produces NaN */
    volatile double nan_prod = d_inf * 0.0;
    if (nan_prod != nan_prod) {  /* UNEQ check */
        accumulate(21);
    }
    
    /* FMA with NaN input */
    volatile double fma_result = __builtin_fma(__builtin_nan(""), 2.0, 3.0);
    if (fma_result == fma_result) {  /* Should be false for NaN */
        accumulate(22);
    }
    
    /* Division by zero producing infinity */
    volatile double inf_div = 1.0 / 0.0;
    if (inf_div > __builtin_nan("")) {  /* UNGT check */
        accumulate(23);
    }
}

/* Test with inline assembly for explicit condition codes */
static void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    
    int result;
    
    /* Using ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    accumulate(result);
    
    /* Using fucomi with conditional moves */
    double asm_result;
    asm volatile (
        "fucomi %2, %1\n\t"
        "jp 1f\n\t"           /* Jump if unordered */
        "mov $0, %0\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "mov $1, %0\n\t"
        "2:"
        : "=r"(asm_result)
        : "t"(c), "u"(a)
        : "cc"
    );
    accumulate((int)asm_result);
}

/* Control flow driven by unordered comparison results */
static void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double vals[] = {1.0, 2.0, __builtin_inf(), nan};
    int results[4];
    
    /* Switch based on comparison results */
    for (int i = 0; i < 4; i++) {
        int code = 0;
        
        /* Build a condition code from multiple comparisons */
        if (__builtin_isunordered(vals[i], nan)) code |= 1;
        if (__builtin_islessgreater(vals[i], 1.5)) code |= 2;
        if (!__builtin_isless(vals[i], 3.0)) code |= 4;
        if (__builtin_isgreaterequal(vals[i], 0.0)) code |= 8;
        
        switch (code) {
            case 0: results[i] = 0; break;
            case 1: results[i] = 1; break;  /* UNORDERED */
            case 2: results[i] = 2; break;  /* LTGT */
            case 3: results[i] = 3; break;  /* Combined */
            case 4: results[i] = 4; break;  /* UNGE/UNGT */
            case 5: results[i] = 5; break;
            case 6: results[i] = 6; break;
            case 7: results[i] = 7; break;
            default: results[i] = 8; break;
        }
    }
    
    /* Use results to prevent elimination */
    for (int i = 0; i < 4; i++) {
        accumulate(results[i]);
    }
}

/* Main test driver for x86 */
int main(void) {
    printf("Testing x86 floating-point unordered condition codes...\n");
    
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_types();
    test_inline_asm();
    test_control_flow();
    
    printf("Checksum: %u\n", (unsigned int)checksum);
    return (int)checksum;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
