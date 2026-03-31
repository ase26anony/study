/* test_condcodes.c - Target x86 condition code coverage */
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

/* Global variables to prevent optimization */
volatile double global_d = 3.14159;
volatile float global_f = 2.71828f;
int checksum = 0;

/* Opaque function to get inputs */
volatile double get_input_double(void) {
    static double counter = 0.0;
    return counter++ * 1.5;
}

volatile float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

void use_result(int val) {
    checksum += val;
}

int main(void) {
    /* Initialize mixed floating-point variables */
    volatile double vd1 = get_input_double();
    volatile double vd2 = global_d;
    double d1 = vd1;
    double d2 = vd2;
    double d3 = 0.0 / 0.0;  /* Potential NaN with -ffast-math assumptions */
    
    volatile float vf1 = get_input_float();
    volatile float vf2 = global_f;
    float f1 = vf1;
    float f2 = vf2;
    float f3 = 1.0f / 0.0f;  /* Infinity */
    
    /* Integer mixed with float */
    int i1 = 42;
    int i2 = -7;
    
    /* 1. Standard floating-point comparisons with relational operators */
    /* These should generate various condition codes with -ffast-math */
    
    /* UNORDERED / UNEQ patterns */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        use_result(1);
    }
    
    /* UNLE pattern */
    if (f1 <= f2) {  /* May generate UNLE */
        use_result(2);
    }
    
    /* UNGE pattern */
    if (d1 >= d2) {  /* May generate UNGE (nlt) */
        use_result(3);
    }
    
    /* UNGT pattern */
    if (f1 > f2) {  /* May generate UNGT (nle) */
        use_result(4);
    }
    
    /* UNLT pattern */
    if (d1 < d2) {  /* May generate UNLT (ult) */
        use_result(5);
    }
    
    /* Mixed float/int comparisons */
    if ((float)i1 != f1) {  /* May generate UNEQ */
        use_result(6);
    }
    
    if ((double)i2 <= d1) {  /* May generate UNLE */
        use_result(7);
    }
    
    /* 2. Explicit built-in functions for unordered checks */
    /* These directly map to specific condition codes */
    
    /* UNORDERED condition */
    if (__builtin_isunordered(d1, d3)) {
        use_result(8);
    }
    
    /* ORDERED condition */
    if (__builtin_isordered(f1, f3)) {
        use_result(9);
    }
    
    /* LTGT condition (une) */
    if (__builtin_islessgreater(d1, d2)) {
        use_result(10);
    }
    
    /* UNEQ condition */
    if (!__builtin_islessgreater(f1, f2) && !__builtin_isunordered(f1, f2)) {
        use_result(11);
    }
    
    /* 3. Vector (SIMD) comparisons */
    /* These often generate predicate masks with condition codes */
    
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v4sf vec_c = {1.0f, 1.0f, 3.0f, 3.0f};
    
    /* Vector equality - may use UNEQ */
    v4sf cmp_eq = (vec_a == vec_c);
    int eq_mask = __builtin_ia32_movmskps((__v4sf)cmp_eq);
    use_result(eq_mask);
    
    /* Vector inequality - may use LTGT */
    v4sf cmp_neq = (vec_a != vec_b);
    int neq_mask = __builtin_ia32_movmskps((__v4sf)cmp_neq);
    use_result(neq_mask);
    
    /* Vector less than - may use UNLT */
    v4sf cmp_lt = (vec_a < vec_b);
    int lt_mask = __builtin_ia32_movmskps((__v4sf)cmp_lt);
    use_result(lt_mask);
    
    /* Vector greater or equal - may use UNGE */
    v4sf cmp_ge = (vec_a >= vec_b);
    int ge_mask = __builtin_ia32_movmskps((__v4sf)cmp_ge);
    use_result(ge_mask);
    
    /* Explicit unordered vector comparison using intrinsic */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* Direct UNORDERED */
    int unord_mask = _mm_movemask_ps(unord_cmp);
    use_result(unord_mask);
    
    /* 4. Conditional moves based on FP comparisons */
    /* Forces materialization of condition codes */
    
    double cond_d = (d1 > d2) ? d1 : d2;  /* May use UNGT */
    float cond_f = (f1 == f2) ? f1 : f2;  /* May use UNEQ */
    
    /* Use conditional results to prevent dead code elimination */
    int cond_sum = (int)(cond_d * 100) + (int)(cond_f * 100);
    use_result(cond_sum);
    
    /* 5. Loop with floating-point condition */
    /* Creates multiple emission sites */
    
    float arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = (float)i * 0.5f;
    }
    
    int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0f); i++) {  /* May use UNEQ in loop */
        count++;
    }
    use_result(count);
    
    /* Switch based on comparison results */
    int cmp_result = 0;
    if (d1 < d2) cmp_result |= 1;   /* UNLT */
    if (d1 > d2) cmp_result |= 2;   /* UNGT */
    if (d1 == d2) cmp_result |= 4;  /* UNEQ */
    if (d1 != d2) cmp_result |= 8;  /* LTGT */
    
    switch (cmp_result & 3) {
        case 0: use_result(100); break;
        case 1: use_result(101); break;  /* d1 < d2 */
        case 2: use_result(102); break;  /* d1 > d2 */
        case 3: use_result(103); break;  /* unordered? */
    }
    
    /* 6. Mixed precision and complex expressions */
    
    /* Chained comparisons */
    int chain = (f1 < f2) && (d1 > d2) && (f1 != f2);
    use_result(chain ? 200 : 201);
    
    /* Ternary with float comparison */
    double complex_cond = (f1 <= f2) ? 
                         ((d1 >= d2) ? d1 : d2) : 
                         ((f1 != f2) ? f1 : f2);
    use_result((int)complex_cond);
    
    /* Print final checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
