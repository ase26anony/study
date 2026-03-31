/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* External non-inlineable functions */
void __attribute__((noinline)) foo(void);
void __attribute__((noinline)) bar(void);
void __attribute__((noinline)) baz(void);

/* Helper to prevent optimization */
static inline void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function 1: Heavy integer register pressure with call at block end */
int __attribute__((noinline)) test_integer_pressure(int a, int b, int c) {
    volatile int v0 = a;
    register int r0 asm ("r10") = a + 1;
    register int r1 asm ("r11") = r0 * 2;
    register int r2 asm ("r12") = r1 + b;
    register int r3 asm ("r13") = r2 * 3;
    register int r4 asm ("r14") = r3 - c;
    register int r5 asm ("r15") = r4 / 2;
    int r6 = r5 + 100;
    int r7 = r6 * a;
    int r8 = r7 - b;
    int r9 = r8 + c;
    int r10 = r9 * 2;
    int r11 = r10 - 5;
    int r12 = r11 + a;
    int r13 = r12 * b;
    int r14 = r13 - c;
    int r15 = r14 / 3;
    int r16 = r15 + 7;
    int r17 = r16 * a;
    int r18 = r17 - b;
    int r19 = r18 + c;
    int r20 = r19 * 4;
    
    /* Create basic block structure with call at end */
    if (a > b) {
        /* More pressure in this path */
        int r21 = r20 + r0;
        int r22 = r21 * r1;
        int r23 = r22 - r2;
        int r24 = r23 + r3;
        int r25 = r24 * r4;
        
        /* Clobber many caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all variables after call */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
               r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 +
               r21 + r22 + r23 + r24 + r25;
    } else {
        /* Different path to create CFG */
        bar();
        return a + b + c;
    }
}

/* Function 2: Heavy floating-point pressure */
double __attribute__((noinline)) test_fp_pressure(double a, double b, double c) {
    volatile double v0 = a;
    double d0 = sin(a);
    double d1 = cos(b);
    double d2 = d0 * d1;
    double d3 = tan(c);
    double d4 = d2 + d3;
    double d5 = exp(d4);
    double d6 = log(fabs(d5));
    double d7 = d6 * 2.0;
    double d8 = d7 - a;
    double d9 = d8 + b;
    double d10 = d9 * c;
    double d11 = sin(d10);
    double d12 = cos(d11);
    double d13 = d12 * 3.14159;
    double d14 = d13 / 2.71828;
    double d15 = d14 + 1.0;
    double d16 = d15 * a;
    double d17 = d16 - b;
    double d18 = d17 + c;
    double d19 = sin(d18);
    double d20 = cos(d19);
    
    /* Create switch with call at block end */
    switch ((int)a % 4) {
        case 0: {
            double d21 = d20 * 2.0;
            double d22 = d21 + d0;
            double d23 = d22 * d1;
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            foo();
            
            return d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                   d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
                   d21 + d22 + d23;
        }
        case 1:
            bar();
            return a + b;
        case 2:
            baz();
            return b + c;
        default:
            return a + b + c;
    }
}

/* Function 3: Vector/SIMD pressure */
#ifdef __AVX2__
__m256i __attribute__((noinline)) test_vector_pressure(__m256i a, __m256i b, __m256i c) {
    __m256i v0 = _mm256_add_epi32(a, b);
    __m256i v1 = _mm256_sub_epi32(v0, c);
    __m256i v2 = _mm256_mullo_epi32(v1, a);
    __m256i v3 = _mm256_slli_epi32(v2, 3);
    __m256i v4 = _mm256_srli_epi32(v3, 1);
    __m256i v5 = _mm256_add_epi32(v4, b);
    __m256i v6 = _mm256_sub_epi32(v5, c);
    __m256i v7 = _mm256_mullo_epi32(v6, v0);
    __m256i v8 = _mm256_add_epi32(v7, v1);
    __m256i v9 = _mm256_sub_epi32(v8, v2);
    __m256i v10 = _mm256_mullo_epi32(v9, v3);
    
    /* Unrolled loop with call at end of iteration */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            __m256i v11 = _mm256_add_epi32(v10, v4);
            __m256i v12 = _mm256_sub_epi32(v11, v5);
            __m256i v13 = _mm256_mullo_epi32(v12, v6);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3",
                "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            foo();
            
            v10 = _mm256_add_epi32(v13, v7);
        } else {
            bar();
        }
    }
    
    return _mm256_add_epi32(v10, v8);
}
#endif

/* Function 4: Mixed pressure with complex control flow */
float __attribute__((noinline)) test_mixed_pressure(int a, float b, double c) {
    volatile int vi = a;
    volatile float vf = b;
    volatile double vd = c;
    
    /* Integer pressure */
    int i0 = a * 2;
    int i1 = i0 + 5;
    int i2 = i1 * 3;
    int i3 = i2 - a;
    int i4 = i3 / 2;
    
    /* Float pressure */
    float f0 = b * 2.0f;
    float f1 = f0 + 3.14f;
    float f2 = f1 * b;
    float f3 = f2 - 1.0f;
    float f4 = f3 / 2.0f;
    
    /* Double pressure */
    double d0 = c * 3.0;
    double d1 = d0 + 2.71828;
    double d2 = d1 * c;
    double d3 = d2 - 1.0;
    double d4 = d3 / 3.0;
    
    /* Nested if-else with calls at block ends */
    if (a > 0) {
        if (b > 0.0f) {
            int i5 = i4 * 2;
            float f5 = f4 * 3.0f;
            double d5 = d4 * 4.0;
            
            /* Maximum clobber */
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            foo();
            
            return (float)(i0 + i1 + i2 + i3 + i4 + i5) +
                   (f0 + f1 + f2 + f3 + f4 + f5) +
                   (float)(d0 + d1 + d2 + d3 + d4 + d5);
        } else {
            bar();
            return b;
        }
    } else {
        baz();
        return b * 2.0f;
    }
}

int main(void) {
    int result = 0;
    
    /* Test integer pressure */
    result += test_integer_pressure(1, 2, 3);
    result += test_integer_pressure(10, 20, 30);
    
    /* Test FP pressure */
    double fp_res = test_fp_pressure(1.0, 2.0, 3.0);
    result += (int)fp_res;
    
    /* Test vector pressure if available */
#ifdef __AVX2__
    __m256i a = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    __m256i b = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    __m256i c = _mm256_set_epi32(1, 1, 1, 1, 1, 1, 1, 1);
    __m256i vres = test_vector_pressure(a, b, c);
    int varr[8];
    _mm256_storeu_si256((__m256i*)varr, vres);
    for (int i = 0; i < 8; i++) result += varr[i];
#endif
    
    /* Test mixed pressure */
    float mixed_res = test_mixed_pressure(5, 3.14f, 2.71828);
    result += (int)mixed_res;
    
    printf("Result: %d\n", result);
    return 0;
}
