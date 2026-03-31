/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink;
    sink = val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_input_double();
    double d2 = get_input_double();
    double d3 = 3.14159;
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    float f1 = get_input_float();
    float f2 = get_input_float();
    float f3 = 2.71828f;
    
    /* 2. Perform relational operator comparisons with -ffast-math */
    
    /* UNORDERED/ORDERED patterns */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (vd1 <= vd2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (f1 >= f2) {  /* May generate UNGE */
        checksum += 4;
    }
    
    if (vf1 < vf2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    if (d3 > d1) {  /* May generate UNGT */
        checksum += 16;
    }
    
    /* Mixed float/double comparisons */
    if ((double)f1 != d2) {
        checksum += 32;
    }
    
    /* 3. Explicit built-in unordered checks */
    
    /* Direct UNORDERED test */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 64;
    }
    
    /* LTGT condition */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 128;
    }
    
    /* UNEQ via builtins */
    if (!__builtin_islessgreater(f1, f2) && !__builtin_isunordered(f1, f2)) {
        checksum += 256;
    }
    
    /* 4. Conditional moves based on FP results */
    double cmov_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)(cmov_result * 10) % 31;
    
    float f_cmov = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)(f_cmov * 10) % 31;
    
    /* 5. Loop with FP condition */
    for (int i = 0; i < 10 && (d1 != 0.0); ++i) {
        checksum += i;
        d1 = get_input_double();  /* Change value */
    }
    
    /* 6. Vector (SIMD) comparisons */
    
    /* Initialize vector floats */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_c);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_c);    /* May use UNGE */
    
    /* Extract results from vectors */
    int vcmp_result = 0;
    float* feq = (float*)&cmp_eq;
    float* fneq = (float*)&cmp_neq;
    
    for (int i = 0; i < 4; ++i) {
        vcmp_result += (feq[i] != 0.0f) ? (1 << i) : 0;
        vcmp_result += (fneq[i] != 0.0f) ? (1 << (i + 4)) : 0;
    }
    checksum += vcmp_result;
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Check if any element is unordered */
    int unord_mask = _mm_movemask_ps(cmp_unord);
    checksum += unord_mask * 3;
    
    /* 7. Switch statement with FP comparisons */
    int switch_val = checksum % 5;
    double switch_result = 0.0;
    
    switch (switch_val) {
        case 0:
            switch_result = (d2 < d3) ? d2 : d3;  /* UNLT */
            break;
        case 1:
            switch_result = (d2 > d3) ? d2 : d3;  /* UNGT */
            break;
        case 2:
            switch_result = (d2 == d3) ? d2 : d3; /* UNEQ */
            break;
        case 3:
            switch_result = (d2 != d3) ? d2 : d3; /* LTGT */
            break;
        default:
            switch_result = (d2 <= d3) ? d2 : d3; /* UNLE */
            break;
    }
    checksum += (int)switch_result;
    
    /* 8. Prevent dead code elimination */
    use_result(checksum);
    
    /* Final output to prevent optimization */
    printf("Condition code checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
