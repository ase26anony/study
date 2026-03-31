// Example: Blend two vectors based on a mask
__m512i result = _mm512_mask_blend_epi32(mask, a, b);
