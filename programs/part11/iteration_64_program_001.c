/* test_optabs_coverage.c - Cover 10/11 operand cases in GCC optabs.cc */
/* Compile with: gcc -O2 -mavx512f -mavx512vl -fopenmp -ftree-vectorize -fno-tree-slp-vectorize test_optabs_coverage.c -o test */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(v16si* result, v16si a, v16si b) {
    /* Large shuffle with explicit indices - may expand to many operands */
    *result = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 
        16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Complex permute with computation */
    v16si mask = {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23};
    *result = __builtin_shuffle(a, b, mask);
}

/* ==================== Pattern 2: AVX-512 Gather Intrinsics ==================== */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic(double* result, const double* base, 
                          const int* indices, __mmask8 mask) {
    /* Gather operations with many parameters */
    __m512d src = _mm512_set1_pd(1.0);
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __m512d scale = _mm512_set1_pd(1.0);
    
    /* This gather intrinsic takes many operands during expansion */
    __m512d gathered = _mm512_mask_i32gather_pd(src, mask, vindex, base, 8);
    _mm512_storeu_pd(result, gathered);
}
#endif

/* ==================== Pattern 3: Atomic Operations with Many Parameters ==================== */
__attribute__((noipa, noinline))
int test_atomic_operation(volatile int* ptr, int expected, int desired) {
    int result = 0;
    
    /* __atomic_compare_exchange with all parameters specified */
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0,  /* weak */
                              __ATOMIC_SEQ_CST,  /* success memory order */
                              __ATOMIC_ACQUIRE); /* failure memory order */
    
    /* __atomic_exchange with memory order */
    result = __atomic_exchange_n(ptr, desired, __ATOMIC_SEQ_CST);
    
    return result;
}

/* ==================== Pattern 4: OpenMP SIMD with Complex Clauses ==================== */
__attribute__((noipa, noinline))
void test_openmp_simd(double* a, const double* b, const double* c, int n) {
    int i;
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) \
                reduction(+:a[0:n]) private(i)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + (double)i;
    }
}

/* ==================== Pattern 5: Multi-Operand Inline Assembly ==================== */
__attribute__((noipa, noinline))
long test_multi_operand_asm(long a, long b, long c, long d, 
                           long e, long f, long g, long h, 
                           long i, long j) {
    long result;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Multi-operand test %0 = f(%1..%9) */\n\t"
        "imulq %2, %1\n\t"
        "addq %3, %1\n\t"
        "addq %4, %1\n\t"
        "addq %5, %1\n\t"
        "addq %6, %1\n\t"
        "addq %7, %1\n\t"
        "addq %8, %1\n\t"
        "addq %9, %1\n\t"
        "movq %1, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "cc"
    );
    
    return result;
}

/* ==================== Pattern 6: Complex Vector Operations ==================== */
typedef float v32sf __attribute__((vector_size(128)));

__attribute__((noipa, noinline))
void test_complex_vector_op(v32sf* out, v32sf a, v32sf b, v32sf c) {
    /* Complex FMA-like operation that might expand to many operands */
    v32sf t1 = a * b + c;
    v32sf t2 = __builtin_shufflevector(t1, a, 
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47);
    *out = t2;
}

/* ==================== Main Test Driver ==================== */
int main(int argc, char* argv[]) {
    /* Use argv[0] to create a "random" seed */
    unsigned seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    control = seed;
    
    int checksum = 0;
    
    /* Pattern 1: Vector shuffle */
    if ((control & 1)) {
        v16si a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        v16si b = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
        v16si result;
        
        test_vector_shuffle(&result, a, b);
        
        /* Compute checksum */
        for (int i = 0; i < 16; i++) {
            checksum += result[i];
        }
        use(&result);
    }
    
    /* Pattern 2: Gather intrinsic (AVX-512 only) */
#ifdef __AVX512F__
    if ((control & 2)) {
        double base[64] __attribute__((aligned(64)));
        int indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
        double result[8] __attribute__((aligned(64)));
        
        for (int i = 0; i < 64; i++) {
            base[i] = i * 1.5;
        }
        
        test_gather_intrinsic(result, base, indices, 0xFF);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)result[i];
        }
        use(result);
    }
#endif
    
    /* Pattern 3: Atomic operation */
    if ((control & 4)) {
        volatile int atomic_var = 42;
        int expected = 42;
        int desired = 100;
        
        int atomic_result = test_atomic_operation(&atomic_var, expected, desired);
        checksum += atomic_result;
        use(&atomic_var);
    }
    
    /* Pattern 4: OpenMP SIMD */
    if ((control & 8)) {
        const int N = 128;
        double a[N] __attribute__((aligned(64)));
        double b[N] __attribute__((aligned(64)));
        double c[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            b[i] = i * 0.5;
            c[i] = i * 1.5;
        }
        
        test_openmp_simd(a, b, c, N);
        
        for (int i = 0; i < N; i += 8) {
            checksum += (int)a[i];
        }
        use(a);
    }
    
    /* Pattern 5: Multi-operand assembly */
    if ((control & 16)) {
        long asm_result = test_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        checksum += (int)asm_result;
        use(&asm_result);
    }
    
    /* Pattern 6: Complex vector operation */
    if ((control & 32)) {
        v32sf va, vb, vc, vresult;
        float* fa = (float*)&va;
        float* fb = (float*)&vb;
        float* fc = (float*)&vc;
        
        for (int i = 0; i < 32; i++) {
            fa[i] = i * 0.1f;
            fb[i] = i * 0.2f;
            fc[i] = i * 0.3f;
        }
        
        test_complex_vector_op(&vresult, va, vb, vc);
        
        float* fresult = (float*)&vresult;
        for (int i = 0; i < 32; i += 4) {
            checksum += (int)fresult[i];
        }
        use(&vresult);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
