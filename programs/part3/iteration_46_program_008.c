/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_input;
extern float get_float(void) __attribute__((noinline));
extern double get_double(void) __attribute__((noinline));

/* Opaque function to consume results */
extern void use_result(int) __attribute__((noinline));

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f, vf2 = 2.0f;
volatile double vd1 = 1.0, vd2 = 2.0;
volatile int vi = 0;

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with -ffast-math assumptions */
    float f1 = get_float();
    float f2 = get_float();
    double d1 = get_double();
    double d2 = get_double();
    
    /* These should generate various condition codes */
    if (f1 != f2) checksum += 1;  /* May generate UNEQ or LTGT */
    if (vf1 <= vf2) checksum += 2; /* May generate UNLE */
    if (d1 >= d2) checksum += 4;   /* May generate UNGE */
    if (vd1 < vd2) checksum += 8;  /* May generate UNLT */
    if (f1 == f2) checksum += 16;  /* May generate UNEQ */
    if (d1 > d2) checksum += 32;   /* May generate UNGT */
    
    /* Mixed float/double comparisons */
    if ((double)f1 != d1) checksum += 64;
    if (f1 <= (float)d2) checksum += 128;
    
    /* 2. Explicit built-in unordered checks */
    checksum += __builtin_isunordered(f1, f2) ? 256 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(d1, d2) ? 512 : 0;    /* LTGT */
    checksum += __builtin_islessequal(vf1, vf2) ? 1024 : 0;   /* UNLE */
    checksum += __builtin_isgreaterequal(vd1, vd2) ? 2048 : 0; /* UNGE */
    
    /* 3. Vector (SIMD) comparisons */
    v4sf vec_a = {f1, f2, 3.0f, 4.0f};
    v4sf vec_b = {f2, f1, 3.0f, 5.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons - these expand to condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float* fp = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (fp[i] != 0.0f) checksum += 4096;
    }
    
    /* SSE intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, f1, f2);
    __m128 sse_b = _mm_set_ps(2.0f, 1.0f, f2, f1);
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result = (d1 != d2) ? d1 : d2;      /* May use UNEQ/LTGT */
    float cmov_f = (vf1 <= vf2) ? vf1 : vf2;        /* May use UNLE */
    checksum += (int)cmov_result + (int)cmov_f;
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (f1 != 0.0f); i++) {
        checksum += i;
        f1 *= 0.5f;  /* Change value to potentially exit loop */
    }
    
    /* Switch based on comparison results */
    int cmp_case = 0;
    if (__builtin_isunordered(d1, d2)) cmp_case = 1;
    else if (d1 == d2) cmp_case = 2;
    else if (d1 < d2) cmp_case = 3;
    else if (d1 > d2) cmp_case = 4;
    
    switch (cmp_case) {
        case 1: checksum += 8192; break;  /* UNORDERED */
        case 2: checksum += 16384; break; /* UNEQ */
        case 3: checksum += 32768; break; /* UNLT */
        case 4: checksum += 65536; break; /* UNGT */
    }
    
    /* 5. Complex expression with multiple condition codes */
    double complex_expr = (d1 < d2) ? 
                         ((f1 != f2) ? d1 : d2) :
                         ((d1 == d2) ? 0.0 : d2);
    checksum += (int)complex_expr;
    
    /* 6. Prevent dead code elimination */
    use_result(checksum);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy implementations to satisfy linker */
float get_float(void) { return (float)vi + 0.5f; }
double get_double(void) { return (double)vi + 0.25; }
void use_result(int x) { vi = x; }
volatile double external_input = 3.14159;
