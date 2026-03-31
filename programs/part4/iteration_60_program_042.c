#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Checksum to prevent dead code elimination */
static volatile int checksum = 0;

/* Function to use comparison results in non-trivial ways */
void use_result(int cond) {
    checksum ^= cond;
}

/* Function with complex control flow based on comparisons */
int complex_decision(double a, double b, double c) {
    int result = 0;
    
    /* Multiple unordered comparisons in control flow */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    if (__builtin_islessgreater(a, c)) {
        result |= 2;
    }
    
    if (!__builtin_islessequal(b, c)) {
        result |= 4;
    }
    
    return result;
}

int main() {
#ifdef __x86_64__ || __i386__
    /* Volatile to prevent optimization */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double two = 2.0;
    
    /* Array to store comparison results */
    int results[32];
    int idx = 0;
    
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    /* ===== 1. EXPLICIT UNORDERED FLOATING-POINT COMPARISONS ===== */
    
    /* UNORDERED cases: comparisons involving NaN */
    results[idx++] = (nan < inf) ? 1 : 0;          /* Should be false (unordered) */
    results[idx++] = (nan > inf) ? 1 : 0;          /* Should be false (unordered) */
    results[idx++] = (nan <= inf) ? 1 : 0;         /* Should be false (unordered) */
    results[idx++] = (nan >= inf) ? 1 : 0;         /* Should be false (unordered) */
    
    /* UNEQ: unordered or equal */
    results[idx++] = (nan == nan) ? 1 : 0;         /* Should be false (NaN != NaN) */
    results[idx++] = (inf == inf) ? 1 : 0;         /* Should be true */
    
    /* LTGT: less, greater, or unordered (but not equal) */
    results[idx++] = (nan != nan) ? 1 : 0;         /* Should be true (NaN != NaN) */
    results[idx++] = (inf != neg_inf) ? 1 : 0;     /* Should be true */
    
    /* ORDERED comparisons */
    results[idx++] = (inf < neg_inf) ? 1 : 0;      /* Should be false */
    results[idx++] = (inf > neg_inf) ? 1 : 0;      /* Should be true */
    results[idx++] = (one <= two) ? 1 : 0;         /* Should be true */
    results[idx++] = (two >= one) ? 1 : 0;         /* Should be true */
    
    /* ===== 2. BUILT-IN UNORDERED COMPARISON FUNCTIONS ===== */
    
    /* Direct use of built-ins that map to condition codes */
    results[idx++] = __builtin_isunordered(nan, inf);
    results[idx++] = __builtin_isunordered(inf, nan);
    results[idx++] = __builtin_isunordered(nan, nan);
    results[idx++] = __builtin_isunordered(inf, inf);
    
    results[idx++] = __builtin_islessgreater(nan, inf);
    results[idx++] = __builtin_islessgreater(inf, nan);
    results[idx++] = __builtin_islessgreater(one, two);
    results[idx++] = __builtin_islessgreater(two, one);
    
    results[idx++] = __builtin_isless(nan, inf);
    results[idx++] = __builtin_isless(inf, nan);
    results[idx++] = __builtin_isless(one, two);
    results[idx++] = __builtin_isless(two, one);
    
    results[idx++] = __builtin_isgreater(nan, inf);
    results[idx++] = __builtin_isgreater(inf, nan);
    results[idx++] = __builtin_isgreater(two, one);
    results[idx++] = __builtin_isgreater(one, two);
    
    results[idx++] = __builtin_islessequal(nan, inf);
    results[idx++] = __builtin_islessequal(inf, nan);
    results[idx++] = __builtin_islessequal(one, two);
    results[idx++] = __builtin_islessequal(two, one);
    
    results[idx++] = __builtin_isgreaterequal(nan, inf);
    results[idx++] = __builtin_isgreaterequal(inf, nan);
    results[idx++] = __builtin_isgreaterequal(two, one);
    results[idx++] = __builtin_isgreaterequal(one, two);
    
    /* ===== 3. VECTOR COMPARISONS WITH GCC EXTENSIONS ===== */
    
#ifdef __SSE__
    typedef float v4sf __attribute__((vector_size(16)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v4sf vec_a = {nan, inf, 1.0f, 2.0f};
    v4sf vec_b = {inf, nan, 2.0f, 1.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4si cmp_result = vec_a > vec_b;
    v4si cmp_result2 = vec_a < vec_b;
    v4si cmp_result3 = vec_a == vec_b;
    v4si cmp_result4 = vec_a != vec_b;
    
    /* Extract results to force code generation */
    float temp[4];
    memcpy(temp, &cmp_result, sizeof(temp));
    results[idx++] = (int)temp[0];
    
    memcpy(temp, &cmp_result2, sizeof(temp));
    results[idx++] = (int)temp[1];
    
    memcpy(temp, &cmp_result3, sizeof(temp));
    results[idx++] = (int)temp[2];
    
    memcpy(temp, &cmp_result4, sizeof(temp));
    results[idx++] = (int)temp[3];
#endif
    
    /* ===== 4. MIXED-TYPE COMPARISONS AND ARITHMETIC ===== */
    
    /* Operations that can produce NaN */
    volatile double nan_prod = zero / zero;        /* Should produce NaN */
    volatile double inf_minus_inf = inf - inf;     /* Should produce NaN */
    volatile double inf_div_inf = inf / inf;       /* Should produce NaN */
    
    /* Mixed type comparisons */
    volatile float f_nan = (float)nan;
    volatile long double ld_inf = (long double)inf;
    
    results[idx++] = (f_nan < ld_inf) ? 1 : 0;
    results[idx++] = (nan_prod == inf_minus_inf) ? 1 : 0;
    results[idx++] = (inf_div_inf != nan_prod) ? 1 : 0;
    
    /* Complex expressions with math functions */
    results[idx++] = __builtin_isless(nan_prod * 2.0, inf_minus_inf + 1.0);
    results[idx++] = __builtin_isgreater(inf_div_inf / 2.0, nan_prod - 1.0);
    
    /* ===== 5. INLINE ASSEMBLY WITH EXPLICIT CONDITION CODES ===== */
    
#ifdef __x86_64__
    /* Inline assembly that uses specific x86 FP compare instructions */
    double a = 1.5;
    double b = 2.5;
    int result1, result2, result3;
    
    /* ucomisd with setp (parity flag for unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    /* ucomisd with seta (above) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result2)
        : "x"(b), "x"(a)  /* reversed for greater than */
        : "al", "cc"
    );
    
    /* ucomisd with setb (below) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    results[idx++] = result1;
    results[idx++] = result2;
    results[idx++] = result3;
#endif
    
    /* ===== 6. CONTROL FLOW DRIVEN BY UNORDERED RESULTS ===== */
    
    /* Switch statement based on comparison combinations */
    int switch_val = 0;
    
    if (__builtin_isunordered(nan, inf)) switch_val |= 1;
    if (__builtin_isless(one, two)) switch_val |= 2;
    if (__builtin_isgreater(inf, neg_inf)) switch_val |= 4;
    if (__builtin_islessgreater(nan, one)) switch_val |= 8;
    
    switch (switch_val) {
        case 0:
            results[idx++] = 100;
            break;
        case 1:
            results[idx++] = 101;  /* UNORDERED */
            break;
        case 2:
            results[idx++] = 102;  /* LESS */
            break;
        case 3:
            results[idx++] = 103;  /* UNORDERED | LESS */
            break;
        case 4:
            results[idx++] = 104;  /* GREATER */
            break;
        case 5:
            results[idx++] = 105;  /* UNORDERED | GREATER */
            break;
        case 6:
            results[idx++] = 106;  /* LESS | GREATER (LTGT) */
            break;
        case 7:
            results[idx++] = 107;
            break;
        case 8:
            results[idx++] = 108;  /* UNORDERED with LTGT */
            break;
        default:
            results[idx++] = 109;
            break;
    }
    
    /* Loop with comparison-dependent iteration */
    for (int i = 0; i < 10; i++) {
        volatile double x = (i % 2 == 0) ? nan : (double)i;
        volatile double y = (i % 3 == 0) ? inf : (double)(i * 2);
        
        if (__builtin_isunordered(x, y)) {
            results[idx++ % 32] += i;
        } else if (__builtin_isless(x, y)) {
            results[idx++ % 32] -= i;
        } else if (__builtin_isgreater(x, y)) {
            results[idx++ % 32] *= (i + 1);
        }
    }
    
    /* Update checksum with all results */
    for (int i = 0; i < 32 && i < idx; i++) {
        checksum += results[i];
    }
    
    /* Complex decision function calls */
    checksum += complex_decision(nan, inf, one);
    checksum += complex_decision(inf, nan, two);
    checksum += complex_decision(one, two, nan);
    checksum += complex_decision(zero, inf, neg_inf);
    
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons performed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
    
#else
    /* Non-x86 fallback */
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
#endif
}
