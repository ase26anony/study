// Pseudo-code for what these might implement
__m512i blendv_epi8(__m512i a, __m512i b, __mmask64 mask) {
    // Each of 64 bytes selected from a or b based on mask bits
    return _mm512_mask_blend_epi8(mask, a, b);
}
