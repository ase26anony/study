/* test_condition_codes.c - Cover GCC i386 condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile to prevent optimization */
volatile int global_result = 0;

/* ========== Scalar builtins for various condition codes ========== */
NOINLINE int test_scalar_unordered(float a, float b) {
    /* UNORDERED case */
    int r1 = __builtin_isunordered(a, b);
    
    /* ORDERED case - via negation */
    int r2 = !__builtin_isunordered(a, b);
    
    /* UNEQ case - unordered or equal */
    int r3 = __builtin_isunordered(a, b) || (a == b);
    
    /* UNGE case - not less than (nlt) */
    int r4 = !__builtin_isless(a, b);
    
    /* UNGT case - not less or equal (nle) */
    int r5 = !__builtin_islessequal(a, b);
    
    /* UNLE case - unordered or less or equal (ule) */
    int r6 = __builtin_isunordered(a, b) || __builtin_islessequal(a, b);
    
    /* UNLT case - unordered or less than (ult) */
    int r7 = __builtin_isunordered(a, b) || __builtin_isless(a, b);
    
    /* LTGT case - less than or greater than (une) */
    int r8 = (a < b) || (a > b);
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* ========== Vector intrinsics for SSE condition codes ========== */
NOINLINE int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 va, vb, vcmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        va = _mm_loadu_ps(&a[i]);
        vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED: _mm_cmpunord_ps */
        vcmp = _mm_cmpunord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* ORDERED: _mm_cmpord_ps */
        vcmp = _mm_cmpord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGE: _mm_cmpnlt_ps (nlt) */
        vcmp = _mm_cmpnlt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGT: _mm_cmpnle_ps (nle) */
        vcmp = _mm_cmpnle_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLE: _mm_cmple_ps with unordered handling */
        vcmp = _mm_cmple_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLT: _mm_cmplt_ps with unordered handling */
        vcmp = _mm_cmplt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* LTGT: _mm_cmpneq_ps (une) */
        vcmp = _mm_cmpneq_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNEQ: _mm_cmpeq_ps with ordered check */
        vcmp = _mm_cmpeq_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
    }
    
    return result;
}

/* ========== Inline assembly with condition codes ========== */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Force use of various condition codes in inline asm */
    
    /* UNORDERED: "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED: "no" flag (not overflow, but we need ordered) */
    /* Using ordered comparison via fcomi */
    asm volatile (
        "fcomi %%st(1), %%st(0)\n\t"
        "setnp %0"
        : "=r"(tmp) : : "cc"
    );
    result += tmp;
    
    /* UNGE: "nb" or "ae" (not below/above or equal) */
    asm volatile ("setae %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT: "nbe" (not below or equal) */
    asm volatile ("seta %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE: "be" (below or equal) - unordered included */
    asm volatile ("setbe %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT: "b" (below) - unordered included */
    asm volatile ("setb %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* LTGT: "ne" (not equal) */
    asm volatile ("setne %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNEQ: "e" (equal) with unordered */
    asm volatile ("sete %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    return result;
}

/* ========== Complex floating-point conditions ========== */
NOINLINE int test_complex_conditions(float *arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float x = arr[i];
        float y = arr[i + 1];
        
        /* Complex condition that might generate UNORDERED */
        if (isnan(x) || isnan(y)) {
            count++;
        }
        
        /* Condition that might generate UNGE (nlt) */
        if (!(x < y)) {
            count++;
        }
        
        /* Condition that might generate UNGT (nle) */
        if (!(x <= y)) {
            count++;
        }
        
        /* Condition that might generate UNLE */
        if (isnan(x) || isnan(y) || (x <= y)) {
            count++;
        }
        
        /* Condition that might generate UNLT */
        if (isnan(x) || isnan(y) || (x < y)) {
            count++;
        }
        
        /* Condition that might generate LTGT (une) */
        if (x != y) {
            count++;
        }
        
        /* Condition that might generate UNEQ */
        if (isnan(x) || isnan(y) || (x == y)) {
            count++;
        }
    }
    
    return count;
}

/* ========== Main test driver ========== */
int main() {
    const int N = 64;
    float arr1[N], arr2[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 0.1f) - 3.0f;
        arr2[i] = (i % 2 == 0) ? (i * 0.2f) : NAN;
    }
    
    /* Add some special cases */
    arr1[0] = NAN;
    arr2[1] = NAN;
    arr1[10] = INFINITY;
    arr2[10] = -INFINITY;
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_unordered(arr1[0], arr2[0]);
    checksum += test_scalar_unordered(arr1[1], arr2[1]);
    checksum += test_scalar_unordered(arr1[10], arr2[10]);
    checksum += test_scalar_unordered(1.0f, 2.0f);
    checksum += test_scalar_unordered(2.0f, 1.0f);
    checksum += test_scalar_unordered(1.0f, 1.0f);
    
    /* Test vector intrinsics */
    checksum += test_vector_intrinsics(arr1, arr2, N);
    
    /* Test inline assembly */
    checksum += test_inline_asm(arr1[0], arr2[0]);
    checksum += test_inline_asm(1.0f, 2.0f);
    
    /* Test complex conditions */
    checksum += test_complex_conditions(arr1, N);
    checksum += test_complex_conditions(arr2, N);
    
    /* Store to volatile to prevent optimization */
    global_result = checksum;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}
