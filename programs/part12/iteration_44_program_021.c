// Pseudo-code for what these might generate
__m512i result = _mm512_mask_blend_epi32(mask, src1, src2);
// For each element i: result[i] = mask[i] ? src2[i] : src1[i]
