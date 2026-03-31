/* test_condcodes.c - Target x86 condition code generation for i386.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(void) {
    return (double)rand() / RAND_MAX * 100.0 - 50.0;
}

float __attribute__((noinline)) get_float_input(void) {
    return (float)rand() / RAND_MAX * 100.0f - 50.0f;
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
    
    /* Initialize with mix of sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = -1.0;
    double d1 = get_double_input();
    double d2 = get_double_input();
    
    volatile float vf1 = 2.0f;
    volatile float vf2 = -2.0f;
    float f1 = get_float_input();
    float f2 = get_float_input();
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED/ORDERED patterns - using fast-math may generate these */
    if (vd1 != vd2) checksum += 1;           /* May generate UNEQ or LTGT */
    if (d1 == d2) checksum += 2;             /* May generate UNEQ with fast-math */
    
    /* UNGE (nlt) - not less than */
    if (f1 >= f2) checksum += 4;             /* May generate UNGE */
    
    /* UNGT (nle) - not less than or equal */
    if (vf1 > vf2) checksum += 8;            /* May generate UNGT */
    
    /* UNLE (ule) - unordered or less than or equal */
    if (d1 <= d2) checksum += 16;            /* May generate UNLE */
    
    /* UNLT (ult) - unordered or less than */
    if (f1 < f2) checksum += 32;             /* May generate UNLT */
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    if (vd1 < vd2 || vd1 > vd2) checksum += 64;  /* May generate LTGT */
    
    /* ===== EXPLICIT BUILTIN CALLS ===== */
    
    /* Direct unordered check */
    if (__builtin_isunordered(d1, d2)) checksum += 128;  /* UNORDERED */
    
    /* Ordered check */
    if (__builtin_isordered(f1, f2)) checksum += 256;    /* ORDERED */
    
    /* Less-greater (LTGT) builtin */
    if (__builtin_islessgreater(vd1, vd2)) checksum += 512;
    
    /* Unordered or equal */
    if (!__builtin_islessgreater(d1, d2)) checksum += 1024;  /* May use UNEQ */
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* Initialize vector variables */
    v4sf vec_f1 = {f1, f2, 3.0f, 4.0f};
    v4sf vec_f2 = {f2, f1, 1.0f, 5.0f};
    v2df vec_d1 = {d1, d2};
    v2df vec_d2 = {d2, d1};
    
    /* Vector comparisons - these generate predicate masks */
    v4sf cmp_eq = (vec_f1 == vec_f2);        /* May use UNEQ */
    v4sf cmp_neq = (vec_f1 != vec_f2);       /* May use LTGT */
    v4sf cmp_ge = (vec_f1 >= vec_f2);        /* May use UNGE */
    v4sf cmp_gt = (vec_f1 > vec_f2);         /* May use UNGT */
    v4sf cmp_le = (vec_f1 <= vec_f2);        /* May use UNLE */
    v4sf cmp_lt = (vec_f1 < vec_f2);         /* May use UNLT */
    
    /* Extract results to scalar checksum */
    float* cmp_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (cmp_ptr[i] != 0.0f) checksum += 2048;
    }
    
    /* Use SSE intrinsics for explicit unordered comparison */
    __m128 sse_vec1 = _mm_loadu_ps((float*)&vec_f1);
    __m128 sse_vec2 = _mm_loadu_ps((float*)&vec_f2);
    __m128 cmp_unord = _mm_cmpunord_ps(sse_vec1, sse_vec2);  /* UNORDERED */
    
    /* Check if any element is unordered */
    int mask = _mm_movemask_ps(cmp_unord);
    if (mask != 0) checksum += 4096;
    
    /* ===== CONDITIONAL MOVES/EXPRESSIONS ===== */
    
    /* Conditional move based on FP comparison */
    double cond_result1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cond_result2 = (f1 != f2) ? f1 : f2;   /* May use UNEQ or LTGT */
    
    checksum += (int)(cond_result1 + cond_result2);
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (get_float_input() != 0.0f); i++) {
        checksum += i;  /* May generate UNEQ in loop condition */
    }
    
    /* ===== MIXED INTEGER/FLOAT COMPARISONS ===== */
    
    /* Compare float with integer */
    int int_val = rand() % 100;
    if (f1 > int_val) checksum += 8192;      /* May generate UNGT */
    
    /* Compare double with integer constant */
    if (d1 <= 0) checksum += 16384;          /* May generate UNLE */
    
    /* ===== SWITCH STATEMENT WITH FP-DERIVED CONDITIONS ===== */
    
    /* Create index based on multiple comparisons */
    int index = 0;
    if (__builtin_isunordered(d1, d2)) index = 1;
    else if (d1 == d2) index = 2;
    else if (d1 < d2) index = 3;
    else if (d1 > d2) index = 4;
    
    switch (index) {
        case 1: checksum += 32768; break;  /* UNORDERED path */
        case 2: checksum += 65536; break;  /* UNEQ path */
        case 3: checksum += 131072; break; /* UNLT path */
        case 4: checksum += 262144; break; /* UNGT path */
        default: break;
    }
    
    /* Prevent dead code elimination */
    use_result(checksum);
    
    /* Print result to ensure observable behavior */
    printf("Condition codes checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
