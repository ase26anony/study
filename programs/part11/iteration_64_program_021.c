/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== PATTERN 1: Vector shuffles with many operands ==================== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle_10(volatile int* out) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = potentially many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23); /* Next 8 from b (16-23) */
    
    /* Use AVX-512 specific built-in that may need many operands */
    v8di gather_idx = {0,2,4,6,8,10,12,14};
    v8di gather_mask = {-1,-1,-1,-1,-1,-1,-1,-1};
    double src[32];
    for (int i = 0; i < 32; i++) src[i] = i * 1.5;
    
    /* __builtin_ia32_gather3div8di may expand to many operands */
    v8di result;
    asm volatile (
        "vmovdqu64 %[idx], %%zmm0\n\t"
        "vmovdqu64 %[mask], %%zmm1\n\t"
        "vgatherqpd %[scale](%[base],%%zmm0,8), %%zmm2 %{%%k1%}\n\t"
        "vmovdqu64 %%zmm2, %[res]\n\t"
        : [res] "=m" (result)
        : [base] "r" (src), [idx] "m" (gather_idx), 
          [mask] "m" (gather_mask), [scale] "i" (1)
        : "zmm0", "zmm1", "zmm2", "k1", "memory"
    );
    
    memcpy((void*)out, &c, sizeof(c));
    memcpy((void*)(out + 16), &result, sizeof(result));
}
#endif

/* ==================== PATTERN 2: Atomic operations with many parameters ==================== */
__attribute__((noipa, noinline))
void test_atomic_11(volatile int* out) {
    volatile intptr_t shared = 0;
    intptr_t expected = 0;
    intptr_t desired = 0x12345678;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(&shared, &expected, &desired,
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST,
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic with many memory order parameters */
    intptr_t val = __atomic_exchange_n(&shared, 0x87654321, __ATOMIC_RELEASE);
    
    /* __atomic_load with explicit memory order */
    intptr_t loaded = __atomic_load_n(&shared, __ATOMIC_CONSUME);
    
    out[0] = success;
    out[1] = (int)val;
    out[2] = (int)loaded;
    out[3] = (int)expected;
}

/* ==================== PATTERN 3: OpenMP SIMD with many clauses ==================== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd_10(volatile int* out) {
    #define N 128
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    int d[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(16) safelen(32) \
                reduction(+:d[0:4])  /* OpenMP 5.0 array reduction */
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        if (i < 4) d[i] += i;
    }
    
    /* Nested pragmas for complex expansion */
    #pragma omp simd collapse(2)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            a[i*8 + j] += d[i*8 + j] >> 2;
        }
    }
    
    memcpy((void*)out, d, sizeof(int) * 16);
}
#endif

/* ==================== PATTERN 4: Inline assembly with 10+ operands ==================== */
__attribute__((noipa, noinline))
void test_asm_11(volatile int* out) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result1, result2;
    
    /* Assembly with 10 explicit operands + clobbers */
    asm volatile (
        "/* Multi-operand test */\n\t"
        "addl %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "addl %[e], %[f]\n\t"
        "addl %[g], %[h]\n\t"
        "imull %[i], %[j]\n\t"
        "movl %[b], %[r1]\n\t"
        "movl %[j], %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "0" (b), [c] "r" (c), [d] "1" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    /* Another asm with memory operands */
    int arr[10] = {0};
    asm volatile (
        "movl $1, %0\n\t"
        "movl $2, %1\n\t"
        "movl $3, %2\n\t"
        "movl $4, %3\n\t"
        "movl $5, %4\n\t"
        "movl $6, %5\n\t"
        "movl $7, %6\n\t"
        "movl $8, %7\n\t"
        "movl $9, %8\n\t"
        "movl $10, %9"
        : "=m" (arr[0]), "=m" (arr[1]), "=m" (arr[2]), "=m" (arr[3]),
          "=m" (arr[4]), "=m" (arr[5]), "=m" (arr[6]), "=m" (arr[7]),
          "=m" (arr[8]), "=m" (arr[9])
        :
        : "memory"
    );
    
    out[0] = result1 + result2;
    out[1] = arr[control % 10];
}

/* ==================== PATTERN 5: Target-specific built-ins ==================== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_multi_reg(volatile int* out) {
    /* AArch64 multi-register load/store may need many operands */
    int32x4x4_t quad_vec;  // 4 registers of 4 int32s
    
    int32_t data[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    quad_vec = vld1q_s32_x4(data);
    
    /* Use NEON table lookup with large tables */
    uint8x16_t indices = vdupq_n_u8(0);
    uint8x16x4_t table;
    table.val[0] = vdupq_n_u8(1);
    table.val[1] = vdupq_n_u8(2);
    table.val[2] = vdupq_n_u8(3);
    table.val[3] = vdupq_n_u8(4);
    
    uint8x16_t result = vqtbl4q_u8(table, indices);
    
    memcpy((void*)out, &quad_vec, sizeof(quad_vec));
    memcpy((void*)(out + 16), &result, sizeof(result));
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char* argv[]) {
    /* Create volatile seed from argv[0] */
    unsigned seed = 0;
    if (argc > 0) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
    /* Allocate output buffer */
    volatile int* results = malloc(256 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 256; i++) results[i] = 0;
    
    /* Call different test functions based on seed */
    switch (seed % 5) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle_10(results);
            #endif
            break;
        case 1:
            test_atomic_11(results);
            break;
        case 2:
            #ifdef _OPENMP
            test_openmp_simd_10(results);
            #endif
            break;
        case 3:
            test_asm_11(results);
            break;
        case 4:
            #ifdef __aarch64__
            test_aarch64_multi_reg(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    /* Use external function to keep data live */
    use((void*)results);
    
    printf("Checksum: %08x\n", checksum);
    
    free((void*)results);
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    volatile int sink = *(int*)ptr;
    (void)sink;
}
