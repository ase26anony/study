/* test_fp_conditions.c - Target GCC's i386 floating-point condition code generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__) || defined(__i686__)

/* Function to prevent optimization */
static volatile int global_counter = 0;

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test scalar unordered comparisons with operators */
void test_scalar_operators(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    
    /* These should generate various condition codes */
    if (nan < inf)   checksum ^= 1;   /* UNORDERED case */
    if (nan == nan)  checksum ^= 2;   /* UNEQ case */
    if (inf != nan)  checksum ^= 4;   /* LTGT case */
    if (nan <= inf)  checksum ^= 8;   /* UNLE case */
    if (nan >= inf)  checksum ^= 16;  /* UNGE case */
    if (nan > inf)   checksum ^= 32;  /* UNGT case */
    if (inf < nan)   checksum ^= 64;  /* UNLT case */
    
    /* Ordered comparisons */
    if (one < inf)   checksum ^= 128; /* ORDERED case */
    if (zero > -inf) checksum ^= 256;
    
    /* Complex expressions that might produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    if (inf_minus_inf == nan) checksum ^= 512;
    if (zero_div_zero != one) checksum ^= 1024;
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_inf = __builtin_infl();
    
    if ((double)f_nan < (double)ld_inf) checksum ^= 2048;
    if (f_nan == (float)nan) checksum ^= 4096;
}

/* Test built-in unordered comparison functions */
void test_builtin_functions(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double one = 1.0;
    
    /* Direct use of built-ins that map to condition codes */
    int r1 = __builtin_isunordered(nan, inf);   /* UNORDERED */
    int r2 = __builtin_islessgreater(nan, one); /* LTGT */
    int r3 = __builtin_isless(nan, inf);        /* UNLT */
    int r4 = __builtin_isgreater(inf, nan);     /* UNGT */
    int r5 = __builtin_islessequal(one, nan);   /* UNLE */
    int r6 = __builtin_isgreaterequal(nan, one);/* UNGE */
    
    checksum += r1 + (r2 << 1) + (r3 << 2) + (r4 << 3) + (r5 << 4) + (r6 << 5);
    
    /* Nested built-ins in conditional expressions */
    if (__builtin_isunordered(nan, inf) && __builtin_islessgreater(one, nan)) {
        checksum ^= 0xABCD;
    }
    
    /* Ternary operator with built-ins */
    int result = __builtin_isunordered(inf, nan) ? 
                 __builtin_isless(one, nan) : 
                 __builtin_isgreaterequal(nan, one);
    checksum += result;
}

/* Test vector comparisons using GCC extensions */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    
    /* Vector comparisons that may generate multiple condition checks */
    v4sf cmp_gt = vec_a > vec_b;   /* May generate UNGT/UNLT */
    v4sf cmp_lt = vec_a < vec_b;   /* May generate UNLT/UNGT */
    v4sf cmp_eq = vec_a == vec_b;  /* May generate UNEQ */
    v4sf cmp_ne = vec_a != vec_b;  /* May generate LTGT */
    
    /* Extract comparison masks */
    int mask_gt = __builtin_ia32_movmskps(cmp_gt);
    int mask_lt = __builtin_ia32_movmskps(cmp_lt);
    int mask_eq = __builtin_ia32_movmskps(cmp_eq);
    int mask_ne = __builtin_ia32_movmskps(cmp_ne);
    
    checksum += mask_gt + (mask_lt << 4) + (mask_eq << 8) + (mask_ne << 12);
    
    /* Double precision vector comparisons */
    v2df vec_da = {__builtin_nan(""), __builtin_inf()};
    v2df vec_db = {__builtin_inf(), __builtin_nan("")};
    
    v2df cmp_d = vec_da > vec_db;
    /* Access elements to force computation */
    double d0 = cmp_d[0];
    double d1 = cmp_d[1];
    checksum += (int)(d0 * 1000) + (int)(d1 * 1000);
}

/* Test mixed-type arithmetic and comparisons */
void test_mixed_arithmetic(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double zero = 0.0;
    
    /* Arithmetic that produces NaN */
    volatile double expr1 = (inf * zero) + nan;
    volatile double expr2 = (inf / inf) - (zero / zero);
    volatile double expr3 = __builtin_fma(nan, inf, zero);
    
    /* Comparisons after arithmetic */
    if (expr1 < expr2) checksum ^= 0x1111;  /* UNORDERED/UNLT */
    if (expr2 == expr3) checksum ^= 0x2222; /* UNEQ */
    if (expr1 != expr2) checksum ^= 0x3333; /* LTGT */
    if (expr3 >= expr1) checksum ^= 0x4444; /* UNGE */
    
    /* Long double comparisons */
    volatile long double ld_nan = __builtin_nanl("");
    volatile long double ld_inf = __builtin_infl();
    
    if (ld_nan < ld_inf) checksum ^= 0x5555;
    if (ld_nan == ld_nan) checksum ^= 0x6666;
    
    /* Mixed float/double/long double */
    if ((float)nan < (double)ld_inf) checksum ^= 0x7777;
    if ((long double)inf != (double)nan) checksum ^= 0x8888;
}

/* Inline assembly with explicit condition codes */
void test_inline_asm(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = 0.0;
    
    int result1, result2, result3, result4;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* fucomi with conditional moves */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %0"
        : "=r"(result2)
        : "t"(c), "u"(d)
        : "cc"
    );
    
    /* Multiple comparisons in sequence */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0\n\t"
        "ucomisd %4, %3\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result3)
        : "x"(a), "x"(c), "x"(b), "x"(d)
        : "cc", "al"
    );
    
    checksum += result1 + (result2 << 1) + (result3 << 2);
}

/* Control flow based on unordered comparison results */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double vals[] = {1.0, 2.0, nan, inf, -inf};
    
    /* Switch-like behavior using comparison results */
    for (int i = 0; i < 5; i++) {
        volatile double x = vals[i];
        volatile double y = vals[(i + 1) % 5];
        
        /* Complex conditional that requires multiple condition codes */
        if (__builtin_isunordered(x, y)) {
            checksum ^= (i << 0);  /* UNORDERED */
        } else if (__builtin_islessgreater(x, y)) {
            checksum ^= (i << 4);  /* LTGT */
        } else if (__builtin_isless(x, y)) {
            checksum ^= (i << 8);  /* UNLT */
        } else if (__builtin_isgreater(x, y)) {
            checksum ^= (i << 12); /* UNGT */
        } else if (__builtin_islessequal(x, y)) {
            checksum ^= (i << 16); /* UNLE */
        } else if (__builtin_isgreaterequal(x, y)) {
            checksum ^= (i << 20); /* UNGE */
        } else {
            checksum ^= (i << 24); /* UNEQ or ORDERED */
        }
    }
    
    /* Nested conditionals */
    volatile double a = nan;
    volatile double b = inf;
    volatile double c = 0.0;
    
    if (a < b) {
        if (b > c) {
            if (a != c) {
                if (c == c) {
                    checksum ^= 0xDEAD;
                }
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Run all tests */
    test_scalar_operators();
    test_builtin_functions();
    test_vector_comparisons();
    test_mixed_arithmetic();
    test_inline_asm();
    test_control_flow();
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %u\n", checksum);
    global_counter = checksum;
    
    return (int)checksum & 0xFF;
}

#else /* Non-x86 target */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}
#endif
