#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static void use_result(int result) {
    volatile static int sink;
    sink = result;
}

/* Function to create complex NaN-producing expressions */
static double create_nan_expr(double a, double b) {
    return (a * b) / (a - b);  /* Could produce NaN if a == b */
}

int main(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double two = 2.0;
    
    int results[32] = {0};
    int idx = 0;
    
    /* ===== 1. Explicit unordered floating-point comparisons ===== */
    
    /* UNORDERED cases - comparisons involving NaN */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED/UNLT */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED/UNGT */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED/UNLE */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED/UNGE */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[idx++] = (nan != nan) ? 1 : 0;     /* ORDERED/LTGT */
    
    /* ORDERED cases - comparisons without NaN */
    results[idx++] = (inf < neg_inf) ? 1 : 0;  /* ORDERED/LT */
    results[idx++] = (inf > neg_inf) ? 1 : 0;  /* ORDERED/GT */
    results[idx++] = (zero == zero) ? 1 : 0;   /* ORDERED/EQ */
    
    /* Mixed comparisons */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/NEQ */
    results[idx++] = (one < two) ? 1 : 0;      /* ORDERED/LT */
    results[idx++] = (two > one) ? 1 : 0;      /* ORDERED/GT */
    
    /* ===== 2. Built-in unordered comparison functions ===== */
    
    /* These built-ins directly map to condition codes */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, inf);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* UNLT */
    results[idx++] = __builtin_isgreater(nan, inf);        /* UNGT */
    results[idx++] = __builtin_islessequal(nan, inf);      /* UNLE */
    results[idx++] = __builtin_isgreaterequal(nan, inf);   /* UNGE */
    
    /* Ordered comparisons using built-ins */
    results[idx++] = __builtin_isless(one, two);           /* LT */
    results[idx++] = __builtin_isgreater(two, one);        /* GT */
    results[idx++] = __builtin_islessequal(one, one);      /* LE */
    results[idx++] = __builtin_isgreaterequal(two, two);   /* GE */
    
    /* UNEQ through combination */
    results[idx++] = !__builtin_isunordered(inf, inf) && 
                     (inf == inf);                         /* UNEQ */
    
    /* ===== 3. Vector comparisons with GCC extensions ===== */
    
    v4sf vec_a = {nan, inf, 1.0f, 2.0f};
    v4sf vec_b = {inf, nan, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector comparisons generate multiple condition checks */
    v4sf cmp_result = vec_a > vec_b;      /* UNGT/GT mixed */
    v4sf cmp_result2 = vec_a < vec_b;     /* UNLT/LT mixed */
    v4sf cmp_result3 = vec_a == vec_b;    /* UNEQ/EQ mixed */
    
    /* Extract comparison masks */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    int mask2 = __builtin_ia32_movmskps(cmp_result2);
    int mask3 = __builtin_ia32_movmskps(cmp_result3);
    
    results[idx++] = mask1;
    results[idx++] = mask2;
    results[idx++] = mask3;
    
    /* Double precision vector comparisons */
    v2df vec_da = {nan, inf};
    v2df vec_db = {inf, nan};
    v2df cmp_dresult = vec_da > vec_db;
    
    /* Store to memory and check individual elements */
    double mem_result[2];
    memcpy(mem_result, &cmp_dresult, sizeof(cmp_dresult));
    results[idx++] = (mem_result[0] != 0.0) ? 1 : 0;
    results[idx++] = (mem_result[1] != 0.0) ? 1 : 0;
    
    /* ===== 4. Mixed-type comparisons and arithmetic ===== */
    
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    volatile float f_inf = __builtin_inff();
    
    /* Cross-type comparisons */
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;         /* UNLT */
    results[idx++] = ((double)ld_nan > inf) ? 1 : 0;       /* UNGT */
    
    /* NaN-producing arithmetic followed by comparison */
    volatile double expr1 = create_nan_expr(inf, inf);     /* inf - inf = NaN */
    volatile double expr2 = zero / zero;                   /* 0/0 = NaN */
    volatile double expr3 = inf * zero;                    /* inf * 0 = NaN */
    
    results[idx++] = (expr1 < expr2) ? 1 : 0;              /* UNORDERED */
    results[idx++] = (expr2 == expr3) ? 1 : 0;             /* UNORDERED/UNEQ */
    results[idx++] = (expr1 != expr3) ? 1 : 0;             /* ORDERED/LTGT */
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, one, two);
    results[idx++] = (fma_result > two) ? 1 : 0;           /* UNORDERED/UNGT */
    
    /* ===== 5. Inline assembly with explicit condition codes ===== */
    
    double a = nan;
    double b = inf;
    int asm_result = 0;
    
    /* Using ucomisd with explicit condition code checks */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"          /* UNORDERED - parity flag */
        "sete %%bl\n\t"          /* ORDERED/EQ */
        "setb %%cl\n\t"          /* ORDERED/LT */
        "seta %%dl\n\t"          /* ORDERED/GT */
        "orb %%al, %%bl\n\t"     /* Combine for UNEQ */
        "movzbl %%bl, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "al", "bl", "cl", "dl", "cc"
    );
    
    results[idx++] = asm_result;
    
    /* Another assembly block for different condition */
    asm volatile (
        "ucomisd %1, %0\n\t"
        "setne %%al\n\t"         /* ORDERED/NEQ or UNORDERED/LTGT */
        "movzbl %%al, %0"
        : "+r" (asm_result)
        : "x" (one), "x" (two)
        : "al", "cc"
    );
    
    results[idx++] = asm_result;
    
    /* ===== 6. Control flow driven by unordered results ===== */
    
    int switch_var = 0;
    
    /* Build a value based on comparison results */
    if (__builtin_isunordered(nan, inf)) switch_var |= 1;      /* UNORDERED */
    if (__builtin_islessgreater(inf, nan)) switch_var |= 2;    /* LTGT */
    if (!__builtin_isunordered(one, two) && one < two) switch_var |= 4; /* LT */
    if (nan == nan) switch_var |= 8;                           /* UNORDERED/UNEQ */
    if (inf >= neg_inf) switch_var |= 16;                      /* ORDERED/GE */
    
    /* Switch statement that depends on comparison results */
    switch (switch_var & 0x7) {
        case 0:  /* All comparisons false */
            results[idx++] = 100;
            break;
        case 1:  /* Only UNORDERED true */
            results[idx++] = 101;
            break;
        case 2:  /* Only LTGT true */
            results[idx++] = 102;
            break;
        case 3:  /* UNORDERED and LTGT true */
            results[idx++] = 103;
            break;
        case 4:  /* Only LT true */
            results[idx++] = 104;
            break;
        case 5:  /* UNORDERED and LT true */
            results[idx++] = 105;
            break;
        case 6:  /* LTGT and LT true */
            results[idx++] = 106;
            break;
        case 7:  /* All three true */
            results[idx++] = 107;
            break;
    }
    
    /* Loop controlled by comparison results */
    int loop_count = 0;
    for (int i = 0; i < 10; i++) {
        volatile double x = (i & 1) ? nan : (double)i;
        volatile double y = (i & 2) ? inf : (double)(i * 2);
        
        if (__builtin_isunordered(x, y)) {
            loop_count++;
        } else if (x < y) {
            loop_count += 2;
        } else if (x > y) {
            loop_count += 3;
        }
    }
    results[idx++] = loop_count;
    
    /* ===== Final checksum to prevent dead code elimination ===== */
    
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
        use_result(results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Total tests: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;

#else
    /* Non-x86 fallback */
    int main(void) {
        printf("This test is for x86 architecture only.\n");
        return 0;
    }
#endif
}
