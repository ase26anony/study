/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding with opaque functions */
extern double get_input_double(void) __attribute__((noinline));
extern float get_input_float(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile double volatile_d = 1.5;
volatile float volatile_f = 2.5f;
int global_checksum = 0;

/* Function to accumulate results with side effects */
void accumulate(int cond) {
    global_checksum = (global_checksum * 31 + cond) & 0xFFFF;
}

int main(void) {
    /* Initialize floating-point variables from mixed sources */
    double d1 = get_input_double();
    double d2 = 3.14159;
    double d3 = volatile_d;
    
    float f1 = get_input_float();
    float f2 = 2.71828f;
    float f3 = volatile_f;
    
    int checksum = 0;
    
    /* ============================================
       SCALAR FLOATING-POINT COMPARISONS WITH -ffast-math
       ============================================ */
    
    /* 1. UNORDERED (unord) - Test for NaN conditions */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 1;
    }
    
    /* 2. ORDERED (ord) - Test for ordered (non-NaN) */
    if (__builtin_isordered(f1, f2)) {
        checksum += 2;
    }
    
    /* 3. UNEQ (ueq) - Unordered or equal */
    if (!(d1 == d3)) {  /* With -ffast-math, != may generate UNEQ */
        checksum += 4;
    }
    
    /* 4. UNGE (nlt) - Unordered or greater than or equal */
    if (d2 >= d1) {  /* May generate UNGE with -ffast-math */
        checksum += 8;
    }
    
    /* 5. UNGT (nle) - Unordered or greater than */
    if (d1 > d3) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* 6. UNLE (ule) - Unordered or less than or equal */
    if (f1 <= f2) {  /* May generate UNLE */
        checksum += 32;
    }
    
    /* 7. UNLT (ult) - Unordered or less than */
    if (f3 < f1) {  /* May generate UNLT */
        checksum += 64;
    }
    
    /* 8. LTGT (une) - Less than or greater than (ordered, not equal) */
    if (__builtin_islessgreater(d1, d2)) {
        checksum += 128;
    }
    
    /* Mixed integer/float comparisons */
    int i = 42;
    if (d1 != (double)i) {  /* May generate UNEQ */
        checksum += 256;
    }
    
    /* Conditional move based on FP comparison */
    double result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)(result * 10) % 256;
    
    /* ============================================
       VECTOR (SIMD) COMPARISONS
       ============================================ */
    
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector comparisons using GCC extensions */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_c);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_c);    /* May use UNGE */
    
    /* Extract results from vector comparisons */
    float* eq_results = (float*)&cmp_eq;
    for (int j = 0; j < 4; j++) {
        if (eq_results[j] != 0.0f) checksum += 512;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* _CMP_EQ_UQ generates UNEQ condition */
    __m128 cmp_eq_uq = _mm_cmp_ps(sse_a, sse_b, _CMP_EQ_UQ);
    
    /* _CMP_NLT_UQ generates UNGE condition */
    __m128 cmp_nlt_uq = _mm_cmp_ps(sse_a, sse_b, _CMP_NLT_UQ);
    
    /* Extract mask from comparison results */
    int mask = _mm_movemask_ps(cmp_unord);
    checksum += mask * 1024;
    
    /* Double precision vector comparisons */
    v2df vec_d1 = {d1, d2};
    v2df vec_d2 = {d2, d1};
    v2df cmp_double = (vec_d1 > vec_d2);  /* May generate UNGT */
    
    /* ============================================
       LOOP WITH FP CONDITION TO PREVENT OPTIMIZATION
       ============================================ */
    
    float array[10];
    for (int k = 0; k < 10; k++) {
        array[k] = (float)k * 0.1f;
    }
    
    /* Loop condition with FP comparison */
    float sum = 0.0f;
    for (int k = 0; k < 10 && (array[k] != 0.0f); k++) {  /* May use UNEQ */
        sum += array[k];
        if (array[k] <= 1.0f) {  /* May use UNLE */
            checksum += k;
        }
    }
    
    /* Switch statement based on FP comparison results */
    int switch_val = 0;
    if (d1 < d2) switch_val = 1;      /* May use UNLT */
    else if (d1 == d2) switch_val = 2; /* May use UNEQ */
    else if (d1 > d2) switch_val = 3;  /* May use UNGT */
    
    switch (switch_val) {
        case 1: checksum += 2048; break;
        case 2: checksum += 4096; break;
        case 3: checksum += 8192; break;
    }
    
    /* ============================================
       FINAL OUTPUT TO PREVENT DEAD CODE ELIMINATION
       ============================================ */
    
    /* Use all results to create observable output */
    checksum += (int)(sum * 100);
    checksum += (int)(result * 1000) % 65536;
    
    /* Store to global with side effect */
    global_checksum = checksum;
    
    /* Call external function to prevent optimization */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 255;  /* Return non-constant value */
}

/* Dummy implementations to satisfy linker */
double get_input_double(void) { 
    static double counter = 0.0;
    return counter++ * 0.5; 
}

float get_input_float(void) { 
    static float counter = 0.0f;
    return counter++ * 0.3f; 
}

void use_result(int val) {
    /* Empty but prevents optimization */
    volatile int dummy = val;
    (void)dummy;
}
