/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* These should generate various condition codes */
    if (__builtin_isunordered(a, b)) result |= 1;      /* UNORDERED */
    if (!__builtin_isunordered(a, b)) result |= 2;     /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;        /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;          /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16;  /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;     /* UNLE or LE */
    
    /* Explicit unordered comparisons */
    if (a != b) result |= 64;                         /* LTGT or NE */
    if (!(a < b) && !__builtin_isunordered(a, b)) result |= 128;  /* UNGE */
    if (!(a > b) && !__builtin_isunordered(a, b)) result |= 256;  /* UNLE */
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various unordered comparison intrinsics */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);    /* UNORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);        /* ORDERED */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);        /* UNGE (nlt) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);        /* UNGT (nle) */
        __m128 cmp_ule = _mm_cmpule_ps(va, vb);        /* UNLE (ule) */
        __m128 cmp_ult = _mm_cmpult_ps(va, vb);        /* UNLT (ult) */
        __m128 cmp_neq = _mm_cmpneq_ps(va, vb);        /* LTGT (une) */
        
        /* Mix results to prevent optimization */
        __m128 t1 = _mm_and_ps(cmp_unord, cmp_ord);
        __m128 t2 = _mm_or_ps(cmp_nlt, cmp_nle);
        __m128 t3 = _mm_xor_ps(cmp_ule, cmp_ult);
        __m128 t4 = _mm_add_ps(t1, t2);
        __m128 t5 = _mm_add_ps(t3, cmp_neq);
        sum = _mm_add_ps(sum, _mm_add_ps(t4, t5));
    }
    
    /* Extract some result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    unsigned char out;
    
    /* Inline asm with various condition codes */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* UNORDERED */
    result |= out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "seto %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* ORDERED */
    result |= (out << 1);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* EQ/UNEQ */
    result |= (out << 2);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* NE/LTGT */
    result |= (out << 3);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* GT/UNGT */
    result |= (out << 4);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* GE/UNGE */
    result |= (out << 5);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* LT/UNLT */
    result |= (out << 6);
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0" 
                  : "=r"(out) : "x"(a), "x"(b));      /* LE/UNLE */
    result |= (out << 7);
    
    return result;
}

__attribute__((noinline))
int test_double_operations(double a, double b) {
    int result = 0;
    
    /* Double precision comparisons */
    if (__builtin_isunordered(a, b)) result |= 1;
    if (__builtin_isgreater(a, b)) result |= 2;
    if (__builtin_isless(a, b)) result |= 4;
    
    /* Generate LTGT (une) */
    if (a != b) result |= 8;
    
    /* Generate UNEQ through complex condition */
    if (!(a < b) && !(a > b) && !__builtin_isunordered(a, b)) result |= 16;
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float fa[16], fb[16];
    double da = NAN, db = 3.14;
    
    /* Initialize with mix of normal and NaN values */
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 4 == 0) ? NAN : (float)(i * 2);
    }
    
    int checksum = 0;
    
    /* Test all functions */
    checksum += test_scalar_builtins(fa[0], fb[0]);
    checksum += test_scalar_builtins(NAN, 1.0f);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(2.0f, 1.0f);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(1.0f, 1.0f);
    
    checksum += test_vector_intrinsics(fa, fb, 16);
    
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, fb[2]);
    checksum += test_inline_asm(fa[3], NAN);
    
    checksum += test_double_operations(da, db);
    checksum += test_double_operations(5.0, 3.0);
    checksum += test_double_operations(3.0, 5.0);
    checksum += test_double_operations(4.0, 4.0);
    checksum += test_double_operations(NAN, 2.0);
    
    /* Use checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
