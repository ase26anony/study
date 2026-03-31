/* Program to test GCC's expansion of instructions with 10-11 operands */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Helper to initialize arrays with non-constant values */
static void init_values(uint64_t *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = simple_rand() ^ (i * 0x12345678);
    }
}

/* Function designed to trigger 10-operand expansion */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
    /* Use architecture-specific intrinsics where available */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands */
    /* This intrinsic expands to multiple instructions with many operands */
    __m512i data = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | ((a1 & 0xFF) << 8));
    char buffer[64] __attribute__((aligned(64)));
    
    /* _mm512_mask_compressstoreu_epi64 expands with many operands */
    _mm512_mask_compressstoreu_epi64(buffer, mask, data);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += ((uint64_t*)buffer)[i] & 0xFF;
    }
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and multiple registers */
    /* Note: Actual SVE intrinsics may vary - this is illustrative */
    /* svbool_t pg = svwhilelt_b64(a0, a1); */
    /* svuint64_t data = svdup_u64(a2); */
    /* svuint64_t bases = svdup_u64(a3); */
    /* Complex SVE operations often expand to many operands */
    
    /* Fallback to inline assembly for SVE-like pattern */
    asm volatile (
        "/* SVE-like multi-operand operation %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */"
        : "=r"(result)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
        : "memory"
    );
    
#else
    /* Generic fallback: Extended inline assembly with 10 operands */
    /* This forces GCC to handle 10 rtx operands during expansion */
    asm volatile (
        "/* 10-operand dummy operation */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
    );
#endif
    
    return result;
}

/* Function designed to trigger 11-operand expansion */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked gather with multiple operands */
    __m512i vindex = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
    __mmask8 mask = (__mmask8)((a0 & 0xFF) | ((a1 & 0xFF) << 8));
    int64_t base[8] = {a2, a2+1, a2+2, a2+3, a2+4, a2+5, a2+6, a2+7};
    
    /* _mm512_mask_i64gather_epi64 expands with many operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(), mask, vindex, base, 8);
    
    /* Reduce the result */
    result = _mm512_reduce_add_epi64(gathered) & 0xFFFF;
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE complex operation with 11 operands */
    /* Using inline assembly to ensure 11 operands */
    asm volatile (
        "/* SVE 11-operand operation */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10)
        : "cc"
    );
    
#else
    /* Generic fallback: Extended inline assembly with 11 operands */
    asm volatile (
        "/* 11-operand dummy operation */\n"
        "mov %0, %1\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10)
    );
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t args[12];
    int result = 0;
    
    /* Initialize with non-constant values using argv and PRNG */
    for (int i = 0; i < 12; i++) {
        args[i] = simple_rand();
        if (i < argc) {
            args[i] ^= (uint64_t)argv[i];
        }
    }
    
    /* Call 10-operand function */
    result += func_10_operands(args[0], args[1], args[2], args[3],
                               args[4], args[5], args[6], args[7],
                               args[8], args[9]);
    
    /* Call 11-operand function */
    result += func_11_operands(args[0], args[1], args[2], args[3],
                               args[4], args[5], args[6], args[7],
                               args[8], args[9], args[10]);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional test with different argument patterns */
    for (int i = 0; i < 12; i++) {
        args[i] = args[i] * 0x12345678 + i;
    }
    
    int result2 = func_10_operands(args[1], args[2], args[3], args[4],
                                   args[5], args[6], args[7], args[8],
                                   args[9], args[10]);
    
    result2 += func_11_operands(args[2], args[3], args[4], args[5],
                                args[6], args[7], args[8], args[9],
                                args[10], args[11], args[0]);
    
    printf("Result2: %d\n", result2);
    
    return (result + result2) > 0 ? 0 : 1;
}
