/* Test program for covering 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t prng_state = 123456789;

static uint64_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function to use 10 operands */
int func_10_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9) {
    uint64_t result = 0;
    
    /* Architecture-specific 10-operand patterns */
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 mask compress/store with many operands */
    #include <immintrin.h>
    {
        __mmask8 mask = (__mmask8)(a0 & 0xFF);
        int64_t addr[8] __attribute__((aligned(64)));
        __m512i data = _mm512_set_epi64(a9, a8, a7, a6, a5, a4, a3, a2);
        
        /* This intrinsic expands to many operands internally */
        _mm512_mask_compressstoreu_epi64(addr, mask, data);
        
        /* Use the result */
        for (int i = 0; i < 8; i++) {
            result += addr[i];
        }
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter store with predicate and base addressing */
    #include <arm_sve.h>
    {
        svbool_t pg = svwhilelt_b64(a0, a1);
        svuint64_t bases = svld1_u64(pg, (uint64_t[]){a2, a3, a4, a5, a6, a7, a8, a9});
        svuint64_t data = svdup_u64(a0 + a1 + a2);
        
        /* Complex scatter operation */
        svst1_scatter_u64base_u64(pg, bases, data);
        result = svaddv_u64(pg, bases);
    }
#else
    /* Generic fallback: inline assembly with 10 operands */
    /* This forces the compiler to handle 10 rtx operands */
    uint64_t out0, out1, out2, out3, out4;
    
    asm volatile (
        /* Dummy operation that uses all 10 input operands */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "add %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "add %4, %4, %14\n\t"
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9)
        : "cc"
    );
    
    result = out0 + out1 + out2 + out3 + out4;
#endif
    
    return (int)(result & 0x7FFFFFFF);
}

/* Function to use 11 operands */
int func_11_operands(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                     uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
                     uint64_t a8, uint64_t a9, uint64_t a10) {
    uint64_t result = 0;
    
    /* Architecture-specific 11-operand patterns */
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512VL__)
    /* AVX-512 masked gather with multiple operands */
    #include <immintrin.h>
    {
        __mmask8 mask = (__mmask8)(a0 & 0xFF);
        __m512i vindex = _mm512_set_epi64(a10, a9, a8, a7, a6, a5, a4, a3);
        const int64_t* base = (const int64_t*)(uintptr_t)a1;
        int scale = (int)(a2 & 3) + 1;
        
        __m512i gathered = _mm512_mask_i64gather_epi64(
            _mm512_setzero_si512(), mask, vindex, base, scale);
        
        /* Reduce the result */
        result = _mm512_reduce_add_epi64(gathered);
    }
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE complex predicate operation */
    #include <arm_sve.h>
    {
        svbool_t pg0 = svwhilelt_b64(a0, a1);
        svbool_t pg1 = svwhilelt_b64(a2, a3);
        svuint64_t data0 = svdup_u64(a4);
        svuint64_t data1 = svdup_u64(a5);
        svuint64_t data2 = svdup_u64(a6);
        
        /* Complex predicate operation with many operands */
        svuint64_t res = svadd_u64_z(pg0, data0, data1);
        res = svmad_u64_z(pg1, res, data2, svdup_u64(a7));
        
        result = svaddv_u64(svptrue_b64(), res);
    }
#else
    /* Generic fallback: inline assembly with 11 operands */
    uint64_t out0, out1, out2, out3, out4, out5;
    
    asm volatile (
        /* Dummy operation that uses all 11 input operands */
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
        "add %5, %5, %6\n\t"  /* Reuse first operand */
        : "=r"(out0), "=r"(out1), "=r"(out2), "=r"(out3), 
          "=r"(out4), "=r"(out5)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
          "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
          "r"(a10)
        : "cc"
    );
    
    result = out0 + out1 + out2 + out3 + out4 + out5;
#endif
    
    return (int)(result & 0x7FFFFFFF);
}

int main(int argc, char *argv[]) {
    /* Generate 12 non-constant values using PRNG and argv */
    uint64_t vals[12];
    
    /* Mix PRNG with argv for variability */
    for (int i = 0; i < 12; i++) {
        vals[i] = prng_next();
        if (i < argc) {
            vals[i] ^= (uint64_t)argv[i];
        }
    }
    
    /* Call both functions with overlapping but different operand sets */
    int res1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                vals[4], vals[5], vals[6], vals[7],
                                vals[8], vals[9]);
    
    int res2 = func_11_operands(vals[1], vals[2], vals[3], vals[4],
                                vals[5], vals[6], vals[7], vals[8],
                                vals[9], vals[10], vals[11]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = res1 + res2;
    
    /* Use the result (prevents optimization) */
    if (final_result > 1000000) {
        printf("Result: %d (high)\n", final_result);
    } else {
        printf("Result: %d (low)\n", final_result);
    }
    
    return final_result > 0 ? 0 : 1;
}
