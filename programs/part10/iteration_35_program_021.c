// Pseudo-code for what these instructions do
__m512i blendmv64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}
