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

/* Checksum to prevent dead code elimination */
static uint32_t checksum = 0;

/* Helper to update checksum */
static void update_checksum(int value) {
    checksum = (checksum << 3) ^ (checksum >> 29) ^ (uint32_t)value;
}

/* Test function with various unordered comparisons */
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
    results[idx++] = (nan < inf) ? 1 : 0;        /* UNORDERED */
    results[idx++] = (nan > inf) ? 1 : 0;        /* UNORDERED */
    results[idx++] = (nan <= inf) ? 1 : 0;       /* UNORDERED */
    results[idx++] = (nan >= inf) ? 1 : 0;       /* UNORDERED */
    results[idx++] = (nan == nan) ? 1 : 0;       /* UNORDERED/UNEQ */
    results[idx++] = (nan != nan) ? 1 : 0;       /* ORDERED/LTGT */
    results[idx++] = (inf != nan) ? 1 : 0;       /* ORDERED */
    results[idx++] = (inf == inf) ? 1 : 0;       /* ORDERED/EQ */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);    /* UNORDERED */
    results[idx++] = !__builtin_isunordered(inf, one);   /* ORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);  /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, nan);  /* UNORDERED */
    results[idx++] = __builtin_islessgreater(inf, neg_inf); /* ORDERED/LTGT */
    
    results[idx++] = __builtin_isless(nan, inf);         /* UNORDERED */
    results[idx++] = __builtin_isless(neg_inf, inf);     /* ORDERED/LT */
    
    results[idx++] = __builtin_isgreater(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_isgreater(inf, neg_inf);  /* ORDERED/GT */
    
    results[idx++] = __builtin_islessequal(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_islessequal(neg_inf, inf); /* ORDERED/LE */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNORDERED */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf); /* ORDERED/GE */
    
    /* 3. Complex expressions with arithmetic that could produce NaN */
    volatile double nan_prod = zero / zero;
    volatile double inf_minus_inf = inf - inf;
    
    results[idx++] = (nan_prod < one) ? 1 : 0;           /* UNORDERED */
    results[idx++] = (inf_minus_inf == nan_prod) ? 1 : 0; /* UNORDERED/UNEQ */
    results[idx++] = (inf_minus_inf != nan_prod) ? 1 : 0; /* UNORDERED/LTGT */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;       /* UNORDERED */
    results[idx++] = ((double)ld_nan == nan) ? 1 : 0;    /* UNORDERED/UNEQ */
    
    /* 5. Ternary operators with unordered comparisons */
    results[idx++] = __builtin_isunordered(nan, one) ? 
                     __builtin_isless(neg_one, zero) : 
                     __builtin_isgreater(one, zero);
    
    results[idx++] = !__builtin_isunordered(inf, neg_inf) ? 
                     __builtin_islessequal(zero, one) : 
                     __builtin_isgreaterequal(one, zero);
    
    /* Update checksum with all results */
    for (int i = 0; i < idx; i++) {
        update_checksum(results[i]);
    }
}

/* Test function with vector comparisons */
void test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that may generate unordered condition codes */
    v4sf cmp_result;
    
    /* Greater than - may generate UNORDERED/UNGT/UNLE etc. */
    cmp_result = vec_a > vec_b;
    
    /* Less than */
    cmp_result = vec_a < vec_b;
    
    /* Equal */
    cmp_result = vec_a == vec_b;
    
    /* Not equal */
    cmp_result = vec_a != vec_b;
    
    /* Extract comparison mask */
    int mask;
    #ifdef __SSE__
    mask = __builtin_ia32_movmskps(cmp_result);
    #else
    /* Fallback: store to memory and check */
    float mem[4];
    memcpy(mem, &cmp_result, sizeof(mem));
    mask = (mem[0] != 0.0f) | ((mem[1] != 0.0f) << 1) | 
           ((mem[2] != 0.0f) << 2) | ((mem[3] != 0.0f) << 3);
    #endif
    
    update_checksum(mask);
    
    /* Double precision vector comparisons */
    v2df vec_d = {__builtin_nan(""), __builtin_inf()};
    v2df vec_e = {__builtin_inf(), __builtin_nan("")};
    
    v2df dbl_cmp = vec_d > vec_e;
    
    #ifdef __SSE2__
    int dbl_mask = __builtin_ia32_movmskpd(dbl_cmp);
    #else
    double dbl_mem[2];
    memcpy(dbl_mem, &dbl_cmp, sizeof(dbl_mem));
    int dbl_mask = (dbl_mem[0] != 0.0) | ((dbl_mem[1] != 0.0) << 1);
    #endif
    
    update_checksum(dbl_mask);
}

/* Test function with inline assembly */
void test_asm_comparisons(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result_p, result_np, result_z, result_nz;
    
    /* Inline assembly with explicit condition codes */
    #if defined(__x86_64__)
    /* Using ucomisd for unordered comparisons */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzx %%al, %0"
        : "=r" (result_p)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzx %%al, %0"
        : "=r" (result_np)
        : "x" (c), "x" (d)
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setz %%al\n\t"
        "movzx %%al, %0"
        : "=r" (result_z)
        : "x" (b), "x" (b)  /* inf == inf */
        : "al", "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnz %%al\n\t"
        "movzx %%al, %0"
        : "=r" (result_nz)
        : "x" (a), "x" (c)  /* nan != 1.0 */
        : "al", "cc"
    );
    #endif
    
    update_checksum(result_p);
    update_checksum(result_np);
    update_checksum(result_z);
    update_checksum(result_nz);
}

/* Control flow based on unordered comparisons */
void test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, -inf, 0.0, 1.0, -1.0};
    
    int switch_var = 0;
    
    /* Build switch value from comparison results */
    if (__builtin_isunordered(nan, inf)) switch_var |= 1;
    if (!__builtin_isunordered(inf, 1.0)) switch_var |= 2;
    if (__builtin_islessgreater(inf, -inf)) switch_var |= 4;
    if (__builtin_isless(nan, inf)) switch_var |= 8;
    if (__builtin_isgreater(inf, nan)) switch_var |= 16;
    
    /* Switch statement that forces generation of condition codes */
    switch (switch_var & 0x1F) {
        case 0:  /* ORDERED, EQ, etc. */
            sink = 0;
            break;
        case 1:  /* UNORDERED */
            sink = 1;
            break;
        case 3:  /* ORDERED, LT */
            sink = 2;
            break;
        case 5:  /* UNORDERED, UNEQ */
            sink = 3;
            break;
        case 7:  /* ORDERED, LTGT */
            sink = 4;
            break;
        case 9:  /* UNORDERED, UNLT */
            sink = 5;
            break;
        case 11: /* UNORDERED, UNLE */
            sink = 6;
            break;
        case 13: /* UNORDERED, UNGT */
            sink = 7;
            break;
        case 15: /* UNORDERED, UNGE */
            sink = 8;
            break;
        default:
            sink = 9;
            break;
    }
    
    update_checksum(sink);
    
    /* Loop with unordered comparison as condition */
    int count = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (__builtin_isunordered(values[i], values[j])) {
                count++;
            }
        }
    }
    
    update_checksum(count);
}

int main(void) {
    /* Initialize checksum */
    checksum = 0xDEADBEEF;
    
    /* Run all tests */
    test_unordered_comparisons();
    test_vector_comparisons();
    test_asm_comparisons();
    test_control_flow();
    
    /* Print checksum to prevent optimization */
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}

#else /* Non-x86 target */

/* Minimal fallback for non-x86 targets */
int main(void) {
    printf("This test is for x86 targets only.\n");
    return 0;
}

#endif
