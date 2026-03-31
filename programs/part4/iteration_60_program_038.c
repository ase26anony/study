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

/* Function to perform various unordered comparisons */
static int test_unordered_comparisons(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double neg_one = -1.0;
    
    int results[32];
    int idx = 0;
    
    /* 1. Direct unordered comparisons using operators with NaN operands */
    results[idx++] = (nan < inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan > inf) ? 1 : 0;      /* UNORDERED case */
    results[idx++] = (nan <= inf) ? 1 : 0;     /* UNORDERED case */
    results[idx++] = (nan >= inf) ? 1 : 0;     /* UNORDERED case */
    results[idx++] = (nan == nan) ? 1 : 0;     /* UNORDERED/UNEQ case */
    results[idx++] = (inf != nan) ? 1 : 0;     /* ORDERED/LTGT case */
    
    /* 2. Built-in unordered comparison functions */
    results[idx++] = __builtin_isunordered(nan, inf);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(inf, nan);    /* UNORDERED */
    results[idx++] = __builtin_isunordered(nan, nan);    /* UNORDERED */
    
    results[idx++] = __builtin_islessgreater(nan, inf);  /* LTGT */
    results[idx++] = __builtin_islessgreater(inf, nan);  /* LTGT */
    results[idx++] = __builtin_islessgreater(one, neg_one); /* ORDERED */
    
    results[idx++] = __builtin_isless(nan, inf);         /* UNLT */
    results[idx++] = __builtin_isless(inf, nan);         /* UNORDERED */
    results[idx++] = __builtin_isless(neg_inf, inf);     /* ORDERED */
    
    results[idx++] = __builtin_isgreater(nan, inf);      /* UNGT */
    results[idx++] = __builtin_isgreater(inf, nan);      /* ORDERED */
    results[idx++] = __builtin_isgreater(inf, neg_inf);  /* ORDERED */
    
    results[idx++] = __builtin_islessequal(nan, inf);    /* UNLE */
    results[idx++] = __builtin_islessequal(inf, nan);    /* UNORDERED */
    results[idx++] = __builtin_islessequal(neg_inf, inf);/* ORDERED */
    
    results[idx++] = __builtin_isgreaterequal(nan, inf); /* UNGE */
    results[idx++] = __builtin_isgreaterequal(inf, nan); /* ORDERED */
    results[idx++] = __builtin_isgreaterequal(inf, neg_inf); /* ORDERED */
    
    /* 3. Complex expressions with arithmetic that could produce NaN */
    volatile double nan_prod = zero / zero;           /* Produces NaN */
    volatile double inf_minus_inf = inf - inf;        /* Produces NaN */
    volatile double inf_div_zero = inf / zero;        /* Produces inf */
    
    results[idx++] = (nan_prod == inf_minus_inf) ? 1 : 0;  /* UNORDERED/UNEQ */
    results[idx++] = (nan_prod != inf_div_zero) ? 1 : 0;   /* UNORDERED/LTGT */
    results[idx++] = (inf_div_zero > nan_prod) ? 1 : 0;    /* UNORDERED/UNGT */
    results[idx++] = (inf_minus_inf < inf_div_zero) ? 1 : 0; /* UNORDERED/UNLT */
    
    /* 4. Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    results[idx++] = (f_nan == (float)nan) ? 1 : 0;        /* UNORDERED/UNEQ */
    results[idx++] = (ld_nan != (long double)inf) ? 1 : 0; /* UNORDERED/LTGT */
    results[idx++] = ((double)f_nan > nan) ? 1 : 0;        /* UNORDERED/UNGT */
    results[idx++] = (nan < (double)ld_nan) ? 1 : 0;       /* UNORDERED/UNLT */
    
    /* 5. Ternary operators with unordered comparisons */
    results[idx++] = __builtin_isunordered(nan, inf) ? 
                     __builtin_isless(one, zero) : 
                     __builtin_isgreater(zero, one);      /* Mixes conditions */
    
    results[idx++] = !__builtin_isunordered(inf, nan) ? 
                     __builtin_islessequal(neg_inf, inf) : 
                     __builtin_isgreaterequal(inf, neg_inf); /* ORDERED case */
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i] * (i + 1);
    }
    
    return checksum;
}

/* Function to test vector comparisons */
static int test_vector_comparisons(void) {
    v4sf vec_a = {__builtin_nanf(""), 1.0f, __builtin_inff(), -1.0f};
    v4sf vec_b = {1.0f, __builtin_nanf(""), -1.0f, __builtin_inff()};
    v4sf vec_c = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Vector comparisons that generate condition codes */
    v4sf cmp_result;
    
    /* These should generate various condition code checks */
    cmp_result = vec_a > vec_b;   /* UNORDERED/UNGT cases */
    cmp_result = vec_a < vec_b;   /* UNORDERED/UNLT cases */
    cmp_result = vec_a == vec_b;  /* UNORDERED/UNEQ cases */
    cmp_result = vec_a != vec_b;  /* UNORDERED/LTGT cases */
    
    /* Extract comparison masks */
    int mask1 = __builtin_ia32_movmskps(cmp_result);
    
    /* More vector operations */
    v2df vec_d = {__builtin_nan(""), __builtin_inf()};
    v2df vec_e = {__builtin_inf(), __builtin_nan("")};
    
    v2df cmp_result_d = vec_d > vec_e;
    v2df cmp_result_e = vec_d == vec_e;
    
    /* Store to memory to force evaluation */
    volatile float mem_store[4];
    memcpy((void*)mem_store, (void*)&cmp_result, sizeof(cmp_result));
    
    volatile double mem_store_d[2];
    memcpy((void*)mem_store_d, (void*)&cmp_result_d, sizeof(cmp_result_d));
    
    return mask1 + (int)mem_store[0] + (int)mem_store_d[0];
}

/* Function with inline assembly for explicit condition codes */
static int test_inline_assembly(void) {
    volatile double a = __builtin_nan("");
    volatile double b = __builtin_inf();
    volatile double c = 1.0;
    volatile double d = -1.0;
    
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    /* Inline assembly with ucomisd and condition codes */
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
        : "x"(b), "x"(a)
        : "al", "cc"
    );
    
    /* Test with fucomi instruction */
    asm volatile (
        "fucomi %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result3)
        : "t"(c), "u"(d)
        : "al", "cc"
    );
    
    asm volatile (
        "fucomi %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result4)
        : "t"(d), "u"(c)
        : "al", "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Control flow based on unordered comparison results */
static int test_control_flow(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double values[] = {nan, inf, -inf, 0.0, 1.0, -1.0};
    int result = 0;
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            /* Switch on combination of comparison results */
            int cmp_result = 0;
            if (__builtin_isunordered(values[i], values[j])) {
                cmp_result |= 1;  /* UNORDERED */
            }
            if (__builtin_isless(values[i], values[j])) {
                cmp_result |= 2;  /* UNLT or ORDERED */
            }
            if (__builtin_isgreater(values[i], values[j])) {
                cmp_result |= 4;  /* UNGT or ORDERED */
            }
            if (__builtin_islessequal(values[i], values[j])) {
                cmp_result |= 8;  /* UNLE or ORDERED */
            }
            if (__builtin_isgreaterequal(values[i], values[j])) {
                cmp_result |= 16; /* UNGE or ORDERED */
            }
            
            /* Use switch to force different code paths */
            switch (cmp_result & 0x1F) {
                case 0:  /* All false - UNEQ? */
                    result += 1;
                    break;
                case 1:  /* Only UNORDERED */
                    result += 2;
                    break;
                case 2:  /* Only less (ORDERED) */
                    result += 3;
                    break;
                case 4:  /* Only greater (ORDERED) */
                    result += 4;
                    break;
                case 6:  /* less and greater (LTGT) */
                    result += 5;
                    break;
                case 8:  /* Only lessequal (ORDERED) */
                    result += 6;
                    break;
                case 16: /* Only greaterequal (ORDERED) */
                    result += 7;
                    break;
                default: /* Mixed conditions */
                    result += 8;
                    break;
            }
        }
    }
    
    return result;
}

int main(void) {
    int total_result = 0;
    
    /* Run all tests */
    total_result ^= test_unordered_comparisons();
    total_result ^= test_vector_comparisons();
    total_result ^= test_inline_assembly();
    total_result ^= test_control_flow();
    
    /* Use results to prevent dead code elimination */
    use_result(total_result);
    
    printf("Test completed with result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
}
#endif
