/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Strategy 1: Use AVX-512 intrinsics for x86 targets */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inlining to ensure expansion */
__attribute__((always_inline, target("avx512f,fma")))
static inline __m512 test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, 
                                               __m512 d, __m512 e, __m512 f,
                                               __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that may expand to multi-operand instructions */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);  /* 5 operands */
    __m512 t2 = _mm512_mask_fmadd_ps(d, k2, e, f); /* 6 operands */
    __m512 t3 = _mm512_mask_sub_ps(t1, k1, t2, a); /* 5 operands */
    
    /* Nested FMA with mask - potentially 7+ operands during expansion */
    __m512 result = _mm512_mask3_fmadd_ps(t1, t2, t3, k1);
    
    /* Additional masked operation with immediate constant */
    result = _mm512_mask_mul_ps(result, k2, result, 
                               _mm512_set1_ps(2.0f)); /* 5 operands */
    
    return result;
}
#endif

/* Strategy 2: Use ARM NEON/SVE intrinsics for aarch64 */
#ifdef __ARM_NEON
#include <arm_neon.h>

/* Complex multi-vector operation with lane selection */
__attribute__((always_inline))
static inline int32x4_t test_neon_multi_operand(int32x4_t a, int32x4_t b,
                                                int32x4_t c, int32x4_t d,
                                                int32x4_t e, int32x4_t f) {
    /* Table lookup with multiple registers - expands to many operands */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vmlaq_s32(c, d, e);  /* Multiply-add: c + d*e */
    int32x4_t t3 = vqdmulhq_s32(t1, t2); /* Saturating doubling multiply high */
    
    /* Complex permutation */
    int32x4_t perm = vextq_s32(t1, t2, 2);
    int32x4_t result = vmlsq_s32(t3, perm, f); /* t3 - perm*f */
    
    return result;
}
#endif

/* Strategy 3: GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex expression with multiple vector operations */
__attribute__((always_inline))
static inline v8sf test_gcc_vector_multi_operand(v8sf a, v8sf b, v8sf c,
                                                 v8sf d, v8sf e, v8sf f,
                                                 v8sf g, v8sf h) {
    /* This complex expression may require many temporaries during expansion */
    v8sf t1 = a + b * c;          /* 3 operands */
    v8sf t2 = d - e / f;          /* 3 operands */
    v8sf t3 = g * h + t1;         /* 3 operands */
    v8sf t4 = t2 - t3 * a;        /* 3 operands */
    v8sf t5 = b + c * d - e;      /* 4 operands */
    v8sf result = t4 * t5 + f / g - h; /* 5 operands */
    
    /* Fused multiply-add style operation */
    result = result * a + b * c - d * e + f * g - h;
    
    return result;
}

/* Strategy 4: Inline assembly with many operands */
__attribute__((always_inline))
static inline long test_inline_asm_multi_operand(long a, long b, long c,
                                                 long d, long e, long f,
                                                 long g, long h, long i,
                                                 long j, long k) {
    long r1, r2, r3, r4, r5;
    
    /* 11-operand inline asm statement */
    __asm__ volatile (
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "imul %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "xor %0, %0, %5\n\t"
        "or %0, %0, %6\n\t"
        "and %0, %0, %7\n\t"
        "shl %0, %0, %8\n\t"
        "shr %0, %0, %9\n\t"
        "lea %0, [%0 + %10]"
        : "=&r"(r1), "=&r"(r2), "=&r"(r3), "=&r"(r4), "=&r"(r5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc", "memory"
    );
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Strategy 5: Complex reduction with OpenMP SIMD */
#ifdef _OPENMP
__attribute__((noinline, hot))
static float test_omp_simd_reduction(const float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Complex expression that may expand to multi-operand instructions */
        sum += arr[i] * arr[i] - arr[i] / (i + 1.0f) + 
               (arr[i] > 0 ? arr[i] : -arr[i]);
    }
    
    return sum;
}
#endif

/* Strategy 6: Built-in complex math operations */
__attribute__((always_inline))
static inline double test_builtin_fma_chain(double a, double b, double c,
                                            double d, double e, double f,
                                            double g, double h, double i,
                                            double j) {
    /* Chain of FMA operations - each FMA has 3 operands, chain creates many */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    double result = __builtin_fma(t4, j, a * b - c / d + e - f * g / h);
    
    /* Additional complex expression to prevent optimization */
    result = __builtin_fma(result, a, __builtin_fma(b, c, __builtin_fma(d, e, f)));
    
    return result;
}

/* Main test driver */
int main() {
    volatile int result = 0;
    
    /* Test 1: Vector operations (architecture specific) */
#ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 d = _mm512_set1_ps(4.0f);
        __m512 e = _mm512_set1_ps(5.0f);
        __m512 f = _mm512_set1_ps(6.0f);
        __mmask16 k1 = 0xAAAA;
        __mmask16 k2 = 0x5555;
        
        __m512 res = test_avx512_multi_operand(a, b, c, d, e, f, k1, k2);
        float* fres = (float*)&res;
        result += (int)fres[0];
    }
#endif
    
#ifdef __ARM_NEON
    {
        int32x4_t a = vdupq_n_s32(1);
        int32x4_t b = vdupq_n_s32(2);
        int32x4_t c = vdupq_n_s32(3);
        int32x4_t d = vdupq_n_s32(4);
        int32x4_t e = vdupq_n_s32(5);
        int32x4_t f = vdupq_n_s32(6);
        
        int32x4_t res = test_neon_multi_operand(a, b, c, d, e, f);
        result += vgetq_lane_s32(res, 0);
    }
#endif
    
    /* Test 2: GCC vector extensions */
    {
        v8sf a = {1,2,3,4,5,6,7,8};
        v8sf b = {2,3,4,5,6,7,8,9};
        v8sf c = {3,4,5,6,7,8,9,10};
        v8sf d = {4,5,6,7,8,9,10,11};
        v8sf e = {5,6,7,8,9,10,11,12};
        v8sf f = {6,7,8,9,10,11,12,13};
        v8sf g = {7,8,9,10,11,12,13,14};
        v8sf h = {8,9,10,11,12,13,14,15};
        
        v8sf res = test_gcc_vector_multi_operand(a, b, c, d, e, f, g, h);
        result += (int)res[0];
    }
    
    /* Test 3: Inline assembly with many operands */
    {
        long asm_res = test_inline_asm_multi_operand(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        result += (int)asm_res;
    }
    
    /* Test 4: OpenMP SIMD reduction */
#ifdef _OPENMP
    {
        float arr[100];
        for (int i = 0; i < 100; i++) arr[i] = (float)i;
        
        float omp_res = test_omp_simd_reduction(arr, 100);
        result += (int)omp_res;
    }
#endif
    
    /* Test 5: Built-in FMA chain */
    {
        double fma_res = test_builtin_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0,
                                                6.0, 7.0, 8.0, 9.0, 10.0);
        result += (int)fma_res;
    }
    
    /* Additional test: Complex nested expressions */
    {
        /* Create a complex expression that might generate many operands */
        typedef struct { float x, y, z, w; } Vec4;
        Vec4 v1 = {1,2,3,4}, v2 = {5,6,7,8}, v3 = {9,10,11,12};
        
        /* Dot product and cross product combined */
        float dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
        Vec4 cross = {
            v1.y*v2.z - v1.z*v2.y,
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x,
            0
        };
        
        /* Complex transformation */
        float transform[4][4] = {
            {1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16}
        };
        
        Vec4 transformed;
        for (int i = 0; i < 4; i++) {
            transformed.x += transform[i][0] * v1.x;
            transformed.y += transform[i][1] * v1.y;
            transformed.z += transform[i][2] * v1.z;
            transformed.w += transform[i][3] * v1.w;
        }
        
        result += (int)(dot + cross.x + cross.y + cross.z + transformed.x);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
