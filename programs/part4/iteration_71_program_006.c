__m512i a, b, mask;
__m512i result = _mm512_mask_blend_epi8(mask, a, b);
