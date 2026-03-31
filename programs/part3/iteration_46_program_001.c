/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static volatile double values[] = {1.0, 2.0, 0.0, -1.0, 3.14};
    return values[idx % 5];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static volatile float values[] = {1.0f, 0.0f, -1.0f, 2.5f, 3.0f};
    return values[idx % 5];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile static int sink;
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 0.0/0.0;  /* Potential NaN with -ffast-math assumptions */
    double d1 = get_double_input(0);
    double d2 = get_double_input(1);
    double d3 = get_double_input(2);
    
    volatile float vf1 = 2.0f;
    volatile float vf2 = -1.0f;
    float f1 = get_float_input(0);
    float f2 = get_float_input(1);
    
    /* 2. Perform relational operator comparisons with -ffast-math */
    /* These should generate various condition codes */
    
    /* UNORDERED / UNEQ patterns */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (!(vd1 == vd2)) {  /* Complex pattern for unordered */
        checksum += 2;
    }
    
    /* UNLE / UNLT patterns */
    if (f1 <= f2) {  /* May generate UNLE */
        checksum += 4;
    }
    
    if (vf1 < vf2) {  /* May generate UNLT */
        checksum += 8;
    }
    
    /* UNGE / UNGT patterns */
    if (d2 >= d3) {  /* May generate UNGE */
        checksum += 16;
    }
    
    if (d1 > d3) {  /* May generate UNGT */
        checksum += 32;
    }
    
    /* ORDERED check */
    if (f1 == f1 && f2 == f2) {  /* Ordered comparison */
        checksum += 64;
    }
    
    /* 3. Explicit built-in function calls */
    /* Directly map to specific condition codes */
    
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 128;
    }
    
    /* LTGT condition */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 256;
    }
    
    /* UNEQ condition */
    if (!__builtin_islessgreater(d1, d2) && !__builtin_isunordered(d1, d2)) {
        checksum += 512;
    }
    
    /* ORDERED condition */
    if (__builtin_isordered(vf1, vf2)) {
        checksum += 1024;
    }
    
    /* 4. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_eq = (vec_a == vec_c);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* cmp_results = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (cmp_results[i] != 0.0f) checksum += 2048;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* Check unordered mask */
    int mask = _mm_movemask_ps(sse_cmp);
    if (mask != 0) {
        checksum += 4096;
    }
    
    /* 5. Conditional moves based on FP comparisons */
    /* Force materialization of condition codes */
    double cond_mov_result;
    
    /* UNGE pattern */
    cond_mov_result = (d1 >= d2) ? d1 : d2;
    checksum += (int)cond_mov_result;
    
    /* UNLE pattern */
    float f_cond = (f1 <= f2) ? f1 : f2;
    checksum += (int)f_cond;
    
    /* LTGT pattern with conditional */
    double d_cond = (d1 != d3) ? d1 * 2.0 : d3 / 2.0;
    checksum += (int)d_cond;
    
    /* 6. Loop with FP condition to generate multiple instances */
    for (int i = 0; i < 10; i++) {
        double loop_val = get_double_input(i);
        /* Mix different comparisons in loop */
        if (loop_val != 0.0) {           /* UNEQ/LTGT */
            checksum += i * 2;
        }
        if (loop_val <= 2.0) {           /* UNLE */
            checksum += i * 3;
        }
        if (__builtin_isunordered(loop_val, 1.0)) {  /* UNORDERED */
            checksum += i * 5;
        }
    }
    
    /* 7. Switch based on FP comparison results */
    /* Create multiple basic blocks with different condition codes */
    int fp_switch = 0;
    
    if (d1 < d2) fp_switch = 1;      /* UNLT */
    else if (d1 > d2) fp_switch = 2; /* UNGT */
    else if (d1 == d2) fp_switch = 3; /* UNEQ */
    else fp_switch = 4;               /* UNORDERED */
    
    switch (fp_switch) {
        case 1: checksum += 8192; break;
        case 2: checksum += 16384; break;
        case 3: checksum += 32768; break;
        case 4: checksum += 65536; break;
    }
    
    /* 8. Mixed integer/float comparisons */
    int int_val = 5;
    if (d1 < int_val) {           /* Mixed mode comparison */
        checksum += 131072;
    }
    
    if (f1 >= int_val) {          /* Another mixed mode */
        checksum += 262144;
    }
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
