#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi __attribute__((used));
volatile __m512i global_v32hi __attribute__((used));
volatile __m512i global_v16si __attribute__((used));
volatile __m512i global_v8di __attribute__((used));
volatile __m512 global_v16sf __attribute__((used));
volatile __m512d global_v8df __attribute__((used));

#ifdef __AVX512FP16__
volatile __m512h global_v32hf __attribute__((used));
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf __attribute__((used));
#endif

int main() {
    int result_sum = 0;
    
    // Initialize with volatile to prevent constant propagation
    volatile int seed = 42;
    int actual_seed = seed;
    
#ifdef __AVX512BW__
    // ================= V64QImode =================
    {
        __m512i a = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        __m512i b = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Create mask by comparing elements
        __mmask64 mask64 = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_set1_epi8(1)
        );
        
        // Blend based on mask
        __m512i res64qi = _mm512_mask_blend_epi8(mask64, a, b);
        global_v64qi = res64qi;
        
        // Use result to prevent dead code elimination
        int64_t sum64 = _mm512_reduce_add_epi64(res64qi);
        result_sum += (int)(sum64 & 0xFF);
    }
    
    // ================= V32HImode =================
    {
        __m512i a = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Create mask using comparison
        __mmask32 mask32 = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(a, _mm512_set1_epi16(1)),
            _mm512_set1_epi16(1)
        );
        
        // Blend operation
        __m512i res32hi = _mm512_mask_blend_epi16(mask32, a, b);
        global_v32hi = res32hi;
        
        // Use result
        int32_t sum32 = _mm512_reduce_add_epi32(res32hi);
        result_sum += (int)(sum32 & 0xFFFF);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // ================= V16SImode =================
    {
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Create mask
        __mmask16 mask16 = _mm512_cmpeq_epi32_mask(
            _mm512_and_si512(a, _mm512_set1_epi32(1)),
            _mm512_set1_epi32(1)
        );
        
        // Blend operation
        __m512i res16si = _mm512_mask_blend_epi32(mask16, a, b);
        global_v16si = res16si;
        
        // Use result
        int32_t sum16 = _mm512_reduce_add_epi32(res16si);
        result_sum += (int)(sum16 & 0xFF);
    }
    
    // ================= V8DImode =================
    {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        
        // Create mask
        __mmask8 mask8 = _mm512_cmpeq_epi64_mask(
            _mm512_and_si512(a, _mm512_set1_epi64(1)),
            _mm512_set1_epi64(1)
        );
        
        // Blend operation
        __m512i res8di = _mm512_mask_blend_epi64(mask8, a, b);
        global_v8di = res8di;
        
        // Use result
        int64_t sum8 = _mm512_reduce_add_epi64(res8di);
        result_sum += (int)(sum8 & 0xFF);
    }
    
    // ================= V16SFmode =================
    {
        __m512 a = _mm512_set_ps(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        
        // Create mask using floating-point comparison
        __mmask16 mask16f = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_LT_OQ);
        
        // Blend operation
        __m512 res16sf = _mm512_mask_blend_ps(mask16f, a, b);
        global_v16sf = res16sf;
        
        // Use result - horizontal sum
        float sumf = _mm512_reduce_add_ps(res16sf);
        result_sum += (int)sumf;
    }
    
    // ================= V8DFmode =================
    {
        __m512d a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        __m512d b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        
        // Create mask
        __mmask8 mask8d = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_LT_OQ);
        
        // Blend operation
        __m512d res8df = _mm512_mask_blend_pd(mask8d, a, b);
        global_v8df = res8df;
        
        // Use result
        double sumd = _mm512_reduce_add_pd(res8df);
        result_sum += (int)sumd;
    }
#endif // __AVX512F__

#ifdef __AVX512FP16__
    // ================= V32HFmode =================
    {
        // Initialize with _Float16 values
        _Float16 hvals_a[32];
        _Float16 hvals_b[32];
        
        for (int i = 0; i < 32; i++) {
            hvals_a[i] = (_Float16)i;
            hvals_b[i] = (_Float16)(31 - i);
        }
        
        __m512h a = _mm512_loadu_ph(hvals_a);
        __m512h b = _mm512_loadu_ph(hvals_b);
        
        // Create mask - compare with threshold
        __mmask32 mask32hf = _mm512_cmp_ph_mask(
            a, 
            _mm512_set1_ph((_Float16)15.5),
            _CMP_LT_OQ
        );
        
        // Blend operation
        __m512h res32hf = _mm512_mask_blend_ph(mask32hf, a, b);
        global_v32hf = res32hf;
        
        // Use result - store and check first element
        _Float16 result_h[32];
        _mm512_storeu_ph(result_h, res32hf);
        result_sum += (int)result_h[0];
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // ================= V32BFmode =================
    {
        // For BF16, we need to use the same intrinsic as FP16
        // but with __m512bh type
        __m512bh a, b;
        
        // Initialize with some pattern
        uint16_t bfvals_a[32];
        uint16_t bfvals_b[32];
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern for BF16
            bfvals_a[i] = i << 8;  // Just shift to create different values
            bfvals_b[i] = (31 - i) << 8;
        }
        
        // Load as epi16 then cast
        __m512i a_epi = _mm512_loadu_si512(bfvals_a);
        __m512i b_epi = _mm512_loadu_si512(bfvals_b);
        
        a = _mm512_castsi512_bh(a_epi);
        b = _mm512_castsi512_bh(b_epi);
        
        // Create mask - we need to compare, so convert to float first
        __m512 a_f32 = _mm512_cvtpbh_ps(_mm512_castbh_si512(a));
        __m512 b_f32 = _mm512_cvtpbh_ps(_mm512_castbh_si512(b));
        
        __mmask16 mask16_float = _mm512_cmp_ps_mask(
            a_f32, 
            _mm512_set1_ps(15.5f),
            _CMP_LT_OQ
        );
        
        // Expand 16-bit mask to 32-bit for blend_ph
        __mmask32 mask32bf = _cvtu32_mask32(_cvtmask16_u32(mask16_float) * 0x00010001);
        
        // Blend operation - uses same intrinsic as FP16
        __m512bh res32bf = _mm512_mask_blend_ph(mask32bf, a, b);
        global_v32bf = res32bf;
        
        // Use result - convert back and check
        __m512 res_f32 = _mm512_cvtpbh_ps(_mm512_castbh_si512(res32bf));
        float first_val = _mm512_cvtss_f32(res_f32);
        result_sum += (int)first_val;
    }
#endif // __AVX512BF16__

    printf("Result checksum: %d\n", result_sum);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), 
                     "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v16sf), "m"(global_v8df)
#ifdef __AVX512FP16__
                     , "m"(global_v32hf)
#endif
#ifdef __AVX512BF16__
                     , "m"(global_v32bf)
#endif
                     );
    
    return 0;
}
