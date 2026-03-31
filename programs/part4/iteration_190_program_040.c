/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function for 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    int result = 0;
    
    /* Architecture-specific high operand count operations */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress store with many operands when expanded */
    /* Note: This intrinsic may expand to many operands in RTL */
    #include <immintrin.h>
    {
        volatile uint32_t mem[16] = {0};
        __m512i v1 = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
        __m512i v2 = _mm512_set_epi64(a1, a0, a9, a8, a7, a6, a5, a4);
        __mmask16 mask = (__mmask16)((a0 ^ a1 ^ a2) & 0xFFFF);
        
        /* Complex operation that may expand to many operands */
        _mm512_mask_compressstoreu_epi32(mem, mask, v1);
        
        /* Use result to prevent optimization */
        for (int i = 0; i < 16; i++) {
            result += mem[i];
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and multiple registers */
    /* Note: Requires SVE support */
    #include <arm_sve.h>
    {
        volatile uint64_t mem[16] = {0};
        svuint64_t data = svdup_u64(a0);
        svuint64_t bases = svdup_u64(a1);
        svbool_t pred = svwhilelt_b64(0, 16);
        
        /* Complex SVE operation */
        svst1_scatter_u64base_u64(pred, (uint64_t*)mem, bases, data);
        
        /* Use result */
        for (int i = 0; i < 16; i++) {
            result += mem[i];
        }
    }
#else
    /* Generic fallback: Extended inline assembly with 10 operands */
    /* This forces GCC to handle many rtx operands during expansion */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        /* Dummy multi-operand operation template */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "add %4, %4, %14"
        : "=&r" (out0), "=&r" (out1), "=&r" (out2), "=&r" (out3), "=&r" (out4)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9)
        : "cc"
    );
    
    /* Use all outputs to prevent optimization */
    result = (int)(out0 + out1 + out2 + out3 + out4);
#endif
    
    return result;
}

/* Function for 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    /* AVX-512 masked gather with multiple operands */
    #include <immintrin.h>
    {
        volatile uint32_t mem[16] = {0};
        __m512i v1 = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
        __m512i v2 = _mm512_set_epi64(a2, a1, a0, a10, a9, a8, a7, a6);
        __m512i v3 = _mm512_set_epi64(a5, a4, a3, a2, a1, a0, a10, a9);
        __mmask16 mask = (__mmask16)((a0 ^ a1 ^ a2 ^ a3) & 0xFFFF);
        
        /* Initialize memory with values */
        for (int i = 0; i < 16; i++) {
            mem[i] = (uint32_t)(prng_next() & 0xFFFFFFFF);
        }
        
        /* Complex masked operation */
        __m512i gathered = _mm512_mask_i32gather_epi32(v1, mask, 
            _mm512_castsi512_si256(v2), mem, 4);
        
        /* Use result */
        uint32_t* ptr = (uint32_t*)&gathered;
        for (int i = 0; i < 16; i++) {
            result += ptr[i];
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE gather with predicate and offset */
    #include <arm_sve.h>
    {
        volatile uint64_t mem[16] = {0};
        svuint64_t data = svdup_u64(a0);
        svuint64_t bases = svdup_u64(a1);
        svuint64_t offsets = svdup_u64(a2);
        svbool_t pred = svwhilelt_b64(0, 16);
        
        /* Initialize memory */
        for (int i = 0; i < 16; i++) {
            mem[i] = prng_next();
        }
        
        /* Complex SVE gather */
        svuint64_t gathered = svld1_gather_u64offset_u64(pred, mem, offsets);
        
        /* Use result - sum elements */
        uint64_t sum = 0;
        svbool_t pall = svptrue_b64();
        sum = svaddv_u64(pall, gathered);
        result = (int)sum;
    }
#else
    /* Generic fallback: Extended inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    
    asm volatile (
        /* Dummy 11-operand operation */
        "mov %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "mov %1, %8\n\t"
        "add %1, %1, %9\n\t"
        "mov %2, %10\n\t"
        "add %2, %2, %11\n\t"
        "mov %3, %12\n\t"
        "add %3, %3, %13\n\t"
        "mov %4, %14\n\t"
        "add %4, %4, %15\n\t"
        "mov %5, %16\n\t"
        "add %5, %5, %6"
        : "=&r" (out0), "=&r" (out1), "=&r" (out2), 
          "=&r" (out3), "=&r" (out4), "=&r" (out5)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (a4),
          "r" (a5), "r" (a6), "r" (a7), "r" (a8), "r" (a9),
          "r" (a10)
        : "cc"
    );
    
    /* Use all outputs */
    result = (int)(out0 + out1 + out2 + out3 + out4 + out5);
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    uint64_t vars[11];
    
    /* Use argv for some variability, PRNG for the rest */
    for (int i = 0; i < 11; i++) {
        if (i < argc && argv[i] != NULL) {
            vars[i] = (uint64_t)argv[i][0] + i * 256;
        } else {
            vars[i] = prng_next();
        }
    }
    
    /* Call both functions with overlapping but different operand counts */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7],
                                   vars[8], vars[9], vars[10]);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2;
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
