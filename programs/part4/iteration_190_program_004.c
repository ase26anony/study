#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function for 10 operands */
int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9) {
    int result = 0;
    
    /* Architecture-specific high-operand-count operations */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands when expanded */
    #include <immintrin.h>
    __mmask16 mask = (__mmask16)(a0 & 0xFFFF);
    int32_t data[16] = {a1, a2, a3, a4, a5, a6, a7, a8, a9, a0,
                        a1, a2, a3, a4, a5, a6};
    __m512i vdata = _mm512_loadu_si512(data);
    void* addr = (void*)(uintptr_t)a7;
    
    /* This intrinsic expands to many operands in RTL */
    _mm512_mask_compressstoreu_epi32(addr, mask, vdata);
    
    /* Use the result */
    result = (int)mask + data[0];
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and base addressing */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 16);
    uint64_t bases[4] = {a0, a1, a2, a3};
    uint64_t data[4] = {a4, a5, a6, a7};
    
    /* Complex SVE operation that may expand to many operands */
    svst1_scatter_u64base_u64(pg, svld1_u64(pg, bases), svld1_u64(pg, data));
    
    result = (int)(bases[0] + data[0]);
    
#else
    /* Generic fallback: inline assembly with 10 explicit operands */
    /* This forces the compiler to handle 10 rtx operands */
    int out0, out1, out2;
    asm volatile (
        "/* 10-operand dummy assembly */\n\t"
        "add %0, %3, %4\n\t"
        "add %1, %5, %6\n\t"
        "add %2, %7, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %1, %1, %10\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), 
          "r"(a4), "r"(a5), "r"(a6), "r"(a7)
        : "cc"
    );
    
    result = out0 + out1 + out2 + a8 + a9;
#endif
    
    return result;
}

/* Function for 11 operands */
int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9,
                     int a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VBMI2__)
    /* AVX-512 VPCOMPRESSD with mask and multiple data operands */
    #include <immintrin.h>
    __mmask16 mask = (__mmask16)((a0 ^ a1) & 0xFFFF);
    int32_t src[16] = {a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                       a0, a1, a2, a3, a4, a5};
    int32_t dst[16];
    __m512i vsrc = _mm512_loadu_si512(src);
    __m512i vdst;
    
    /* This may expand to many operands */
    vdst = _mm512_mask_compress_epi32(_mm512_loadu_si512(dst), mask, vsrc);
    _mm512_storeu_si512(dst, vdst);
    
    result = (int)mask + dst[0] + dst[1];
    
#elif defined(__ARM_FEATURE_SVE2)
    /* ARM SVE2 complex gather load with predicate */
    #include <arm_sve.h>
    svbool_t pg = svwhilelt_b32(0, 8);
    uint64_t bases[8] = {a0, a1, a2, a3, a4, a5, a6, a7};
    svuint64_t data = svld1_u64(pg, bases);
    
    /* SVE2 gather with predicate, base, and offset */
    svuint64_t gathered = svld1_gather_u64offset_u64(pg, bases, 
                        svdup_u64(a8 + a9 + a10));
    
    uint64_t temp[8];
    svst1_u64(pg, temp, gathered);
    
    result = (int)(temp[0] + temp[1]);
    
#else
    /* Generic fallback: inline assembly with 11 explicit operands */
    int out0, out1, out2, out3;
    asm volatile (
        "/* 11-operand dummy assembly */\n\t"
        "add %0, %4, %5\n\t"
        "add %1, %6, %7\n\t"
        "add %2, %8, %9\n\t"
        "add %3, %10, %11\n\t"
        "add %0, %0, %12\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), 
          "r"(a4), "r"(a5), "r"(a6), "r"(a7),
          "r"(a8), "r"(a9), "r"(a10)
        : "cc"
    );
    
    result = out0 + out1 + out2 + out3;
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Generate 12 non-constant values using PRNG and argv */
    int vals[12];
    
    for (int i = 0; i < 12; i++) {
        if (i < argc && argv[i] != NULL) {
            vals[i] = atoi(argv[i]);
        } else {
            vals[i] = (int)prng();
        }
    }
    
    /* Call both functions with overlapping but different operand counts */
    int res10 = func_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                 vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    int res11 = func_11_operands(vals[1], vals[2], vals[3], vals[4], vals[5],
                                 vals[6], vals[7], vals[8], vals[9], vals[10],
                                 vals[11]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = res10 + res11;
    
    printf("Result: %d\n", final_result);
    
    /* Additional volatile use to ensure operations aren't optimized away */
    volatile int check = final_result;
    (void)check;
    
    return 0;
}
