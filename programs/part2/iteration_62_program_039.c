// Pseudo-code showing what this might be used for
__m512i blend_vectors(__m512i a, __m512i b, __mmask64 mask) {
    // For 64 x 8-bit integers, this would use gen_avx512bw_blendmv64qi
    return _mm512_mask_blend_epi8(mask, a, b);
}
