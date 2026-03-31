/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_seed;
extern float get_float(void) __attribute__((noinline));
extern double get_double(void) __attribute__((noinline));

/* Opaque function to consume results */
extern void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with fast-math assumptions */
    float f1 = get_float();
    float f2 = get_float();
    double d1 = get_double();
    double d2 = get_double();
    
    /* Mix with volatile to prevent constant folding */
    float f3 = vf1;
    float f4 = vf2;
    double d3 = vd1;
    double d4 = vd2;
    
    /* Generate various condition codes through comparisons */
    
    /* UNORDERED - may be generated with fast-math for != */
    if (f1 != f2) {  /* Could generate UNEQ or LTGT */
        checksum += 1;
    }
    
    /* ORDERED - implicit in most comparisons */
    if (d1 < d2) {   /* Could generate UNLT with fast-math */
        checksum += 2;
    }
    
    /* UNEQ - unordered or equal */
    if (f3 == f4) {  /* With fast-math, could use UNEQ */
        checksum += 4;
    }
    
    /* UNGE - unordered or greater than or equal */
    if (d3 >= d4) {  /* Could generate UNGE (nlt) */
        checksum += 8;
    }
    
    /* UNGT - unordered or greater than */
    if (f1 > f2) {   /* Could generate UNGT (nle) */
        checksum += 16;
    }
    
    /* UNLE - unordered or less than or equal */
    if (d1 <= d2) {  /* Could generate UNLE */
        checksum += 32;
    }
    
    /* UNLT - unordered or less than */
    if (f3 < f4) {   /* Could generate UNLT (ult) */
        checksum += 64;
    }
    
    /* LTGT - less than or greater than (unordered) */
    if (d3 != d4) {  /* Could generate LTGT (une) */
        checksum += 128;
    }
    
    /* 2. Explicit built-in functions for unordered checks */
    checksum += __builtin_isunordered(f1, f2) ? 256 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(d1, d2) ? 512 : 0;    /* LTGT */
    checksum += __builtin_islessequal(f3, f4) ? 1024 : 0;     /* UNLE */
    checksum += __builtin_isgreaterequal(d3, d4) ? 2048 : 0;  /* UNGE */
    
    /* 3. Conditional moves based on FP comparisons */
    double cmov_result = (f1 >= f2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)cmov_result;
    
    float cmov_result2 = (d1 <= d2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)cmov_result2;
    
    /* 4. Vector (SIMD) comparisons */
    v4sf vec_a = {f1, f2, f3, f4};
    v4sf vec_b = {f2, f1, f4, f3};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results from vectors */
    float* eq_results = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        checksum += (eq_results[i] != 0.0f) ? (1 << i) : 0;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(f1, f2, f3, f4);
    __m128 sse_b = _mm_set_ps(f2, f1, f4, f3);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* Check unordered results */
    int unord_mask = _mm_movemask_ps(unord_cmp);
    checksum += unord_mask;
    
    /* Double precision vector */
    v2df dvec_a = {d1, d2};
    v2df dvec_b = {d2, d1};
    v2df dvec_cmp = (dvec_a > dvec_b);  /* May use UNGT */
    
    double* dresults = (double*)&dvec_cmp;
    checksum += (dresults[0] != 0.0) ? 4096 : 0;
    checksum += (dresults[1] != 0.0) ? 8192 : 0;
    
    /* 5. Loop with FP condition to generate branch with condition code */
    float arr[4] = {f1, f2, f3, f4};
    for (int i = 0; i < 4 && (arr[i] != 0.0f); ++i) {  /* May use UNEQ */
        checksum += i * 100;
    }
    
    /* 6. Switch based on comparison results */
    int cmp_results = 0;
    cmp_results |= (f1 < f2) ? 1 : 0;    /* UNLT */
    cmp_results |= (d1 > d2) ? 2 : 0;    /* UNGT */
    cmp_results |= (f3 == f4) ? 4 : 0;   /* UNEQ */
    cmp_results |= __builtin_isunordered(d3, d4) ? 8 : 0;  /* UNORDERED */
    
    switch (cmp_results & 0xF) {
        case 0: checksum += 10000; break;
        case 1: checksum += 20000; break;  /* UNLT path */
        case 2: checksum += 30000; break;  /* UNGT path */
        case 4: checksum += 40000; break;  /* UNEQ path */
        case 8: checksum += 50000; break;  /* UNORDERED path */
        default: checksum += 60000; break;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Condition codes checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;
}

/* Dummy implementations to satisfy linker */
volatile double external_seed = 3.14159;
float get_float(void) { return (float)external_seed * rand() / RAND_MAX; }
double get_double(void) { return external_seed * rand() / RAND_MAX; }
void use_result(int x) { volatile int dummy = x; (void)dummy; }
