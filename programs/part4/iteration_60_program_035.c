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
    volatile static int sink = 0;
    sink += result;
}

/* Main test function */
int main(void) {
    int checksum = 0;
    
    /* Volatile to prevent constant folding */
    volatile double nan_d = __builtin_nan("");
    volatile double inf_d = __builtin_inf();
    volatile double neg_inf_d = -__builtin_inf();
    volatile double zero_d = 0.0;
    volatile double one_d = 1.0;
    
    volatile float nan_f = __builtin_nanf("");
    volatile float inf_f = __builtin_inff();
    volatile float neg_inf_f = -__builtin_inff();
    volatile float zero_f = 0.0f;
    volatile float one_f = 1.0f;
    
    volatile long double nan_ld = __builtin_nanl("");
    volatile long double inf_ld = __builtin_infl();
    volatile long double zero_ld = 0.0L;
    volatile long double one_ld = 1.0L;
    
    /* Test 1: Direct unordered comparisons with operators */
    /* These should generate UNORDERED/ORDERED condition codes */
    if (nan_d < inf_d) { /* UNORDERED case - always false with NaN */
        checksum += 1;
    }
    
    if (!(nan_d < inf_d)) { /* ORDERED case - complement of above */
        checksum += 2;
    }
    
    if (nan_d == nan_d) { /* UNEQ case - NaN != NaN, but unordered equal */
        checksum += 4;
    }
    
    if (nan_d != nan_d) { /* LTGT case - not equal and ordered */
        checksum += 8;
    }
    
    /* Test 2: Built-in unordered comparison functions */
    /* These map directly to the condition codes */
    if (__builtin_isunordered(nan_d, inf_d)) { /* UNORDERED */
        checksum += 16;
    }
    
    if (__builtin_islessgreater(nan_d, inf_d)) { /* LTGT */
        checksum += 32;
    }
    
    /* Complex expression to trigger multiple condition codes */
    int result1 = __builtin_isless(nan_f, inf_f) ? 64 : 128; /* UNLT/UNGE */
    int result2 = __builtin_isgreater(nan_f, neg_inf_f) ? 256 : 512; /* UNGT/UNLE */
    int result3 = __builtin_islessequal(inf_f, nan_f) ? 1024 : 2048; /* UNLE/UNGT */
    int result4 = __builtin_isgreaterequal(neg_inf_f, nan_f) ? 4096 : 8192; /* UNGE/UNLT */
    
    checksum += result1 + result2 + result3 + result4;
    
    /* Test 3: Mixed-type comparisons and arithmetic */
    /* Create NaN through arithmetic */
    volatile double nan_arith = inf_d / zero_d;  /* Should produce inf, but with fast-math might be NaN */
    volatile double inf_minus_inf = inf_d - inf_d; /* NaN */
    
    /* Comparisons with arithmetic-generated NaN */
    if (nan_arith > one_d) { /* UNORDERED/UNGT */
        checksum += 16384;
    }
    
    if (inf_minus_inf <= zero_d) { /* UNORDERED/UNLE */
        checksum += 32768;
    }
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan_d, one_d, inf_d);
    if (fma_result == fma_result) { /* UNEQ - NaN comparison */
        checksum += 65536;
    }
    
    /* Test 4: Vector comparisons using GCC extensions */
    /* These may generate multiple condition code checks */
    v4sf vec_a = {nan_f, inf_f, neg_inf_f, zero_f};
    v4sf vec_b = {inf_f, nan_f, zero_f, neg_inf_f};
    v4sf vec_c = {one_f, one_f, one_f, one_f};
    
    /* Vector comparisons - each element comparison needs condition code */
    v4sf cmp_result = vec_a > vec_b;  /* Should generate multiple UNGT/UNLE checks */
    v4sf cmp_result2 = vec_a == vec_b; /* UNEQ checks */
    v4sf cmp_result3 = vec_a < vec_c;  /* UNLT/UNGE checks */
    
    /* Extract comparison masks to force code generation */
    int mask1, mask2, mask3;
    
    /* Use x86-specific intrinsic if available */
    #ifdef __SSE__
    mask1 = __builtin_ia32_movmskps((__v4sf)cmp_result);
    mask2 = __builtin_ia32_movmskps((__v4sf)cmp_result2);
    mask3 = __builtin_ia32_movmskps((__v4sf)cmp_result3);
    #else
    /* Fallback: store to memory and check */
    float store[4];
    memcpy(store, &cmp_result, sizeof(cmp_result));
    mask1 = (store[0] != 0.0f) | ((store[1] != 0.0f) << 1) | 
            ((store[2] != 0.0f) << 2) | ((store[3] != 0.0f) << 3);
    #endif
    
    checksum += mask1 + mask2 + mask3;
    
    /* Test 5: Double vector comparisons */
    v2df vec_d1 = {nan_d, inf_d};
    v2df vec_d2 = {inf_d, nan_d};
    v2df vec_cmp_d = vec_d1 > vec_d2;
    
    double d_store[2];
    memcpy(d_store, &vec_cmp_d, sizeof(vec_cmp_d));
    checksum += (d_store[0] != 0.0) + (d_store[1] != 0.0) * 2;
    
    /* Test 6: Inline assembly with explicit condition codes */
    /* Force compiler to handle these condition codes in its internal representation */
    double a = nan_d;
    double b = inf_d;
    int asm_result = 0;
    
    /* ucomisd sets flags for unordered comparisons */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %%al\n\t"          /* UNORDERED: parity flag set if unordered */
        "sete %%bl\n\t"          /* UNEQ: equal flag */
        "seta %%cl\n\t"          /* UNGT: above flag (CF=0 and ZF=0) */
        "setb %%dl\n\t"          /* UNLT: below flag (CF=1) */
        "movzbl %%al, %[res]\n\t"
        : [res] "=r" (asm_result)
        : [a] "x" (a), [b] "x" (b)
        : "al", "bl", "cl", "dl", "cc"
    );
    
    checksum += asm_result * 1048576;
    
    /* Test 7: Control flow driven by unordered results */
    /* Switch statement where cases depend on comparison results */
    int switch_var = 0;
    
    /* Build a value based on multiple comparisons */
    if (__builtin_isunordered(nan_d, inf_d)) switch_var |= 1;
    if (__builtin_islessgreater(nan_d, one_d)) switch_var |= 2;
    if (!__builtin_isless(nan_f, zero_f)) switch_var |= 4;
    if (__builtin_isgreaterequal(inf_d, nan_d)) switch_var |= 8;
    
    switch (switch_var & 0xF) {
        case 0: checksum += 1; break;  /* ORDERED, !LTGT, UNGE, UNLT */
        case 1: checksum += 2; break;  /* UNORDERED */
        case 2: checksum += 4; break;  /* LTGT */
        case 3: checksum += 8; break;  /* UNORDERED | LTGT */
        case 4: checksum += 16; break; /* UNGE */
        case 5: checksum += 32; break; /* UNORDERED | UNGE */
        case 6: checksum += 64; break; /* LTGT | UNGE */
        case 7: checksum += 128; break;/* UNORDERED | LTGT | UNGE */
        case 8: checksum += 256; break;/* UNLT */
        default: checksum += 512; break;
    }
    
    /* Test 8: Long double comparisons (x87 FPU) */
    /* These use different instructions but same condition codes */
    if (nan_ld < inf_ld) { /* UNORDERED */
        checksum += 1024;
    }
    
    if (inf_ld > nan_ld) { /* UNORDERED */
        checksum += 2048;
    }
    
    /* Test 9: Complex nested conditionals */
    /* Force compiler to generate complex conditional code */
    volatile double x = nan_d;
    volatile double y = inf_d;
    volatile double z = zero_d;
    
    for (int i = 0; i < 3; i++) {
        if (x < y) {
            if (y > z) {
                if (z == x) {
                    checksum += 4096 * i;
                }
            } else if (x != y) {
                checksum += 8192 * i;
            }
        } else if (x > z) {
            checksum += 16384 * i;
        }
        
        /* Rotate values */
        double temp = x;
        x = y;
        y = z;
        z = temp;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;

#else
    /* Non-x86 fallback */
    int main(void) {
        printf("This test is for x86 architecture only.\n");
        return 0;
    }
#endif
}
