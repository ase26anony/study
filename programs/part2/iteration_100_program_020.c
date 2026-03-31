/* test_condition_codes.c - Cover all floating-point condition code cases */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (!__builtin_isunordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less or greater (ordered and not equal) */
    if ((__builtin_isless(a, b) || __builtin_isgreater(a, b)) && 
        !__builtin_isunordered(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *fa, float *fb, int n) {
    __m128 a, b, cmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        a = _mm_loadu_ps(&fa[i]);
        b = _mm_loadu_ps(&fb[i]);
        
        /* UNORDERED */
        cmp = _mm_cmpunord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* ORDERED */
        cmp = _mm_cmpord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGE (nlt) */
        cmp = _mm_cmpnlt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGT (nle) */
        cmp = _mm_cmpnle_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLE (unordered or less or equal) - use sequence */
        __m128 lt = _mm_cmplt_ps(a, b);
        __m128 eq = _mm_cmpeq_ps(a, b);
        __m128 un = _mm_cmpunord_ps(a, b);
        cmp = _mm_or_ps(_mm_or_ps(lt, eq), un);
        result += _mm_movemask_ps(cmp);
        
        /* UNLT (unordered or less than) */
        cmp = _mm_or_ps(_mm_cmplt_ps(a, b), _mm_cmpunord_ps(a, b));
        result += _mm_movemask_ps(cmp);
        
        /* LTGT (unordered or not equal) */
        cmp = _mm_cmpneq_ps(a, b);
        result += _mm_movemask_ps(cmp);
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result_u, result_o, result_nlt, result_nle;
    int result_une, result_ueq, result_ule, result_ult;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(result_u) : : "cc");
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile ("setno %0" : "=r"(result_o) : : "cc");
    
    /* UNGE - "nb" or "ae" (not below/above or equal) */
    asm volatile ("setnb %0" : "=r"(result_nlt) : : "cc");
    
    /* UNGT - "nbe" (not below or equal) */
    asm volatile ("setnbe %0" : "=r"(result_nle) : : "cc");
    
    /* LTGT - "ne" (not equal) */
    asm volatile ("setne %0" : "=r"(result_une) : : "cc");
    
    /* UNEQ - "e" (equal) with unordered consideration */
    asm volatile ("sete %0" : "=r"(result_ueq) : : "cc");
    
    /* UNLE - "be" (below or equal) with unordered */
    asm volatile ("setbe %0" : "=r"(result_ule) : : "cc");
    
    /* UNLT - "b" (below) with unordered */
    asm volatile ("setb %0" : "=r"(result_ult) : : "cc");
    
    return result_u + result_o + result_nlt + result_nle + 
           result_une + result_ueq + result_ule + result_ult;
}

__attribute__((noinline))
int test_mixed_conditions(float *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Generate various condition codes through different comparisons */
        if (__builtin_isgreater(arr[i], arr[i+1])) sum += 1;
        if (__builtin_isless(arr[i], arr[i+1])) sum += 2;
        if (__builtin_isgreaterequal(arr[i], arr[i+1])) sum += 4;
        if (__builtin_islessequal(arr[i], arr[i+1])) sum += 8;
        if (__builtin_isunordered(arr[i], arr[i+1])) sum += 16;
        
        /* Force generation of UNGE/UNGT through negation */
        if (!__builtin_isless(arr[i], arr[i+1])) sum += 32;
        if (!__builtin_islessequal(arr[i], arr[i+1])) sum += 64;
    }
    
    return sum;
}

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[32], fb[32];
    double da[32], db[32];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.7);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 1.3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 32);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    
    /* Test mixed conditions */
    checksum += test_mixed_conditions(fa, 32);
    
    /* Use results to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Additional tests with volatile to force code generation */
    volatile float v1 = NAN, v2 = 1.0f;
    volatile double v3 = NAN, v4 = 2.0;
    
    /* Generate assembly for all condition codes */
    asm volatile ("# UNORDERED test" : : "r"(v1), "r"(v2));
    asm volatile ("# ORDERED test" : : "r"(v3), "r"(v4));
    asm volatile ("# UNEQ test" : : "r"(v1), "r"(v2));
    asm volatile ("# UNGE test" : : "r"(v1), "r"(v2));
    asm volatile ("# UNGT test" : : "r"(v1), "r"(v2));
    asm volatile ("# UNLE test" : : "r"(v1), "r"(v2));
    asm volatile ("# UNLT test" : : "r"(v1), "r"(v2));
    asm volatile ("# LTGT test" : : "r"(v1), "r"(v2));
    
    return 0;
}
