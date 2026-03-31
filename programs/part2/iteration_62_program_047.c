// Pseudocode: result = mask ? a : b
result = _mm512_mask_blend_epi32(mask, a, b);
