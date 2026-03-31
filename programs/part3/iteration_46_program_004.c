/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static volatile double values[] = {1.0, 2.0, 0.0, -1.0, 3.14, -2.71};
    return values[idx % 6];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static volatile float values[] = {1.0f, 0.0f, -1.0f, 2.5f, -3.14f, 0.001f};
    return values[idx % 6];
}

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize FP variables from mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = -0.0;
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    double d3 = get_double_input(2);
    
    volatile float vf1 = 1.0f;
    volatile float vf2 = 0.0f;
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    float f3 = get_float_input(2);
    
    /* 2. Perform relational comparisons with -ffast-math assumptions */
    /* These should generate various condition codes */
    
    /* UNORDERED/ORDERED cases */
    if (vd1 != vd2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (d1 < d2) {     /* May generate UNLT or LT */
        checksum += 2;
    }
    
    if (f1 >= f2) {    /* May generate UNGE or GE */
        checksum += 4;
    }
    
    if (vf1 <= vf2) {  /* May generate UNLE or LE */
        checksum += 8;
    }
    
    /* Mixed type comparisons */
    if ((double)f1 > d1) {
        checksum += 16;
    }
    
    /* 3. Explicit built-in unordered checks */
    /* Directly map to specific condition codes */
    
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 32;
    }
    
    /* LTGT condition */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 64;
    }
    
    /* UNEQ condition */
    if (!__builtin_islessgreater(vd1, vd2) && 
        !__builtin_isunordered(vd1, vd2)) {
        checksum += 128;
    }
    
    /* UNGE condition via builtins */
    if (!__builtin_isless(d1, d2)) {
        checksum += 256;
    }
    
    /* UNLE condition */
    if (!__builtin_isgreater(f1, f2)) {
        checksum += 512;
    }
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result = (d1 == d3) ? d1 : d2;  /* May use UNEQ */
    checksum += (int)cmov_result;
    
    float f_cmov = (vf1 > vf2) ? vf1 : vf2;     /* May use UNGT */
    checksum += (int)f_cmov;
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (get_float_input(i) != 0.0f); ++i) {
        checksum += i;  /* May use UNEQ in loop condition */
    }
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    
    /* Extract results to scalar checksum */
    float* eq_ptr = (float*)&cmp_eq;
    float* lt_ptr = (float*)&cmp_lt;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_ptr[i] != 0.0f) ? 1 : 0;
        checksum += (lt_ptr[i] != 0.0f) ? 2 : 0;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(unord_cmp);
    checksum += mask;
    
    /* Double precision vector */
    v2df vec_da = {1.0, 2.0};
    v2df vec_db = {2.0, 1.0};
    v2df cmp_dle = (vec_da <= vec_db);  /* May use UNLE */
    
    double* dle_ptr = (double*)&cmp_dle;
    checksum += (dle_ptr[0] != 0.0) ? 1024 : 0;
    checksum += (dle_ptr[1] != 0.0) ? 2048 : 0;
    
    /* 6. Switch statement with FP comparisons */
    int switch_val = 0;
    for (int i = 0; i < 5; i++) {
        double a = get_double_input(i);
        double b = get_double_input(i + 1);
        
        if (__builtin_isunordered(a, b)) {
            switch_val = 1;  /* UNORDERED */
        } else if (a == b) {
            switch_val = 2;  /* UNEQ */
        } else if (a < b) {
            switch_val = 3;  /* UNLT */
        } else if (a > b) {
            switch_val = 4;  /* UNGT */
        }
        
        switch (switch_val) {
            case 1: checksum += 4096; break;
            case 2: checksum += 8192; break;
            case 3: checksum += 16384; break;
            case 4: checksum += 32768; break;
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum != 0 ? 0 : 1;
}
