/* test_i386_float_conds.c
 * 
 * This program is designed to trigger the uncovered condition code
 * mnemonics in GCC's i386 backend for floating-point unordered comparisons.
 * It uses NaN and infinity values, built-in unordered comparison functions,
 * vector extensions, inline assembly, and control flow to ensure the
 * compiler generates assembly that exercises the UNORDERED, ORDERED,
 * UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT condition code strings.
 *
 * Compile with:
 *   gcc -O2 -march=x86-64 -ffast-math -fno-trapping-math -S -o test.s test_i386_float_conds.c
 *   or
 *   gcc -O3 -march=haswell -mavx2 -ftree-vectorize -fno-omit-frame-pointer -S -o test.s test_i386_float_conds.c
 */

#include <stdio.h>
#include <stdint.h>

/* Portable detection for x86 */
#if defined(__x86_64__) || defined(__i386__)

/* Function to prevent dead code elimination */
static volatile int global_counter = 0;

/* Checksum to aggregate results and force evaluation */
static uint32_t checksum = 0;

/* Helper to update checksum with any integer */
static void update_checksum(int val) {
    checksum ^= (uint32_t)val;
    checksum = (checksum << 1) | (checksum >> 31);
}

int main(void) {
    /* Volatile to prevent constant folding */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double two = 2.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Explicit unordered floating-point comparisons using operators */
    results[idx++] = (nan < inf);      /* UNORDERED likely */
    results[idx++] = (nan == nan);     /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan);     /* ORDERED/NEQ */
    results[idx++] = (nan > neg_inf);  /* UNORDERED */
    results[idx++] = (inf >= nan);     /* ORDERED/GE */
    results[idx++] = (nan <= one);     /* UNORDERED/UNLE */
    results[idx++] = (two >= nan);     /* ORDERED/UNGE? */
    results[idx++] = (nan == inf);     /* UNORDERED/UNEQ? */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);   /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, one); /* LTGT */
    results[idx++] = __builtin_isless(nan, two);        /* UNLT */
    results[idx++] = __builtin_isgreater(inf, nan);     /* UNGT */
    results[idx++] = __builtin_islessequal(nan, zero);  /* UNLE */
    results[idx++] = __builtin_isgreaterequal(two, nan);/* UNGE */
    results[idx++] = __builtin_isunordered(one, two);   /* ORDERED (false) */
    results[idx++] = __builtin_islessgreater(inf, neg_inf); /* LTGT (true) */
    
    /* 3. Vector comparisons using GCC extensions */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf vec_a = { __builtin_nanf(""), 1.0f, 2.0f, __builtin_inff() };
    v4sf vec_b = { 1.0f, __builtin_nanf(""), __builtin_inff(), 2.0f };
    v4sf vec_cmp = vec_a > vec_b;  /* Generates multiple condition checks */
    
    /* Extract comparison mask to force code generation */
    int mask = __builtin_ia32_movmskps((__v4sf)vec_cmp);
    results[idx++] = mask;
    
    /* 4. Mixed-type comparisons and arithmetic producing NaN */
    volatile float fnan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    volatile double d_inf = __builtin_inf();
    
    /* Arithmetic that may produce NaN */
    volatile double div_zero = one / zero;          /* inf */
    volatile double inf_minus_inf = inf - inf;      /* nan */
    volatile double nan_plus_one = nan + 1.0;
    
    results[idx++] = (fnan < d_inf);                /* UNORDERED */
    results[idx++] = (ld_nan == ld_nan);            /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != div_zero);   /* UNORDERED/NEQ */
    results[idx++] = (nan_plus_one > 0.0);          /* UNORDERED */
    
    /* Use __builtin_fma with NaN input */
    double fma_nan = __builtin_fma(nan, one, two);
    results[idx++] = (fma_nan == fma_nan);          /* UNORDERED/UNEQ */
    
    /* 5. Inline assembly with explicit condition codes */
    double a = nan;
    double b = 3.14;
    int asm_result = 0;
    
    /* ucomisd sets flags: ZF, PF, CF */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "eax", "cc"
    );
    results[idx++] = asm_result;  /* UNORDERED check (parity flag) */
    
    /* Another asm block for ordered comparison */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (one), "x" (two)
        : "eax", "cc"
    );
    results[idx++] = asm_result;  /* ORDERED check */
    
    /* 6. Control flow driven by unordered results */
    int switch_var = 0;
    if (__builtin_isunordered(nan, inf)) switch_var |= 1;
    if (__builtin_islessgreater(one, nan)) switch_var |= 2;
    if (__builtin_isless(nan, two)) switch_var |= 4;
    if (__builtin_isgreater(inf, nan)) switch_var |= 8;
    
    /* Switch ensures compiler may generate different condition code uses */
    switch (switch_var) {
        case 0:
            results[idx++] = 100;
            break;
        case 1:  /* UNORDERED true */
            results[idx++] = 101;
            break;
        case 2:  /* LTGT true with NaN */
            results[idx++] = 102;
            break;
        case 4:  /* UNLT true */
            results[idx++] = 103;
            break;
        case 8:  /* UNGT true */
            results[idx++] = 104;
            break;
        default:
            results[idx++] = 105;
            break;
    }
    
    /* Loop whose iterations depend on comparison results */
    for (int i = 0; i < 5; i++) {
        volatile double x = (i & 1) ? nan : (double)i;
        volatile double y = (i & 2) ? inf : (double)(i+1);
        if (__builtin_isunordered(x, y)) {
            results[idx++] = i * 10;
        } else if (__builtin_islessgreater(x, y)) {
            results[idx++] = i * 20;
        } else if (__builtin_isless(x, y)) {
            results[idx++] = i * 30;
        }
    }
    
    /* Aggregate all results into checksum to prevent elimination */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
    
    /* Also add vector mask and some constants */
    update_checksum(mask);
    update_checksum(asm_result);
    
    /* Print checksum so program has observable output */
    printf("Checksum: %u\n", checksum);
    
    return (int)(checksum & 0xFF);
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This program is intended for x86 targets.\n");
    return 0;
}
#endif
