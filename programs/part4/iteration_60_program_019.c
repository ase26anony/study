#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Portable feature detection */
#if defined(__x86_64__) || defined(__i386__)

/* Vector type for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function to prevent optimization */
static volatile int global_result = 0;

/* Test function that performs various unordered comparisons */
void test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators */
    /* These should generate UNORDERED/ORDERED condition codes */
    results[idx++] = (nan < inf) ? 1 : 0;          /* UNORDERED likely */
    results[idx++] = (nan == nan) ? 1 : 0;         /* UNORDERED/UNEQ */
    results[idx++] = (inf != nan) ? 1 : 0;         /* ORDERED/LTGT */
    results[idx++] = (nan > neg_inf) ? 1 : 0;      /* UNORDERED */
    results[idx++] = (inf >= nan) ? 1 : 0;         /* ORDERED/UNGE? */
    
    /* Complex expressions that might produce NaN */
    volatile double nan_producer = (inf - inf);
    results[idx++] = (nan_producer == zero) ? 1 : 0;  /* UNORDERED/UNEQ */
    results[idx++] = (zero / zero == nan) ? 1 : 0;    /* UNORDERED */
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to the condition codes we want to cover */
    results[idx++] = __builtin_isunordered(nan, inf);      /* UNORDERED */
    results[idx++] = __builtin_islessgreater(nan, one);    /* LTGT */
    results[idx++] = __builtin_isless(nan, inf);           /* UNORDERED/UNLT */
    results[idx++] = __builtin_isgreater(inf, nan);        /* ORDERED/UNGT */
    results[idx++] = __builtin_islessequal(zero, nan);     /* UNORDERED/UNLE */
    results[idx++] = __builtin_isgreaterequal(inf, nan);   /* ORDERED/UNGE */
    
    /* 3. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile long double ld_nan = __builtin_nanl("");
    results[idx++] = (f_nan < (float)inf) ? 1 : 0;
    results[idx++] = (ld_nan == ld_nan) ? 1 : 0;
    
    /* Arithmetic that produces NaN, then comparison */
    volatile double complex_expr = (inf * zero) + (neg_inf / one);
    results[idx++] = (complex_expr > zero) ? 1 : 0;
    results[idx++] = (complex_expr <= complex_expr) ? 1 : 0;
    
    /* 4. Vector comparisons using GCC extensions */
    v4sf vec_a = {nan, 1.0f, 2.0f, 3.0f};
    v4sf vec_b = {1.0f, nan, 2.0f, 4.0f};
    v4sf vec_cmp = vec_a > vec_b;  /* Should generate multiple condition checks */
    
    /* Extract comparison results to force code generation */
    int mask = __builtin_ia32_movmskps(vec_cmp);
    results[idx++] = mask;
    
    /* Double precision vector */
    v2df vec_da = {nan, 2.0};
    v2df vec_db = {1.0, nan};
    v2df vec_dcmp = vec_da < vec_db;
    
    /* Store to memory and check to prevent elimination */
    double mem_store[2];
    memcpy(mem_store, &vec_dcmp, sizeof(vec_dcmp));
    results[idx++] = (mem_store[0] != 0.0) ? 1 : 0;
    results[idx++] = (mem_store[1] != 0.0) ? 1 : 0;
    
    /* 5. Inline assembly with explicit condition codes */
    double a = nan;
    double b = 1.0;
    int asm_result = 0;
    
    /* Using ucomisd which sets condition codes directly */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%cl\n\t"
        "setb %%dl\n\t"
        "orb %%cl, %%al\n\t"
        "orb %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (asm_result)
        : "x" (a), "x" (b)
        : "al", "cl", "dl", "cc"
    );
    results[idx++] = asm_result;
    
    /* Another asm with different comparison */
    asm volatile (
        "comisd %2, %1\n\t"
        "seta %%al\n\t"
        "setp %%cl\n\t"
        "orl %%ecx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (asm_result)
        : "x" (inf), "x" (nan)
        : "eax", "ecx", "cc"
    );
    results[idx++] = asm_result;
    
    /* 6. Control flow driven by unordered results */
    /* Switch statement based on comparison outcomes */
    int switch_var = 0;
    if (__builtin_isunordered(nan, one)) switch_var |= 1;
    if (__builtin_islessgreater(inf, neg_inf)) switch_var |= 2;
    if (!__builtin_isless(nan, nan)) switch_var |= 4;
    
    switch (switch_var) {
        case 0:
            results[idx++] = 100;
            break;
        case 1:  /* UNORDERED case */
            results[idx++] = 101;
            break;
        case 2:  /* LTGT case */
            results[idx++] = 102;
            break;
        case 3:  /* Combination */
            results[idx++] = 103;
            break;
        case 4:  /* UNORDERED/UNLT false case */
            results[idx++] = 104;
            break;
        case 5:
            results[idx++] = 105;
            break;
        case 6:
            results[idx++] = 106;
            break;
        case 7:
            results[idx++] = 107;
            break;
        default:
            results[idx++] = 999;
    }
    
    /* Loop controlled by comparison results */
    int loop_count = 0;
    volatile double vals[] = {nan, inf, 1.0, 2.0, zero};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (__builtin_isunordered(vals[i], vals[j])) {
                loop_count++;
            }
            if (__builtin_islessgreater(vals[i], vals[j])) {
                loop_count--;
            }
        }
    }
    results[idx++] = loop_count;
    
    /* Ternary operators with unordered comparisons */
    results[idx++] = (__builtin_isunordered(nan, inf)) ? 
                     (__builtin_islessgreater(one, zero) ? 1 : 2) : 
                     (__builtin_isless(nan, zero) ? 3 : 4);
    
    /* Nested conditionals */
    if (nan == nan) {
        if (inf > nan) {
            if (!__builtin_isunordered(zero, one)) {
                results[idx++] = 200;
            }
        }
    }
    
    /* Combine all results into a checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
        checksum += results[i];
    }
    
    global_result = checksum;
}

/* Additional test with FMA operations */
void test_fma_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* FMA operations that might produce NaN */
    double fma_result = __builtin_fma(inf, 0.0, nan);
    double fma_result2 = __builtin_fma(nan, 1.0, inf);
    
    int results[4];
    results[0] = (fma_result == fma_result) ? 1 : 0;  /* UNORDERED/UNEQ */
    results[1] = (fma_result > fma_result2) ? 1 : 0;  /* UNORDERED/UNGT */
    results[2] = (fma_result <= inf) ? 1 : 0;         /* UNORDERED/UNLE */
    results[3] = (fma_result2 >= nan) ? 1 : 0;        /* UNORDERED/UNGE */
    
    /* Use results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i];
    }
    global_result += sum;
}

int main(void) {
    printf("Testing x86 floating-point unordered comparisons...\n");
    
    test_unordered_comparisons();
    test_fma_comparisons();
    
    /* Print something to ensure execution */
    printf("Result checksum: %d\n", global_result);
    
    return 0;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86 architecture only.\n");
    return 0;
}
#endif
