/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

float __attribute__((noinline)) get_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    volatile double vx = 1.0;
    volatile double vy = 2.0;
    volatile float vf1 = 3.0f;
    volatile float vf2 = 4.0f;
    
    double d1 = get_double();
    double d2 = get_double();
    float f1 = get_float();
    float f2 = get_float();
    
    /* Mix comparisons with volatile and dynamic values */
    if (vx != vy) checksum += 1;           /* May generate UNEQ or LTGT */
    if (d1 <= d2) checksum += 2;           /* May generate UNLE */
    if (f1 >= f2) checksum += 4;           /* May generate UNGE */
    if (vx < vy) checksum += 8;            /* May generate UNLT */
    if (d1 > d2) checksum += 16;           /* May generate UNGT */
    if (f1 == f2) checksum += 32;          /* May generate UNEQ */
    
    /* 2. Explicit built-in unordered checks */
    checksum += __builtin_isunordered(d1, d2) ? 64 : 0;    /* UNORDERED */
    checksum += __builtin_islessgreater(f1, f2) ? 128 : 0; /* LTGT */
    
    /* Mixed integer/floating comparisons */
    int i = 5;
    if (d1 != i) checksum += 256;          /* Mixed type comparison */
    if (f1 <= i) checksum += 512;
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons that may generate condition codes */
    v4sf cmp_eq = (vec_a == vec_b);        /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);         /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);         /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);        /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);        /* May use UNGE */
    v4sf cmp_ne = (vec_a != vec_b);        /* May use LTGT */
    
    /* Extract results to prevent optimization */
    float results[6];
    results[0] = cmp_eq[0] + cmp_eq[1];
    results[1] = cmp_lt[0] + cmp_lt[1];
    results[2] = cmp_gt[0] + cmp_gt[1];
    results[3] = cmp_le[0] + cmp_le[1];
    results[4] = cmp_ge[0] + cmp_ge[1];
    results[5] = cmp_ne[0] + cmp_ne[1];
    
    for (int j = 0; j < 6; j++) {
        checksum += (results[j] > 0) ? (1 << j) : 0;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 sse_cmp_unord = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* 4. Conditional moves based on FP results */
    double cmov_result1 = (d1 >= d2) ? d1 : d2;    /* May use UNGE */
    float cmov_result2 = (f1 != f2) ? f1 : f2;     /* May use UNEQ or LTGT */
    checksum += (int)cmov_result1;
    checksum += (int)cmov_result2;
    
    /* Loop with FP condition */
    double arr[10];
    for (int k = 0; k < 10; k++) {
        arr[k] = get_double();
    }
    
    int count = 0;
    for (int k = 0; k < 10 && (arr[k] != 0.0); k++) {
        count++;  /* May generate UNEQ in loop condition */
    }
    checksum += count;
    
    /* Switch based on comparison results */
    int cmp_val = 0;
    if (__builtin_isunordered(d1, d2)) cmp_val = 1;
    else if (d1 == d2) cmp_val = 2;
    else if (d1 < d2) cmp_val = 3;
    else if (d1 > d2) cmp_val = 4;
    
    switch (cmp_val) {
        case 1: checksum += 1000; break;  /* UNORDERED */
        case 2: checksum += 2000; break;  /* UNEQ */
        case 3: checksum += 3000; break;  /* UNLT */
        case 4: checksum += 4000; break;  /* UNGT */
    }
    
    /* 5. More builtins for complete coverage */
    checksum += __builtin_isless(f1, f2) ? 5000 : 0;
    checksum += __builtin_islessequal(f1, f2) ? 6000 : 0;
    checksum += __builtin_isgreater(f1, f2) ? 7000 : 0;
    checksum += __builtin_isgreaterequal(f1, f2) ? 8000 : 0;
    
    /* Double vector comparisons */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    v2df dvec_cmp = (dvec_a <= dvec_b);  /* May use UNLE */
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
