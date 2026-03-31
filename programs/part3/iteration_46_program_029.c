/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double get_input_double(void) __attribute__((noinline));
extern volatile float get_input_float(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Opaque function to prevent optimization */
void use_result(int val) {
    /* Create side effect */
    static volatile int sink;
    sink = val;
}

/* Functions to get dynamic inputs */
volatile double get_input_double(void) {
    static volatile double counter = 0.0;
    return counter += 1.234567;
}

volatile float get_input_float(void) {
    static volatile float counter = 0.0f;
    return counter += 0.987654f;
}

int main(void) {
    int checksum = 0;
    
    /* 1. SCALAR FLOATING-POINT COMPARISONS WITH -ffast-math */
    
    /* Use volatile to prevent constant folding */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    /* Get dynamic values */
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* Mix with integer operands */
    int int_val = 42;
    
    /* Series of comparisons to generate different condition codes */
    
    /* UNORDERED: x != x or y != y (NaNs) - with fast-math may still generate */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;  /* Potential UNORDERED */
    }
    
    /* ORDERED: !(x != x || y != y) */
    if (!__builtin_isunordered(f1, f2)) {
        checksum += 2;  /* Potential ORDERED */
    }
    
    /* UNEQ: unordered or equal */
    if (d1 == d2) {  /* With -ffast-math, may use UNEQ */
        checksum += 4;
    }
    
    /* UNGE: unordered or greater-or-equal */
    if (vd1 >= vd2) {  /* May generate UNGE (nlt) */
        checksum += 8;
    }
    
    /* UNGT: unordered or greater */
    if (f1 > f2) {  /* May generate UNGT (nle) */
        checksum += 16;
    }
    
    /* UNLE: unordered or less-or-equal */
    if (d1 <= (double)int_val) {  /* Mixed type, may generate UNLE */
        checksum += 32;
    }
    
    /* UNLT: unordered or less */
    if (vf1 < vf2) {  /* May generate UNLT (ult) */
        checksum += 64;
    }
    
    /* LTGT: less or greater (not equal, not unordered) */
    if (__builtin_islessgreater(d1, d2)) {  /* Direct builtin for LTGT */
        checksum += 128;
    }
    
    /* != operator may generate LTGT (une) with fast-math */
    if (f1 != f2) {
        checksum += 256;
    }
    
    /* 2. CONDITIONAL MOVES BASED ON FP COMPARISONS */
    
    /* Conditional expression with FP result */
    double cond_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)(cond_result * 10) % 100;
    
    float cond_result_f = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)(cond_result_f * 10) % 100;
    
    /* 3. VECTOR (SIMD) COMPARISONS */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    
    /* Vector comparisons - these often generate condition codes */
    v4sf vec_cmp_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf vec_cmp_gt = (vec_a > vec_b);   /* May use UNGT */
    v4sf vec_cmp_le = (vec_a <= vec_b);  /* May use UNLE */
    
    /* Extract results to scalar checksum */
    float* eq_results = (float*)&vec_cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_results[i] != 0.0f) checksum += 512;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(sse_cmp);
    checksum += mask;
    
    /* 4. LOOP WITH FP CONDITION */
    
    /* Array with potential zeros */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_input_double() - 1.234567 * i;
    }
    
    /* Loop condition with FP comparison */
    int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {  /* != may generate LTGT */
        count++;
    }
    checksum += count;
    
    /* 5. SWITCH BASED ON FP COMPARISON RESULTS */
    
    /* Create a value based on multiple comparisons */
    int fp_case = 0;
    fp_case |= (d1 < d2) ? 1 : 0;   /* May use UNLT */
    fp_case |= (f1 > f2) ? 2 : 0;   /* May use UNGT */
    fp_case |= (d1 == d2) ? 4 : 0;  /* May use UNEQ */
    
    switch (fp_case) {
        case 0:
            checksum += 1024;
            break;
        case 1:
            checksum += 2048;  /* UNLT path */
            break;
        case 2:
            checksum += 4096;  /* UNGT path */
            break;
        case 4:
            checksum += 8192;  /* UNEQ path */
            break;
        default:
            checksum += 16384;
            break;
    }
    
    /* 6. MORE BUILTIN USAGE */
    
    /* Test all relevant builtins */
    checksum += __builtin_isunordered(d1, d2) ? 32768 : 0;
    checksum += __builtin_islessgreater(f1, f2) ? 65536 : 0;
    
    /* Ordered comparisons that may generate inverted condition codes */
    checksum += (d1 < d2 && !__builtin_isunordered(d1, d2)) ? 131072 : 0;
    
    /* 7. FINAL OUTPUT TO PREVENT DEAD CODE ELIMINATION */
    
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-constant result */
}
